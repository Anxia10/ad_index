#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <functional> 
#include "kernel/common/concurrency/lock_free_queue.hpp"
#include "kernel/common/log/log.h"

namespace kernel {
namespace concurrency {

typedef std::function<void()> ThreadTask;

struct TaskUnion {
    TaskUnion() {}
    explicit TaskUnion(ThreadTask& t) :
        task(t) {}
    explicit TaskUnion(ThreadTask& t, const std::string& n) :
        task(t), name(n) {}
    explicit TaskUnion(ThreadTask&& t) :
        task(std::move(t)) {}
    explicit TaskUnion(ThreadTask&& t, const std::string& n) :
        task(std::move(t)), name(n) {}
    TaskUnion& operator=(TaskUnion&& t) {
        task = std::move(t.task);
        return *this;
    }
    TaskUnion& operator=(TaskUnion& t) {
        task = t.task;
        return *this;
    }
    ThreadTask task;
    std::string name;
    // CallBackTask failed_cb_task;
};

struct ThreadSrc {
    explicit ThreadSrc(size_t queue_size) : queue(queue_size) {}
    std::condition_variable cond;
    concurrency::LockFreeQueue<TaskUnion> queue;
    std::mutex mutex;
};

class ThreadPool {
    public:
        ThreadPool(size_t worker_size, size_t queue_size);
        ~ThreadPool();
        void Start();
        void Stop();
        bool AddTask(ThreadTask&& task);
        bool AddTask(ThreadTask&& task, const std::string& name);
        void ThreadBody(ThreadSrc* src);
        // bool AddTask(const ThreadTask& task,
        //              const std::string& name,
        //              const CallBackTask& failed_cb_task);

    private:
        size_t worker_size_;
        size_t queue_size_;
        volatile int32_t status_;
        std::vector<std::thread> workers_;
        std::vector<ThreadSrc*> srcs_;
        volatile uint64_t counter_;
        LOG_DECLARE;
};

}
}