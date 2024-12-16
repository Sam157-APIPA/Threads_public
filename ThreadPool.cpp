#include "ThreadPool.h"

void ThreadPool::run()
{
#if defined(_WIN32) || defined(_WIN64)
    while (true)
    {
        std::function<void()> task;

        // �������� ������ � ������� ����� �������
        DWORD waitResult = WaitForSingleObject(semaphore, INFINITE);
        if (waitResult != WAIT_OBJECT_0)
        {
            // ������ �������� ��������
            throw std::runtime_error("Semaphore wait failed: " + std::to_string(GetLastError()));
        }

        {
            // ��������� ������� ��� ������� � �������
            DWORD mutexResult = WaitForSingleObject(winMutex, INFINITE);
            if (mutexResult != WAIT_OBJECT_0)
            {
                throw std::runtime_error("Mutex wait failed: " + std::to_string(GetLastError()));
            }

            // ���� ��� �������� � ������� �����, �������
            if (stop && tasks.empty())
            {
                ReleaseMutex(winMutex);
                break;
            }

            // ��������� ������
            if (!tasks.empty())
            {
                task = std::move(tasks.front());
                tasks.pop();
            }
            ReleaseMutex(winMutex); // ����������� �������
        }

        // ���������� ������
        if (task)
        {
            try
            {
                task();
            }
            catch (const std::exception& ex)
            {
                std::cerr << "Task execution error: " << ex.what() << std::endl;
            }
        }
    }

#else
    while (true)
    {
        std::function<void()> task;

        {
            // ��������� ������� ��� ����������� ������� � �������
            pthread_mutex_lock(&pthreadMutex);
            // ������� ��������� ������ � �������, ���� ��� �����
            while (!stop && tasks.empty())
                pthread_cond_wait(&pthreadCond, &pthreadMutex);
            // ���� ��� �������� � ������� �����, �������
            if (stop || tasks.empty())
            {
                pthread_mutex_unlock(&pthreadMutex); // ����������� �������
                return;
            }

            // ��������� ������ �� �������
            if (!tasks.empty())
            {
                task = std::move(tasks.front());
                tasks.pop();
            }
            pthread_mutex_unlock(&pthreadMutex); // ����������� �������
        }

        // ��������� ������
        if (task)
            task();
    }
#endif
}

std::future<void> ThreadPool::enqueue(std::function<void()> task)
{
    auto taskPtr = std::make_shared<std::packaged_task<void()>>(std::move(task));
    std::future<void> res = taskPtr->get_future();

#if defined(_WIN32) || defined(_WIN64)
    DWORD mutexResult = WaitForSingleObject(winMutex, INFINITE);
    if (mutexResult != WAIT_OBJECT_0)
    {
        throw std::runtime_error("Mutex wait failed: " + std::to_string(GetLastError()));
    }

    if (stop)
    {
        ReleaseMutex(winMutex);
        throw std::runtime_error("enqueue on stopped ThreadPool");
    }

    tasks.emplace([taskPtr]()
        { (*taskPtr)(); });
    ReleaseMutex(winMutex);

    if (!ReleaseSemaphore(semaphore, 1, nullptr))
    {
        throw std::runtime_error("Failed to release semaphore: " + std::to_string(GetLastError()));
    }

#else
    pthread_mutex_lock(&pthreadMutex);
    if (stop)
    {
        pthread_mutex_unlock(&pthreadMutex);
        throw std::runtime_error("enqueue on stopped ThreadPool: " + std::to_string(errno));
    }

    tasks.emplace([taskPtr]()
        { (*taskPtr)(); });
    pthread_mutex_unlock(&pthreadMutex); // ����������� �������

    pthread_cond_signal(&pthreadCond); // ���������� ���� ����� ��� ���������� ������
#endif

    return res;
}

ThreadPool::ThreadPool(size_t threads) : stop(false)
{
    try
    {
        if (threads == 0)
            throw ThreadPoolException("0 threads are available");
    }
    catch (...)
    {
        throw ThreadPoolException("Critical Error");
    }
#if defined(_WIN32) || defined(_WIN64)
    semaphore = CreateSemaphore(nullptr, 0, static_cast<LONG>(threads), nullptr);
    if (!semaphore)
    {
        throw std::runtime_error("Failed to create semaphore: " + std::to_string(GetLastError()));
    }

    winMutex = CreateMutex(nullptr, FALSE, nullptr);
    if (!winMutex)
    {
        CloseHandle(semaphore);
        throw std::runtime_error("Failed to create mutex: " + std::to_string(GetLastError()));
    }

    for (size_t i = 0; i < threads; ++i)
    {
        HANDLE thread = reinterpret_cast<HANDLE>(_beginthreadex(
            nullptr, 0, [](void* param) -> unsigned
            {
                static_cast<ThreadPool*>(param)->run();
                return 0;
            },
            this, 0, nullptr));

        if (!thread)
        {
            CloseHandle(semaphore);
            CloseHandle(winMutex);
            throw std::runtime_error("Failed to create thread: " + std::to_string(GetLastError()));
        }
        workers.emplace_back(thread);
    }
#else
    // ������������� �������� � �������� ���������� ��� POSIX
    if (pthread_mutex_init(&pthreadMutex, nullptr) != 0)
    {
        throw std::runtime_error("Failed to initialize mutex: " + std::to_string(errno));
    }
    if (pthread_cond_init(&pthreadCond, nullptr) != 0)
    {
        pthread_mutex_destroy(&pthreadMutex);
        throw std::runtime_error("Failed to initialize condition variable: " + std::to_string(errno));
    }

    // ������� ������� ������
    for (size_t i = 0; i < threads; ++i)
    {
        pthread_t thread;
        // ��� �������� 0, ���� ����� ������� ������
        if (pthread_create(&thread, nullptr, [](void* param) -> void*
            {
                static_cast<ThreadPool*>(param)->run();
                return nullptr; }, this) != 0)
        {
            pthread_mutex_destroy(&pthreadMutex);
            pthread_cond_destroy(&pthreadCond);
            throw std::runtime_error("Failed to create thread: " + std::to_string(errno));
        }
                workers.emplace_back(thread);
    }
#endif
}
ThreadPool::~ThreadPool()
{
#if defined(_WIN32) || defined(_WIN64)
    WaitForSingleObject(winMutex, INFINITE);
    stop = true;
    ReleaseMutex(winMutex);

    for (size_t i = 0; i < workers.size(); ++i)
    {
        ReleaseSemaphore(semaphore, 1, nullptr);
    }

    for (auto& worker : workers)
    {
        WaitForSingleObject(worker, INFINITE);
        CloseHandle(worker);
    }

    CloseHandle(semaphore);
    CloseHandle(winMutex);
#else
    // POSIX ����������
    pthread_mutex_lock(&pthreadMutex);
    stop = true;
    pthread_mutex_unlock(&pthreadMutex);

    pthread_cond_broadcast(&pthreadCond); // ���������� ������

    // ��������� ������
    for (auto& worker : workers)
    {
        pthread_join(worker, nullptr);
    }

    // ����������� �������
    pthread_mutex_destroy(&pthreadMutex);
    pthread_cond_destroy(&pthreadCond);
#endif
}

int ThreadPool::getCntThreads()
{
    return std::thread::hardware_concurrency();
}
