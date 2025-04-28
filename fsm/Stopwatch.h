//
// Created by ReWyn on 28.04.2025.
//

#pragma once
#include <chrono>

template <typename Clock = std::chrono::steady_clock>
class Stopwatch {
  typename Clock::time_point last;
  public:
  Stopwatch() : last(Clock::now()) {}
  void reset() {
    *this = Stopwatch();
  }

  typename Clock::duration elapsed() const {
    return Clock::now() - last;
  }
};

// template <typename T, typename Rep, typename Period>
// T change_duration(const std::chrono::duration<Rep, Period>& duration) {
//   return duration.count() * static_cast<T>(Period::num) / static_cast<T>(Period::den);
// }

