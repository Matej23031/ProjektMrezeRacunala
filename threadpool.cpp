#include "ThreadPool.h"

ThreadPool::ThreadPool(size_t n) {
    for (size_t i = 0; i < n; ++i) {
        radnici.emplace_back([this] {
            while (true) {
                std::function<void()> t;
                {
                    std::unique_lock<std::mutex> l(mut);
                    cv.wait(l, [this] { return kraj || !zadaci.empty(); });
                    if (kraj && zadaci.empty()) return;
                    t = std::move(zadaci.front());
                    zadaci.pop();
                }
                t();
            }
        });
    }
}

ThreadPool::~ThreadPool() {
    {
        std::unique_lock<std::mutex> l(mut);
        kraj = true;
    }
    cv.notify_all();
    for (auto &t : radnici) t.join();
}

void ThreadPool::enqueue(std::function<void()> f) {
    {
        std::unique_lock<std::mutex> l(mut);
        zadaci.push(std::move(f));
    }
    cv.notify_one();
}
