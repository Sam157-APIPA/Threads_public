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


// ����� ThreadPool ��� ���������� ����� �������
class ThreadPool
{
public:
    // ����������� ��� ������������� ���� ������� � �������� ����������� �������
    ThreadPool(size_t threads = std::thread::hardware_concurrency());

    // ���������� ��������� ������ �������
    ~ThreadPool();
    int getCntThreads();
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = delete;
    ThreadPool& operator=(ThreadPool&&) = delete;

    // ����� ��� ���������� ������ � ��� � ��������� ���������� ����� future
    std::future<void> enqueue(std::function<void()> task);

private:
    void run();

    std::queue<std::function<void()>> tasks;

    bool stop;

#if defined(_WIN32) || defined(_WIN64)
    // ��� Windows: ������ ������������ �������
    std::vector<HANDLE> workers;

    // ������� ��� ���������� ��������� �����
    HANDLE semaphore;

    // ������� ��� ������ ������� �����
    HANDLE winMutex;
#else
    // ��� POSIX: ������ ��������������� ������� pthread
    std::vector<pthread_t> workers;

    // ������� ��� ������������� ������� � �������
    pthread_mutex_t pthreadMutex;

    // �������� ���������� ��� ������� (POSIX)
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