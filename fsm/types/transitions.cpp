//
// Created by ReWyn on 09.05.2025.
//

#include "transitions.h"

namespace types {

void TransitionGroup::Add(const std::string& from, const std::string& to,
                          const std::string& input,
                          const std::optional<std::function<bool()>>& condition,
                          const std::string& delay) {
  if (condition.has_value()) {
    _transitions.insert({from, to, input, {}, condition.value(), delay});
    Count++;
  }
}

size_t TransitionGroup::CostAtMost(const int cost) {
  size_t total = 0;
  for (const auto& transition : _transitions) {
    if (GetCost(transition) <= cost)
      total++;
  }
  return total;
}

TGT TransitionGroup::GroupTransitions() {
  TGT transitions;
  for (const auto& transition : _transitions) {
    transitions[transition.from].Add(transition);
  }
  _transition_groups = transitions;
  return transitions;
}

TransitionGroup TransitionGroup::Retrieve(const std::string& input) {
  if (_transition_groups.empty())
    GroupTransitions();

  if (_transition_groups.contains(input))
    return _transition_groups.at(input);

  return Empty();
}

}  // namespace transitions