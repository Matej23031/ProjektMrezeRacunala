#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <vector>
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class ThreadPool {

    std::vector<std::thread> radnici;
    std::queue<std::function<void()>> zadaci;
    std::mutex mut;
    std::condition_variable cv;
    bool kraj = false;

public:

    ThreadPool(size_t n);
    ~ThreadPool();
    void enqueue(std::function<void()> f);
};

#endif
