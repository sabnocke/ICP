#pragma once

#include <absl/container/btree_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/strings/str_format.h>

#include <functional>
#include <optional>
#include <range/v3/all.hpp>
#include <range/v3/view.hpp>
#include <string>

#include "../Utils.h"

namespace types {
struct TransitionGroup;
struct Transition;
using TGTMap = absl::node_hash_map<std::string, TransitionGroup>;
using TGT = absl::btree_set<Transition>;

struct Transition {
  std::string from;
  std::string to;
  std::string input;
  std::string cond;
  std::string delay;
  int delayInt = 0;

  Transition(std::string _from, std::string _to, std::string _input,
             std::string _cond, std::string _delay)
      : from(std::move(_from)),
        to(std::move(_to)),
        input(std::move(_input)),
        cond(std::move(_cond)),
        delay(std::move(_delay)) {}

  Transition(std::string _from, std::string _to, std::string _input, int _delay)
      : from(std::move(_from)),
        to(std::move(_to)),
        input(std::move(_input)),
        delayInt(_delay) {}

  Transition() = default;

  std::tuple<std::string, std::string, std::string, std::string, std::string>
  Tuple() const {
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
        "{%s -> %s on input: <%s> if condition: <%s> after delay: <%s>} ",
        transition.from, transition.to, transition.input, transition.cond,
        transition.delay);
    if (!transition.cond.empty()) {
      os << "(has condition function)";
    } else {
      os << "(has no condition function)";
    }
    return os;
  }
};

struct TransitionGroup {
 private:
  absl::btree_set<Transition> _transitions;
  std::list<Transition> _tr;
  std::vector<Transition> _tr2;
  TGTMap _transition_groups;

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
  [[nodiscard]] bool None() const { return Size() == 0; }
  [[nodiscard]] bool IsEmpty() const { return Size() == 0; }
  static TransitionGroup Empty() {
    auto tg = TransitionGroup{};
    tg._transition_groups = {};
    return tg;
  }

  void Add(const Transition& transition) { _transitions.insert(transition); }
  void Add(const std::string& from, const std::string& to,
           const std::string& input, int delay) {
    _transitions.insert({from, to, input, delay});
  }
  TransitionGroup Cost(int cost) {
    auto it = _transitions |
              ranges::views::filter([cost](const Transition& transition) {
                return GetCost(transition) == cost;
              });
    return TransitionGroup(it | ranges::to<decltype(_transitions)>);
  }
  size_t CostAtMost(const int cost) {
    size_t total = 0;
    for (const auto& transition : _transitions) {
      if (GetCost(transition) <= cost)
        total++;
    }
    return total;
  }
  static int GetCost(const Transition& transition) {
    auto [from, to, input, cond, delay] = transition.Tuple();
    const int total = (!cond.empty() ? 1 : 0) +   // Add 1 if cond is not empty
                      (!delay.empty() ? 2 : 0) +  // Add 2 if delay is not empty
                      (!input.empty() ? 4 : 0);   // Add 4 if input is not empty
    return total;
  }
  TGTMap GroupTransitions() {
    TGTMap transitions;
    for (const auto& transition : _transitions) {
      transitions[transition.from].Add(transition);
    }
    _transition_groups = transitions;
    return transitions;
  }
  [[nodiscard]] absl::btree_set<Transition> Get() const {
    return _transitions;
  };
  TransitionGroup Retrieve(const std::string& input) {
    if (_transition_groups.empty())
      GroupTransitions();

    if (_transition_groups.contains(input))
      return _transition_groups.at(input);

    return Empty();
  }

  TransitionGroup Transform(const std::function<Transition(Transition)>& fun) {
    auto it = _transitions | ranges::views::transform(fun);
    const auto transitions = it | ranges::to<absl::btree_set<Transition>>;
    return TransitionGroup(transitions);
  }
  void TransformAction(const std::function<Transition(Transition)>& fun) {
    TGT n;
    for (auto& tr : _transitions) {
      n.insert(fun(tr));
    }
    _transitions = n;
  }
  [[nodiscard]] TransitionGroup Where(
      const std::function<bool(Transition)>& pred) const {
    auto it = _transitions | ranges::views::filter(pred);
    return TransitionGroup(it | ranges::to<decltype(_transitions)>);
  }
  [[nodiscard]] TransitionGroup WhereNoEvent() const {
    return Where([](const auto& tr) { return tr.input.empty(); });
  }
  [[nodiscard]] TransitionGroup WhereEvent() const {
    return Where([](const auto& tr) {return !tr.input.empty();});
  }
  [[nodiscard]] TransitionGroup WhereTimer() const {
    return Where([](const auto& tr) { return !tr.delay.empty(); });
  }
  [[nodiscard]] TransitionGroup WhereNoTimer() const {
    return Where([](const auto& tr) { return tr.delay.empty(); });
  }
  [[nodiscard]] TransitionGroup WhereCond() const {
    return Where([](const auto& tr) { return !tr.cond.empty(); });
  }
  [[nodiscard]] TransitionGroup WhereCondTimer() const {
    return WhereCond().WhereTimer();
  }
  [[nodiscard]] TransitionGroup WhereNone() const {
    return Where([](const auto& tr) {
      return tr.cond.empty() && tr.delay.empty() && tr.input.empty();
    });
  }
  [[nodiscard]] std::optional<Transition> Smallest() const {
    const auto min = WhereTimer().First();
    if (!min.has_value())
      return std::nullopt;
    auto min_v = min.value().get();
    for (auto &tr : _transitions) {
      if (tr.delayInt < min_v.delayInt)
        min_v = tr;
    }
    return min_v;
  }
  void Merge(TransitionGroup& trg) {
    for (auto& tr : trg._transitions) {
      if (auto pos = _transitions.find(tr); pos != _transitions.end()) {
        _transitions.erase(pos);
        _transitions.insert(tr);
      } else
        _transitions.insert(tr);
    }
  }
  bool Contains(const Transition& tr) {
    auto f =
        _transitions | ranges::views::filter([&tr](const auto& transition) {
          return transition == tr;
        });
    return ranges::distance(f) == 1;
  }
  absl::btree_set<std::string> ExternalAdd(
      const std::string_view externalName) {
    absl::btree_set<std::string> ext;
    for (auto& tr : _transitions) {
      ext.insert(Format(externalName, tr));
    }
    return ext;
  }

  /// Attempts to retrieve "first" value from group
  ///
  /// First according to ordering of internal set
  ///
  /// Which begs for a question, do I need to remember order?
  /// @return Reference to "first" item in the group
  [[nodiscard]] std::optional<std::reference_wrapper<const Transition>> First()
      const {
    if (_transitions.empty()) {
      return std::nullopt;
    }
    return std::cref(*_transitions.begin());
  }

  friend std::ostream& operator<<(std::ostream& os, const TransitionGroup& tg) {
    for (const auto& tr : tg.Get()) {
      os << tr << std::endl;
    }
    return os;
  }
  std::string Out() {
    std::stringstream ss;
    for (const auto& tr : _transitions) {
      ss << tr.to;
    }
    return ss.str();
  }

  [[nodiscard]] auto begin() const {return _transitions.begin();}
  [[nodiscard]] auto end() const {return _transitions.end();}
  [[nodiscard]] auto empty() const {return _transitions.empty();}

 private:
  static std::string Format(const std::string_view name,
                            const Transition& transition) {
    auto [from, to, input, cond, delay] = transition.Tuple();
    return absl::StrFormat("%s.Add(%s, %s, %s, [](){ %s; }, %s);", name, from,
                           to, input, Utils::Quote(cond), delay);
  }
};
}  // namespace types