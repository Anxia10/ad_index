#include "gtest/gtest.h"
#include "kernel/common/concurrency/thread_pool.h"

kernel::concurrency::ThreadPool pool(5, 10);
static int64_t sum = 0L;

TEST(ThreadPool, Start) {
    // LOG_CONFIG("./test/testdata/log4cpp.properties");
    pool.Start();
    pool.Start();
    pool.Start();
}

TEST(ThreadPool, AddTask) {
    #pragma omp parallel num_threads(100)
    {
        #pragma omp for
        for (int32_t i = 0; i < 10000; i++) {
            std::shared_ptr<int32_t> one(new int32_t);
            *one = 1;
            kernel::concurrency::ThreadTask task =
            [one] {
                for (int32_t i = 0; i < 100; i++) {
                    __sync_fetch_and_add(&sum, *one);
                }
            };
            while (!pool.AddTask(std::move(task))) {}
        }
    }
}

TEST(ThreadPool, Stop) {
    pool.Stop();
    pool.Stop();
    pool.Stop();
    EXPECT_EQ(1000000L, sum);
}