
#include "ThreadPool.h"

threadsPool::threadsPool()
{
    stop = false;
        cntTreads = thread::hardware_concurrency();
        //printf("cnt= %lu", cntTreads);
        for (int i = 0; i < cntTreads; i++) {
            //  thread th(&threadsPool::run,this);
            //   threads.push_back(&th);
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

void threadsPool::run()
{   
    while (!stop) 
    {
        function<void()> t;
        {
            /*int i = 0;
            while (i < 8 && stop_massive[i]==true)
                i++;
            if (i==7 && stop_massive[i])*/
            //printf("xd\n");
            //this_thread::sleep_for(std::chrono::seconds(1));
        std:unique_lock<std::mutex> g(m);
            note.wait(g, [this]() {
                return (!q.empty()||stop);
                });
            //if (!q.empty()) {
                //delete from queue
            if (stop)
                break;
            t = q.front();
                //mutex;
            q.pop();
            g.unlock();
                //вызов функции
            t();
            //}
        }
        
    }  

}

void threadsPool::addToQueue(function<void()> f)
{
    //mutex;
    std:lock_guard<std::mutex> g(m);
    q.push(f);
    note.notify_one();
    //mutex;
}

size_t threadsPool::getCntThreads() {
    return cntTreads;
}