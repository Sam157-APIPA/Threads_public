#pragma once
#include <iostream>
#include <fstream>
#include <vector>
#include <queue>
#include <functional>
#include <future>
#include <stdexcept>
#include <string>
#include <random>

using namespace std;

#if defined(_WIN32) || defined(_WIN64)
#include <windows.h>
#elif defined(__linux__) || defined(__unix__)
#include <pthread.h>
#include <cstring>
#else
#error "The platform is not supported"
#endif


// Класс ThreadPool для управления пулом потоков
class ThreadPool
{
public:
    // Конструктор для инициализации пула потоков с заданным количеством потоков
    ThreadPool(size_t threads = std::thread::hardware_concurrency());

    // Деструктор завершает работу потоков
    ~ThreadPool();
    int getCntThreads();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    // Метод для добавления задачи в пул и получения результата через future
    std::future<void> enqueue(std::function<void()> task);

private:
    void run();

    std::queue<std::function<void()>> tasks;

    bool stop;

#if defined(_WIN32) || defined(_WIN64)
    // Для Windows: массив дескрипторов потоков
    std::vector<HANDLE> workers;

    // Семафор для управления ожиданием задач
    HANDLE semaphore;

    // Мьютекс для защиты очереди задач
    HANDLE winMutex;
#else
    // Для POSIX: массив идентификаторов потоков pthread
    std::vector<pthread_t> workers;

    // Мьютекс для синхронизации потоков и очереди
    pthread_mutex_t pthreadMutex;

    // Условная переменная для потоков (POSIX)
    pthread_cond_t pthreadCond;
#endif
};

class ThreadPoolException : public std::exception {
private:
    std::string message;
public:
    ThreadPoolException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};