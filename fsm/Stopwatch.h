/// Source = https://stackoverflow.com/questions/2808398/easily-measure-elapsed-time

#pragma once
#include <chrono>

template <class DT = std::chrono::milliseconds,
          class ClockT = std::chrono::steady_clock>
class Timer
{
  using time_p_t = typename ClockT::time_point;
  time_p_t _start = ClockT::now(), _end = {};

public:
  void tick() {
    _end = time_p_t{};
    _start = ClockT::now();
  }

  void tock() { _end = ClockT::now(); }

  template <class T = DT>
  auto duration() const {
    return std::chrono::duration_cast<T>(_end - _start);
  }
};


