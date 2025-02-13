#pragma once

#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <stdexcept>
#include <thread>
#include <vector>

namespace nanoenv::threads {
    class ThreadPool {
    public:
        explicit ThreadPool(
            const size_t numThreads
        ) {
            for (size_t i = 0; i < numThreads; ++i) {
                workers.emplace_back([this] {
                    while (true) {
                        std::function<void()> task;

                        {
                            std::unique_lock lock(this->queueMutex);
                            this->condition.wait(lock, [this] { return this->stop || !this->tasks.empty(); });

                            if (this->stop && this->tasks.empty()) {
                                return;
                            }

                            task = std::move(this->tasks.front());
                            this->tasks.pop();
                        }

                        try {
                            task();
                        } catch (const std::exception &e) {
                            // Log the exception or handle it appropriately.
                            // For now, we just catch it to prevent the thread from terminating.
                        } catch (...) {
                            // Catch any non-standard exceptions.
                        }
                    }
                });
            }
        }

        ~ThreadPool() {
            {
                std::unique_lock lock(queueMutex);
                stop = true;
            }
            condition.notify_all();

            for (std::thread &worker : workers) {
                if (worker.joinable()) {
                    worker.join();
                }
            }
        }

        template <class F, class... Args>
        std::future<std::invoke_result_t<F, Args...>> enqueue(
            F &&f,
            Args &&...args
        ) {
            using return_type = std::invoke_result_t<F, Args...>;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...)
            );

            std::future<return_type> result = task->get_future();
            {
                std::unique_lock lock(queueMutex);

                if (stop) {
                    throw std::runtime_error("ThreadPool has been stopped.");
                }

                tasks.emplace([task] { (*task)(); });
            }

            condition.notify_one();
            return result;
        }

    private:
        std::vector<std::thread> workers;
        std::queue<std::function<void()>> tasks;
        std::mutex queueMutex;
        std::condition_variable condition;
        bool stop = false;
    };
} // namespace nanoenv::threads
