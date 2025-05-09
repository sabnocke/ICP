#pragma once

#include <absl/container/btree_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/strings/str_format.h>

#include <functional>
#include <optional>
#include <range/v3/all.hpp>
#include <range/v3/view.hpp>
#include <string>
#include <rxcpp/rx.hpp>

#include "../Utils.h"

namespace types {
struct TransitionGroup;
using TGT = absl::node_hash_map<std::string, TransitionGroup>;

struct Transition {
  std::string from;
  std::string to;
  std::string input;
  std::string cond;
  std::optional<std::function<bool()>> condition;
  std::string delay;

  Transition(std::string from, std::string to, std::string input,
             std::string cond, std::optional<std::function<bool()>> condition,
             std::string delay)
      : from(std::move(from)),
        to(std::move(to)),
        input(std::move(input)),
        cond(std::move(cond)),
        condition(std::move(condition)),
        delay(std::move(delay)) {}
  Transition() = default;

  std::tuple<std::string, std::string, std::string, std::string, std::string>
  Tuple() {
    return std::make_tuple(from, to, input, cond, delay);
  }

  bool operator==(const Transition& transition) const {
    return from == transition.from && to == transition.to &&
           input == transition.input && cond == transition.cond &&
           delay == transition.delay;
  }
  bool operator!=(const Transition& transition) const {
    return !(*this == transition);
  }
  bool operator<(const Transition& other) const {
    return std::tie(from, to, input, cond, delay) <
           std::tie(other.from, other.to, other.input, other.cond, other.delay);
  }
  friend std::ostream& operator<<(std::ostream& os,
                                  const Transition& transition) {
    os << absl::StrFormat(
        "{%s -> %s on input: <%s> if condition: <%s> after delay: <%s>}\n",
        transition.from, transition.to, transition.input, transition.cond,
        transition.delay);
    if (transition.condition.has_value()) {
      os << "(has condition function)" << std::endl;
    } else {
      os << "(has no condition function)" << std::endl;
    }
    return os;
  }
};

struct TransitionGroup {
 private:
  absl::btree_set<Transition> _transitions;
  TGT _transition_groups;

 public:
  TransitionGroup() = default;
  TransitionGroup operator+=(const Transition& tr) {
    _transitions.insert(tr);
    return *this;
  }
  TransitionGroup& operator<<(const Transition& tr) {
    _transitions.insert(tr);
    return *this;
  }
  explicit TransitionGroup(absl::btree_set<Transition> transitions)
      : _transitions(std::move(transitions)) {}

  size_t Count = 0;
  [[nodiscard]] size_t Size() const { return _transitions.size(); }
  [[nodiscard]] bool Some() const { return Size() != 0; }
  [[nodiscard]] bool IsEmpty() const { return Size() == 0; }
  static TransitionGroup Empty() {
    auto tg = TransitionGroup{};
    tg._transition_groups = {};
    return tg;
  }
  // TransitionGroup() = default;
  void Add(const Transition& transition) { _transitions.insert(transition); }
  void Add(const std::string& from, const std::string& to,
           const std::string& input,
           const std::optional<std::function<bool()>>& condition,
           const std::string& delay);
  TransitionGroup Cost(int cost) {
    auto it = _transitions |
              ranges::views::filter([cost](const Transition& transition) {
                return GetCost(transition) == cost;
              });
    return TransitionGroup(it | ranges::to<decltype(_transitions)>);
  }
  size_t CostAtMost(int cost);
  static int GetCost(const Transition& transition) {
    auto [from, to, input, cond, condition, delay] = transition;
    const int total = (!cond.empty() ? 1 : 0) +   // Add 1 if cond is not empty
                      (!delay.empty() ? 2 : 0) +  // Add 2 if delay is not empty
                      (!input.empty() ? 4 : 0);   // Add 4 if input is not empty

    return total;
  }
  TGT GroupTransitions();
  TransitionGroup Retrieve(const std::string& input);
  [[nodiscard]] TransitionGroup Select(
      const std::function<Transition(Transition)>& pred) const {
    auto it = _transitions | ranges::views::transform(pred);
    return TransitionGroup(it | ranges::to<absl::btree_set<Transition>>);
  }
  [[nodiscard]] TransitionGroup Where(
      const std::function<bool(Transition)>& pred) const {
    auto it = _transitions | ranges::views::filter(pred);
    return TransitionGroup(it | ranges::to<decltype(_transitions)>);
  }
  [[nodiscard]] TransitionGroup WhereNoEvent() const {
    return Where([](const auto& tr) { return tr.input == ""; });
  }
  [[nodiscard]] TransitionGroup WhereTimer() const {
    return Where([](const auto& tr) { return tr.delay != ""; });
  }
  [[nodiscard]] TransitionGroup WhereCond() const {
    return Where([](const auto& tr) {
      return tr.condition.has_value() && tr.condition.value()();
    });
  }
  absl::btree_set<std::string> ExternalAdd(
      const std::string_view externalName) {
    absl::btree_set<std::string> ext;
    for (auto& tr : _transitions) {
      ext.insert(Format(externalName, tr));
    }
    return ext;
  }

 private:
  static std::string Format(const std::string_view name,
                            const Transition& transition) {
    auto [from, to, input, cond, condition, delay] = transition;
    return absl::StrFormat("%s.Add(%s, %s, %s, [](){ %s; }, %s);", name, from,
                           to, input, Utils::Quote(cond), delay);
  }
};
}  // namespace types