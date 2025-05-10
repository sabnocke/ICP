//
// Created by ReWyn on 09.05.2025.
//

#pragma once

#include <chrono>
#include <condition_variable>
#include <rxcpp/rx.hpp>
#include <string>
#include <vector>
#include "types/transitions.h"

namespace Timing {
using namespace rxcpp;
using namespace types;
struct TimerConfig {
  TimerConfig() = default;
  TimerConfig(const int id, const types::Transition& transition,
              const std::chrono::milliseconds duration
              )
      : id(id), name(transition.to), duration(duration), transition(transition) {}
  int id{};
  std::string name;
  std::chrono::milliseconds duration{};
  types::Transition transition;
};

struct Timer {


private:
  std::vector<TimerConfig> configs;

public:
  void RegisterTimer(const int id, Transition transition,
                     const std::chrono::milliseconds duration) {
    configs.emplace_back(id, transition, duration);
  }
  void SetTimers(const TransitionGroup& group) {
    int id = 0;
    for (const Transition& tr : group.Get()) {
      const std::chrono::milliseconds duration(tr.delayInt);
      RegisterTimer(id++, tr, duration);
    }
  }

  std::optional<Transition> Start() const;
};
}