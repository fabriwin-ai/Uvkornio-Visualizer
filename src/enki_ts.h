#pragma once

#include <future>
#include <thread>
#include <vector>

namespace uvk {

class EnkiTaskScheduler {
 public:
  void initialize(size_t threadCount = std::thread::hardware_concurrency()) {
    threadCount_ = threadCount == 0 ? 1 : threadCount;
  }

  template <typename Func>
  std::future<void> addTask(Func&& func) {
    return std::async(std::launch::async, std::forward<Func>(func));
  }

  void waitAll(std::vector<std::future<void>>& tasks) {
    for (auto& task : tasks) {
      task.get();
    }
  }

  [[nodiscard]] size_t threadCount() const noexcept { return threadCount_; }

 private:
  size_t threadCount_{1};
};

}  // namespace uvk
