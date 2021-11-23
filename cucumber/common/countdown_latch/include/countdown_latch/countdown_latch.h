#ifndef COUNTDOWN_LATCH_COUNTDOWN_LATCH_H__
#define COUNTDOWN_LATCH_COUNTDOWN_LATCH_H__

#include <condition_variable>
#include <mutex>
#include <vector>
#include <chrono>

// Credit: https://dnikiforov.wordpress.com/2020/05/08/countdownlatch-in-c/
class CountDownLatch {
 public:
  CountDownLatch(int n) : m_rest(n) {}

// TODO: Implement wait with timeout
  void await() {
    std::unique_lock<std::mutex> lck(m_mtx);
    while (m_rest > 0) {
      m_cv.wait(lck);
    }
  }

  void count_down() {
    std::unique_lock<std::mutex> lck(m_mtx);
    --m_rest;
    m_cv.notify_all();
  }

 private:
  int m_rest;
  std::mutex m_mtx;
  std::condition_variable m_cv;
};
#endif // COUNTDOWN_LATCH_COUNTDOWN_LATCH_H__
