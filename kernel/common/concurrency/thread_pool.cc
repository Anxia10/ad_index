#include <sys/time.h>
#include <unistd.h>
#include "kernel/common/concurrency/thread_pool.h"
#define likely(x) __builtin_expect(!!(x), 1)
#define unlikely(x) __builtin_expect(!!(x), 0)
namespace kernel {
namespace concurrency {

LOG_SETUP("thred", ThreadPool);

void ThreadPool::ThreadBody(ThreadSrc* src) {
    TaskUnion task_union;
    bool empty = false;
    while (true) {
        {
            std::unique_lock<std::mutex> lock(src->mutex);
            src->cond.wait(lock,
                [this, src] {
                    return status_ == 0 || !src->queue.Empty();
                });
        }
        // task_queue is empty means status must be 0
        if (src->queue.Empty()) {
            return;
        }
        empty = src->queue.Empty();
        while (!empty) {
            std::unique_lock<std::mutex> lock(src->mutex);
            if (empty || src->queue.Pop(&task_union)) break;
            empty = src->queue.Empty();
        }
        if (empty) {
            continue;
        }
        task_union.task();
    }
}

ThreadPool::ThreadPool(size_t worker_size, size_t queue_size) :
    worker_size_(worker_size), queue_size_(queue_size),
    status_(0), counter_(0UL) {
}

ThreadPool::~ThreadPool() {
    Stop();
    for (ThreadSrc* src : srcs_) {
        if (src != nullptr) delete src;
    }
}

void ThreadPool::Start() {
    if (!__sync_bool_compare_and_swap(&status_, 0, 1)) {
        return;
    }
    for (size_t i = 0; i < worker_size_; i++) {
        srcs_.emplace_back(new ThreadSrc(queue_size_));
    }
    for (size_t i = 0; i < worker_size_; i++) {
        ThreadSrc* src = srcs_[i];
        workers_.emplace_back([this, src]() mutable->void {
            ThreadBody(src);
        });
    }
}

void ThreadPool::Stop() {
    if (!__sync_bool_compare_and_swap(&status_, 1, 0)) {
        return;
    }
    for (size_t i = 0UL; i < worker_size_; i++) {
        std::unique_lock<std::mutex> lock(srcs_[i]->mutex);
        srcs_[i]->cond.notify_all();
    }
    for (size_t i = 0UL; i < worker_size_; i++) {
        workers_[i].join();
    }
}

bool ThreadPool::AddTask(ThreadTask&& task) {
    if (unlikely(status_ == 0)) return false;
    size_t c = __sync_fetch_and_add(&counter_, 1);
    c %= srcs_.size();
    std::unique_lock<std::mutex> lock(srcs_[c]->mutex);
    if (unlikely(!srcs_[c]->queue.Push(TaskUnion(std::move(task))))) {
        return false;
    }
    srcs_[c]->cond.notify_one();
    return true;
}

bool ThreadPool::AddTask(ThreadTask&& task, const std::string& name) {
    if (unlikely(status_ == 0)) return false;
    size_t c = __sync_fetch_and_add(&counter_, 1);
    c %= srcs_.size();
    std::unique_lock<std::mutex> lock(srcs_[c]->mutex);
    if (unlikely(!srcs_[c]->queue.Push(TaskUnion(std::move(task), name)))) {
        return false;
    }
    srcs_[c]->cond.notify_one();
    return true;
}

}
}