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

//TODO or figure out different way of not copying it so much (maybe since the examples won't be complex, copying is acceptable?)
//TODO Where and coll. might instead of creating new copies create TG with different contained type
//TODO enable_if then could give different implementation of certain functions
//TODO thus removing the need for two classes and a lot of moving

namespace types {
/*class TransitionGroup;*/

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
    if (other.function.valid()) {
      function = std::move(other.function);
    } else {
      function = sol::protected_function{};
    }
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
      hasCondition = other.hasCondition;
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

  /*
   * There is an issue with this being a map of TG's since it reserves a lot of memory
   * (each from can have multiple common transitions).
   * One way of handling this is either making index_by_from a map of keys into primary
   * or a map of references (pointers) into primary.
   * Both would still require using GroupTransitions() to keep the index updated
   * But they would differ in accessing the underlying element.
   * Any change to this also requires a change in Retrieve()
   */
  absl::flat_hash_map<std::string, TransitionGroup> index_by_from{};

  TransitionGroup() = default;
  explicit TransitionGroup(TransitionGroupType&& tgt) {
    primary = std::move(tgt);
    GroupTransitions();
  }

  TransitionGroup& operator<<(const Transition& tr) {
    primary[tr.Id] = tr;
    index_by_from[primary.at(tr.Id).from].primary[tr.Id] = tr;
    return *this;
  }

  void Add(Transition&& transition) {
    /*std::cerr << "Add received transition: " << transition << std::endl;*/
    /*transitions.emplace_back(transition);*/
    primary[transition.Id] = std::move(transition);

    index_by_from[primary.at(transition.Id).from].primary[transition.Id] =
        primary[transition.Id];
    /*std::cerr << "Add inserted transition: " << primary[transition.Id]
              << std::endl;*/
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
      index_by_from[primary.at(tr.Id).from].primary[id] = tr;
    }
  }

  std::optional<TransitionGroup> Retrieve(const std::string& input) const& {
    if (index_by_from.empty())
      return std::nullopt;

    if (index_by_from.contains(input))
      return index_by_from.at(input);

    return std::nullopt;
  }

  /// Retrieves all transitions that pass filtering by predicate
  ///
  /// @remark In case of using Split, first is what passed filtering and second what didn't
  ///
  /// @tparam Split Decides whether to discard what didn't pass filtering or keep it
  /// @tparam ResultType Depending on Split, is either pair of TransitionsReference or just TransitionsReference
  /// @return Result
  template <
      bool Split = false, typename Predicate,
      typename ResultType = std::conditional_t<
          Split, std::pair<TransitionGroup, TransitionGroup>, TransitionGroup>>
  [[nodiscard]] ResultType Where(const Predicate& pred) const {
    if constexpr (Split) {
      auto on_true = TransitionGroup();
      auto on_false = TransitionGroup();

      for (auto& [id, transition] : primary) {
        if (pred(transition))
          on_true.primary[id] = transition;
        else
          on_false.primary[id] = transition;
      }

      return std::make_pair(on_true, on_false);
    } else {
      auto on_true = TransitionGroup();

      for (auto [id, transition] : primary) {
        if (pred(transition))
          on_true.primary[id] = transition;
      }
      return on_true;
    }
  }

  template <
      bool Split = false,
      typename ResultType = std::conditional_t<
          Split, std::pair<TransitionGroup, TransitionGroup>, TransitionGroup>>
  [[nodiscard]] ResultType WhereEvent() const {
    return Where<Split>([](const auto& tr) { return !tr.input.empty(); });
  }

  template <
      bool Split = false,
      typename ResultType = std::conditional_t<
          Split, std::pair<TransitionGroup, TransitionGroup>, TransitionGroup>>
  [[nodiscard]] ResultType WhereTimer() const {
    return Where<Split>([](const auto& tr) { return !tr.delay.empty(); });
  }

  [[nodiscard]] TransitionGroup WhereNone() const {
    return Where<>([](const auto& tr) {
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

  [[nodiscard]] auto begin() {
    return primary.begin();
  }
  [[nodiscard]] auto cbegin() const {
    return primary.cbegin();
  }
  [[nodiscard]] auto end() {
    return primary.end();
  }
  [[nodiscard]] auto cend() const {
    return primary.cend();
  }

  [[nodiscard]] auto empty() const { return primary.empty(); }

  TransitionGroup operator&(const TransitionGroup& other) {
    auto final = TransitionGroupType();
    auto first = ranges::size(primary | ranges::views::values);
    auto second = ranges::size(other.primary | ranges::views::values);
    final.reserve(std::min(first, second));

    auto small = first > second ? other.primary : primary;

    for (auto& el : small) {
      if (other.primary.contains(el.first) && primary.contains(el.first))
        final[el.first] = el.second;
    }

    return TransitionGroup(std::move(final));
  }

  TransitionGroup operator&(const TransitionGroup& other) const {
    auto small = Size() > other.Size() ? other.primary : primary;
    auto final = TransitionGroup();

    for (const auto& [id, tr] : small) {
      if (primary.contains(tr.Id) && other.primary.contains(tr.Id))
        final.primary[id] = tr;
    }
    return final;
  }

  TransitionGroup Merge(const TransitionGroup& other) const {
    auto large = Size() > other.Size() ? primary : other.primary;
    auto final = TransitionGroup{};
    final.primary = primary;

    for (auto [id, tr] : other.primary) {
      if (auto seek = primary.find(id);
        seek != primary.end()) {
        final << tr;
      }
    }
    final.GroupTransitions();
    return final;
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