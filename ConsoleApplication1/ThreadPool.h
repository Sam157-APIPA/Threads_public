#pragma once
#include <iostream>
#include <vector>
#include <future>
#include <thread>
#include <queue>
#include <functional>
#include <chrono>
#include <mutex>
#include <random>

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
    void MyExeption(const std::string& ex);
    void run();
    void addToQueue(function <void()>);
    size_t getCntThreads();
};

class ThreadPoolException : public std::exception {
private:
    std::string message;
public:
    ThreadPoolException(const std::string& msg) : message(msg) {}
    const char* what() const noexcept override { return message.c_str(); }
};