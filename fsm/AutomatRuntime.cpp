#include "AutomatRuntime.h"

#include <absl/container/btree_set.h>
#include <absl/container/node_hash_map.h>
#include <fast_float/fast_float.h>

#include <chrono>
#include <range/v3/algorithm/find_if.hpp>
#include <range/v3/range/conversion.hpp>
#include <range/v3/view/filter.hpp>
#include <range/v3/view/map.hpp>
#include <thread>
#include <utility>

#include "AutomatLib.h"
#include "Utils.h"
#include "messages/p2p.h"
#include "types/transitions.h"

namespace AutomatRuntime {
#define NOTHING std::nullopt
using namespace types;

Runtime::Runtime(AutomatLib::Automat& automat) : Automat(automat) {
  Name = automat.Name;
  transitions = automat.transitions;
  states = automat.states;
  GroupTransitions();
}

void Runtime::GroupTransitions() {
  for (const auto& name : states.GetNames()) {
    TransitionGroup grouping;
    for (const Transition& transition : transitions) {
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


}  // namespace AutomatRuntime