/**
 * @file   transitions.h
 * @brief  Definuje typy Transition a TransitionGroup pro správu přechodů automatu.
 * @author xhlochm00 Michal Hloch
 * @author xzelni06 Robert Zelníček
 * @details
 * Struktura Transition uchovává informace o jednotlivém přechodu mezi stavy.
 * TransitionGroup slouží ke kolekci přechodů a poskytuje metody pro filtrování,
 * skupinování, transformace a manipulaci s množinou přechodů.
 * @date   2025-05-11
 */

#pragma once

#include <absl/container/btree_set.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/container/node_hash_map.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <optional>
#include <range/v3/all.hpp>
#include <range/v3/view.hpp>
#include <string>
#include <type_traits>
#include <utility>

#include "../Utils.h"
#include "transitions.h"

namespace types {

/**
 * @class Transition
 * @brief Uchovává podrobnosti jednoho přechodu.
 */
class Transition {
 public:
  std::string from{};
  std::string to{};
  std::string input{};
  std::string condition{};
  sol::protected_function function{};
  bool hasCondition = false;
  std::string delay{};
  int delayInt{};
  unsigned Id{};

  static std::atomic<unsigned> counter;

  /**
  * @brief Konstruktor pro přechod s textovým zpožděním.
  */
  Transition(std::string _from, std::string _to, std::string _input,
             std::string _cond, std::string _delay)
      : from(std::move(_from)),
        to(std::move(_to)),
        input(std::move(_input)),
        condition(std::move(_cond)),
        delay(std::move(_delay)) {
    Id = ++counter;
  }

  explicit Transition(Transition&& other) noexcept
      : from(std::move(other.from)),
        to(std::move(other.to)),
        input(std::move(other.input)),
        condition(std::move(other.condition)),
        function(std::move(other.function)),
        hasCondition(other.hasCondition),
        delay(std::move(other.delay)),
        delayInt(other.delayInt) {
    hasCondition = function.valid();
    Id = other.Id;
  }

  Transition() { /*Id = ++counter;*/ }

  Transition(const Transition& other) = default;
  Transition& operator=(const Transition& other) = default;

  Transition& operator=(Transition&& other) noexcept {
    if (this != &other) {
      from = std::move(other.from);
      to = std::move(other.to);
      condition = std::move(other.condition);
      function = std::move(other.function);
      input = std::move(other.input);
      delay = std::move(other.delay);
      delayInt = other.delayInt;
      Id = other.Id;
      hasCondition = function.valid();
    }
    return *this;
  }

  /**
  * @brief Vrací data přechodu jako tuple.
  * @return Tuple (from, to, input, cond, delay).
  */
  [[nodiscard]] std::tuple<std::string, std::string, std::string, std::string,
                           std::string>
  Tuple() const {
    return std::make_tuple(from, to, input, condition, delay);
  }

  bool operator==(const Transition& transition) const {
    if (Id != transition.Id)
      return false;
    if (from != transition.from)
      return false;
    if (to != transition.to)
      return false;
    if (input != transition.input)
      return false;
    if (condition != transition.condition)
      return false;
    if (delay != transition.delay)
      return false;
    if (hasCondition != transition.hasCondition)
      return false;
    return true;
  }
  bool operator!=(const Transition& transition) const {
    return !(*this == transition);
  }
  bool operator<(const Transition& other) const {
    return std::tie(from, to, input, condition, delay, hasCondition) <
           std::tie(other.from, other.to, other.input, other.condition,
                    other.delay, other.hasCondition);
  }
  /**
  * @brief Výpis přechodu do výstupního proudu.
  */
  friend std::ostream& operator<<(std::ostream& os,
                                  const Transition& transition) {
    os << absl::StrFormat(
        "%d: {%s -> %s on input: <%s> if condition: <%s|%v> after delay: "
        "<%s|%v>} ",
        transition.Id, transition.from, transition.to, transition.input,
        transition.condition, transition.function.valid(), transition.delay,
        transition.delayInt);
    return os;
  }
};

/**
 * @class TransitionGroup
 * @brief Kolekce přechodů s nástroji pro manipulaci.
 */
class TransitionGroup {
 public:
  using TransitionGroupType = absl::flat_hash_map<unsigned, Transition>;

  TransitionGroupType primary{};

  absl::flat_hash_map<std::string, std::vector<unsigned>> index_by_from2{};

  TransitionGroup() = default;
  explicit TransitionGroup(TransitionGroupType&& tgt) {
    primary = std::move(tgt);
    GroupTransitions();
  }

  TransitionGroup& operator<<(const Transition& tr) {
    primary[tr.Id] = tr;
    index_by_from2[primary.at(tr.Id).from].emplace_back(tr.Id);
    return *this;
  }

  void Add(Transition&& transition) {
    primary[transition.Id] = std::move(transition);

    index_by_from2[primary.at(transition.Id).from].emplace_back(transition.Id);
  }

  [[nodiscard]] size_t Size() const { return primary.size(); }
  [[nodiscard]] bool Some() const { return Size() != 0; }
  [[nodiscard]] bool None() const { return Size() == 0; }
  [[deprecated, nodiscard]] bool IsEmpty() const { return Size() == 0; }

  [[deprecated]] static TransitionGroup Empty() {
    auto tg = TransitionGroup{};
    return tg;
  }

  void GroupTransitions() {
    for (const auto& [id, tr] : primary) {
      index_by_from2[primary.at(tr.Id).from].emplace_back(id);
    }
  }

  std::optional<TransitionGroup> Retrieve(const std::string& input) const& {
    if (index_by_from2.empty())
      return std::nullopt;

    if (index_by_from2.contains(input)) {
      auto final = TransitionGroup{};
      const auto index = index_by_from2.at(input);
      for (auto& i : index) {
        final << primary.at(i);
      }
      return final;
    }
    return std::nullopt;
  }

  template <typename Predicate>
  [[nodiscard]] TransitionGroup Where(const Predicate& pred) const {
    auto on_true = TransitionGroup();

    for (auto [id, transition] : primary) {
      if (pred(transition))
        on_true.primary[id] = transition;
    }
    return on_true;
  }

  template <
      bool Split = false,
      typename ResultType = std::conditional_t<
          Split, std::pair<TransitionGroup, TransitionGroup>, TransitionGroup>>
  [[nodiscard]] ResultType WhereTimer() const {
    return Where([](const auto& tr) { return !tr.delay.empty(); });
  }

  [[nodiscard]] TransitionGroup WhereNone() const {
    return Where([](const auto& tr) {
      return tr.hasCondition && tr.delay.empty() && tr.input.empty();
    });
  }
  [[nodiscard]] std::optional<Transition> SmallestTimer() const {
    const auto min = WhereTimer<>().First();
    if (!min.has_value())
      return std::nullopt;
    auto min_v = min.value();
    for (auto& [id, tr] : primary) {
      if (tr.delayInt < min_v.delayInt)
        min_v = tr;
    }
    return min_v;
  }

  bool Contains(const Transition& tr) const { return primary.contains(tr.Id); }

  [[nodiscard]] std::optional<Transition> First() const {
    if (primary.empty()) {
      return std::nullopt;
    }
    return primary.begin()->second;
  }

  [[nodiscard]] auto values() const {
    return ranges::begin(primary | ranges::views::values);
  }

  [[nodiscard]] auto begin() { return primary.begin(); }
  [[nodiscard]] auto cbegin() const { return primary.cbegin(); }
  [[nodiscard]] auto end() { return primary.end(); }
  [[nodiscard]] auto cend() const { return primary.cend(); }

  [[nodiscard]] auto empty() const { return primary.empty(); }

  TransitionGroup operator&(const TransitionGroup& other) {
    auto final = TransitionGroupType();
    const auto first = ranges::size(primary | ranges::views::values);
    const auto second = ranges::size(other.primary | ranges::views::values);
    final.reserve(std::min(first, second));

    auto small = first > second ? other.primary : primary;

    for (const auto& [fst, snd] : small) {
      if (other.primary.contains(fst) && primary.contains(fst))
        final[fst] = snd;
    }

    return TransitionGroup(std::move(final));
  }

  TransitionGroup Merge(const TransitionGroup& other) const {
    auto large = Size() > other.Size() ? primary : other.primary;
    auto final = TransitionGroup{};
    final.primary = primary;

    for (auto [id, tr] : other.primary) {
      if (auto seek = primary.find(id); seek != primary.end()) {
        final << tr;
      }
    }
    final.GroupTransitions();
    return final;
  }

  bool operator==(const TransitionGroup& other) const {
    return primary == other.primary;
  }
  bool operator!=(const TransitionGroup& other) const {
    return !(*this == other);
  }

  friend std::ostream& operator<<(std::ostream& os,
                                  const TransitionGroup& group) {
    for (const auto& [id, tr] : group.primary) {
        os << tr << std::endl;
    }
    return os;
  }
};
}  // namespace types