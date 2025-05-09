#include "AutomatRuntime.h"

#include <absl/container/btree_set.h>
#include <absl/container/node_hash_map.h>
#include <fast_float/fast_float.h>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/map.hpp>

#include <chrono>
#include <thread>
#include <utility>

#include "AutomatLib.h"
#include "Utils.h"
#include "messages/p2p.h"

namespace AutomatRuntime {
#define NOTHING std::nullopt

Runtime::Runtime(AutomatLib::Automat& automat) : Automat(automat) {
  Name = automat.Name;
  transitions = automat.transitions;
  states = automat.states;
  GroupTransitions();
}

void Runtime::GroupTransitions() {
  for (const auto& name : states | ranges::views::keys) {
    TransitionGroup grouping;
    for (const auto& transition : transitions) {
      if (transition.from == name) {
        grouping.Add(transition);
      }
    }
    groupedTransitions[name] = grouping;
  }
}

void Runtime::ExtractTransitions() {}

bool WaitFor(const int delay) {
  std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  return true;
}

template <typename T>
void Runtime::Store(SignalsType& signals, std::string& name, const T& value) {
  if (signals.contains(name)) {
    signals[name] = value;
  }
  //TODO needs to inform that the change happened
  //TODO convert value to string
  //TODO make the function return something
}

std::string Runtime::Load(SignalsType signals, const std::string& name) {
  if (signals.contains(name)) {
    return signals[name];
  }
  return "";
}

bool Runtime::RegisterSignal(SignalsType signals, const std::string& name) {
  absl::node_hash_map<std::string, std::string> hmap;

  if (signals.contains(name)) {
    return false;
  }
  signals[name] = "";
  return true;
}


std::optional<long long> AttemptIntegerConversion(const std::string& input) {
  const auto first = input.data();
  const auto last = input.data() + input.size();
  long long value;

  if (auto [ptr, ec] = fast_float::from_chars(first, last, value);
      ec != std::errc() || ptr != last) {
    return NOTHING;
      }

  return value;
}
std::optional<double> AttemptDoubleConversion(const std::string& input) {
  const auto first = input.data();
  const auto last = input.data() + input.size();
  double value;

  if (auto [ptr, ec] = fast_float::from_chars(first, last, value);
      ec != std::errc{} || ptr != last) {
    return NOTHING;
      }

  return value;
}
Numeric StringToNumeric(const std::string_view str) {
  const auto input = std::string(str);
  if (const auto result = AttemptIntegerConversion(input); result.has_value()) {
    return Numeric(result.value());
  }
  if (const auto result = AttemptDoubleConversion(input); result.has_value()) {
    return Numeric(result.value());
  }
  return Numeric::Empty();
}



#pragma region TransitionGroup

 TransitionGroup::TransitionGroup() {
  _transition_groups = {};
  _transitions = {};
}


void TransitionGroup::Add(const AutomatLib::Transition& transition) {
  _transitions.insert(transition);
  Count++;
}
void TransitionGroup::Add(const std::string& from, const std::string& to,
                          const std::string& input,
                          const std::optional<std::function<bool()>>& condition,
                          const std::string& delay) {
  if (condition.has_value()) {
    _transitions.insert({from, to, input, {}, condition.value(), delay});
    Count++;
  }
}

TransitionGroup TransitionGroup::WhereNoEvent() {
  return Where([](const auto& tr){ return tr.input == ""; });
}
TransitionGroup TransitionGroup::WhereTimer() {
  return Where([](const auto& tr){ return tr.delay != ""; });
}
TransitionGroup TransitionGroup::WhereCond() {
  return Where([](const auto& tr) { return tr.condition.has_value() && tr.condition.value()(); });
}

/// if none := 0
///
/// if cond := 1
///
/// if delay := 2
///
/// if cond + delay := 3
///
/// if input := 4
///
/// if cond + input := 5
///
/// if input + delay := 6
///
/// if input + delay + cond := 7
/// @param transition From what to calculate the cost
/// @return Cost of transition
int TransitionGroup::GetCost(const AutomatLib::Transition& transition) {
  auto [from, to, input, cond, condition, delay] = transition;
  int total = 0;
  if (!cond.empty())
    total += 1;
  if (!delay.empty())
    total += 1 << 1;
  if (!input.empty())
    total += 1 << 2;
  return total;
}

absl::btree_set<AutomatLib::Transition> TransitionGroup::Cost(const int cost) {
  auto res = absl::btree_set<AutomatLib::Transition>();
  for (const auto& transition : _transitions) {
    if (GetCost(transition) == cost) {
      res.insert(transition);
    }
  }
  return res;
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

TransitionGroup TransitionGroup::Where(
    const std::function<bool(AutomatLib::Transition)>& pred) {
  TransitionGroup group;
  for (const auto& transition : _transitions) {
    if (pred(transition))
      group.Add(transition);
  }
  return group;
}

#pragma endregion

}  // namespace AutomatRuntime