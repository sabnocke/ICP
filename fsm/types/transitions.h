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

template <typename T>
class TransitionGroup;
template <typename T>
struct Transition;
template <typename T>
using TGTMap = absl::node_hash_map<std::string, TransitionGroup<T>>;

/**
 * @struct Transition
 * @brief Uchovává podrobnosti jednoho přechodu.
 */

template <typename T = std::string>
struct Transition {
  using ContainedType = T;

  std::string from{};
  std::string to{};
  std::string input{};
  T cond{};
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
        cond(std::move(_cond)),
        delay(std::move(_delay)) {
    Id = ++counter;
  }
  /**
  * @brief Konstruktor pro přechod s celočíselným zpožděním.
  */
  Transition(std::string _from, std::string _to, std::string _input,
             const int _delay)
      : from(std::move(_from)),
        to(std::move(_to)),
        input(std::move(_input)),
        delayInt(_delay) {
    Id = ++counter;
  }

  template <typename OtherT>
  explicit Transition(Transition<OtherT>&& other)
      : from(std::move(other.from)),
        to(std::move(other.to)),
        input(std::move(other.input)),
        delay(std::move(other.delay)),
        delayInt(other.delayInt) {
    cond = T();
    Id = other.Id;
  }

  Transition() { Id = ++counter; }

  Transition(const Transition& other) = default;
  Transition& operator=(const Transition& other) = default;

  Transition& operator=(Transition&& other) noexcept {
    if (this != &other) {
      from = std::move(other.from);
      to = std::move(other.to);
      cond = std::move(other.cond);
      input = std::move(other.input);
      delay = std::move(other.delay);
      delayInt = other.delayInt;
      Id = other.Id;
    }
    return *this;
  }
  template <typename OtherT>
  Transition& operator=(Transition<OtherT>&& other) noexcept {
    if (this != &other) {
      from = std::move(other.from);
      to = std::move(other.to);
      cond = T();
      input = std::move(other.input);
      delay = std::move(other.delay);
      delayInt = other.delayInt;
      Id = other.Id;
    }
    return *this;
  }

  template <typename FromT>
  static Transition Convert(const Transition<FromT>& other) {
    auto result = Transition<T>{};
    result.from = other.from;
    result.to = other.to;
    result.cond = T{};
    result.delay = other.delay;
    result.delayInt = other.delayInt;
    result.Id = other.Id;

    return result;
  }

  /**
  * @brief Vrací data přechodu jako tuple.
  * @return Tuple (from, to, input, cond, delay).
  */
  [[nodiscard]] std::tuple<std::string, std::string, std::string, std::string,
                           std::string>
  Tuple() const {
    return std::make_tuple(from, to, input, cond, delay);
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
    if (cond != transition.cond)
      return false;
    if (delay != transition.delay)
      return false;
    return true;
  }
  bool operator!=(const Transition& transition) const {
    return !(*this == transition);
  }
  bool operator<(const Transition& other) const {
    return std::tie(from, to, input, cond, delay) <
           std::tie(other.from, other.to, other.input, other.cond, other.delay);
  }
  /**
  * @brief Výpis přechodu do výstupního proudu.
  */
  friend std::ostream& operator<<(std::ostream& os,
                                  const Transition& transition) {
    os << absl::StrFormat(
        "%d: {%s -> %s on input: <%s> if condition: <%s> after delay: <%v>} ",
        transition.Id, transition.from, transition.to, transition.input,
        transition.cond, transition.delay);
    return os;
  }
};

/**
 * @class TransitionGroup
 * @brief Kolekce přechodů s nástroji pro manipulaci.
 */
template <typename T>
class TransitionGroup {
public:
  using TransitionGroupType = absl::flat_hash_map<unsigned, Transition<T>>;

  TransitionGroupType primary{};
  absl::flat_hash_map<std::string, TransitionGroup> index_by_from{};


  TransitionGroup() = default;
  explicit TransitionGroup(TransitionGroupType&& tgt) {
    primary = std::move(tgt);
    GroupTransitions();
  }

  TransitionGroup& operator<<(const Transition<T>& tr) {
    primary[tr.Id] = tr;
    index_by_from[primary.at(tr.Id).from].primary[tr.Id] = tr;
    return *this;
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

  void Add(const Transition<T>& transition) {
    primary[transition.Id] = transition;

    index_by_from[primary.at(transition.Id).from].primary[transition.Id] =
        transition;
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

  [[nodiscard]] TransitionGroup WhereCond() const {
    if constexpr (std::is_same_v<T, std::string>)
      return Where<>([](const auto& tr) { return !tr.cond.empty(); });
    else
      return Where<>([](const auto& tr) { return tr.hasCondition; });
  }

  [[nodiscard]] TransitionGroup WhereNone() const {
    return Where<>([](const auto& tr) {
      return tr.hasCondition && tr.delay.empty() && tr.input.empty();
    });
  }
  [[nodiscard]] std::optional<Transition<T>> SmallestTimer() const {
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



  bool Contains(const Transition<T>& tr) { return primary.contains(tr.Id); }

  [[nodiscard]] std::optional<Transition<T>> First() const {
    if (primary.empty()) {
      return std::nullopt;
    }
    return primary.begin()->second;
  }

  [[nodiscard]] auto begin() const {
    return ranges::begin(primary | ranges::views::values);
  }
  [[nodiscard]] auto end() const {
    return ranges::end(primary | ranges::views::values);
  }
  [[nodiscard]] auto empty() const { return primary.empty(); }

  TransitionGroup operator& (const TransitionGroup& other) {
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

  friend std::ostream& operator<<(std::ostream& os, const TransitionGroup& group) {
    for (const auto& [id, tr] : group.primary) {
      os << "ID: " << id << " | " << tr << std::endl;
    }
    return os;
  }
};
}  // namespace types