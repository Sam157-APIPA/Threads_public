
#include "ThreadPool.h"


threadsPool::threadsPool()
{
    stop = false;
    try 
    {
        cntTreads = std::thread::hardware_concurrency();
        if (cntTreads == 0)
            throw ThreadPoolException("0 threads are available");
    }
    catch (...) 
    {
        throw ThreadPoolException("Critical Error");
    }
        for (int i = 0; i < cntTreads; i++) {
            threads.emplace_back(&threadsPool::run, this);
    }
        
}

threadsPool::~threadsPool()
{
    stop = true;
    note.notify_all();
    for (int i = 0; i < cntTreads; ++i) 
    {
        if (threads[i].joinable())
        {
            threads[i].join();        
        }
    }
}

void threadsPool::MyExeption(const std::string& ex)
{
    std::cerr << "Error: " << ex << std::endl;
}

void threadsPool::run()
{   
    while (!stop) 
    {
        function<void()> t;
        {
        std:unique_lock<std::mutex> g(m);
            note.wait(g, [this]() {
                return (!q.empty()||stop);
                });
            if (stop)
                break;
            t = q.front();
            q.pop();
            g.unlock();
            t();
        }
        
    }  

}

void threadsPool::addToQueue(function<void()> f)
{
       {
std:lock_guard<std::mutex> g(m);
    q.push(f);
        }
    note.notify_one();
}

size_t threadsPool::getCntThreads() {
    return cntTreads;
}

