/**
 * @file    thread_pool.h
 * @brief   general purpose thread pool
 * @author  David Hale <dhale@astro.caltech.edu>
 *
 */

#pragma once

#include <atomic>
#include <vector>
#include <queue>
#include <thread>
#include <tuple>
#include <mutex>
#include <condition_variable>
#include <future>
#include <functional>
#include <type_traits>
#include <stdexcept>

class ThreadPool {
  private:
    std::mutex mtx;
    std::condition_variable cv_worker;
    std::condition_variable cv_backlog;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    size_t max_queue_size;
    std::atomic<bool> stop;

    void worker_loop() {
      while (true) {
        std::function<void()> task;
        {
        std::unique_lock<std::mutex> lock(mtx);
        // blocks until work to do, or stop
        cv_worker.wait( lock, [this]() { return stop || !tasks.empty(); } );
        if (stop && tasks.empty()) return;

        task = std::move(tasks.front());
        tasks.pop();
        }
        // notify backlogged tasks waiting for a free slot
        cv_backlog.notify_one();
        // execute the task outside the lock
        task();
      }
    }

  public:
    explicit ThreadPool(size_t nthreads, size_t max_tasks=100)
      : max_queue_size(max_tasks),
        stop(false) {
      if (nthreads==0) throw std::invalid_argument("ThreadPool requires at least one thread");
      if (max_queue_size==0) throw std::invalid_argument("max_queue_size must be at least 1");
      workers.reserve(nthreads);
      for (size_t n=0; n < nthreads; ++n) {
        workers.emplace_back( [this]() { this->worker_loop(); } );
      }
    }

    ~ThreadPool() {
      {
      std::unique_lock<std::mutex> lock(mtx);
      // signal workers to drain and exit
      stop=true;
      }
      // wake-up all blocked workers
      cv_worker.notify_all();
      cv_backlog.notify_all();

      // wait for clean shutdown
      for (auto &thr : workers) { if (thr.joinable()) thr.join(); }
    }

    // not copyable
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;

    // accepts any callable + arguments, returns future
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) -> std::future<std::invoke_result_t<F, Args...>> {
      using return_type = std::invoke_result_t<F, Args...>;
      // wrap in a packaged task
      auto task = std::make_shared<std::packaged_task<return_type()>>(
          [f    = std::forward<F>(f),
           args = std::make_tuple(std::forward<Args>(args)...)]() mutable {
          return std::apply(std::move(f), std::move(args));
          });
      std::future<return_type> ret = task->get_future();
      {
      std::unique_lock<std::mutex> lock(mtx);
      // block until the queue has space
      cv_backlog.wait( lock, [this]() { return stop || tasks.size() < max_queue_size; } );
      if (stop) throw std::runtime_error("job added to stopped ThreadPool");
      tasks.emplace( [task]() { (*task)(); } );
      }
      cv_worker.notify_one();
      return ret;
    }
};
