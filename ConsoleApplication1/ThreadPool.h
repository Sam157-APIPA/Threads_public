#pragma once
#include <iostream>
#include <vector>
#include <future>
#include <thread>
#include <queue>
#include <functional>
#include <chrono>
#include <mutex>

using namespace std;

class threadsPool {
    queue <function <void()>> q;
    size_t cntTreads;
    vector <thread> threads;
    std::mutex m;
    bool stop;
    condition_variable note;
public:
    threadsPool();
    ~threadsPool();
       
    void run();
    void addToQueue(function <void()>);
    size_t getCntThreads();
};