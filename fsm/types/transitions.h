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
#include <fmt/base.h>

#include <functional>
#include <memory>
#include <optional>
#include <range/v3/all.hpp>
#include <range/v3/view.hpp>
#include <string>
#include <type_traits>
#include <utility>

#include "../Utils.h"

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

  /*Transition(std::string&& _from, std::string&& _to, std::string&& _input,
             std::string&& _cond, std::string&& _delay)
      : from(std::move(_from)),
        to(std::move(_to)),
        input(std::move(_input)),
        cond(std::move(_cond)),
        delay(std::move(_delay)) {
    Id = ++counter;
  }*/

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
      cond = T();  // <-- ensuring correct conversion here is unnecessary
      input = std::move(other.input);
      delay = std::move(other.delay);
      delayInt = other.delayInt;
      Id = other.Id;
    }
    return *this;
  }

  /*template <typename FromT>
  static Transition Convert(const Transition<FromT>& other) {
    using PureType = Utils::detail::remove_cvref_t<decltype(other)>;
    return ConvertMove<PureType>(other);
  }*/

  template<typename FromT>
  static Transition Convert(const Transition<FromT>& from) {
    return Convert<FromT>(Transition<FromT>(from));
  }

  template <typename FromT>
  static Transition Convert(Transition<FromT>&& other) {
    auto result = Transition<T>{};
    result.from = std::move(other.from);
    result.to = std::move(other.to);
    result.cond = T{};
    result.delay = std::move(other.delay);
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
  /**
  * @brief Výpis přechodu do výstupního proudu.
  */
  friend std::ostream& operator<<(std::ostream& os,
                                  const Transition& transition) {
    os << absl::StrFormat(
        "%d: {%s -> %s on input: <%s> if condition: <%s> after delay: <%s>} ",
        transition.Id, transition.from, transition.to, transition.input,
        transition.cond, transition.delay);
    return os;
  }
};

template <typename Type, template <typename...> class Args>
struct is_specialization_of final : std::false_type {};

template <template <typename...> class PointerType, typename... Args>
struct is_specialization_of<PointerType<Args...>, PointerType> final
    : std::true_type {};

template <typename PointerType>
inline constexpr bool is_smart_ptr =
    std::disjunction_v<is_specialization_of<PointerType, std::unique_ptr>,
                       is_specialization_of<PointerType, std::shared_ptr>>;

template <typename T>
class TransitionsReference {
  using PureType = T;
  using ElementType = std::reference_wrapper<Transition<T>>;
  using ContainerType = std::vector<ElementType>;
  ContainerType container;

 public:
  explicit TransitionsReference(ContainerType&& coll)
      : container(std::move(coll)) {}

  TransitionsReference() = default;

  [[nodiscard]] auto None() const { return container.empty(); }

  [[nodiscard]] auto Some() const { return !container.empty(); }

  auto begin() { return container.begin(); }
  auto end() { return container.end(); }

  auto cbegin() const { return container.cbegin(); }
  auto cend() const { return container.cend(); }

  auto size() const { return container.size(); }

  /*template <bool Constant = false,
            typename Result = std::conditional_t<
                Constant, std::add_const<ElementType>, ElementType>>
  Result First() {
    return container.front();
  }*/

  [[nodiscard]] std::optional<ElementType> First() {
    if (container.empty())
      return std::nullopt;

    return container.front();
  }

  [[nodiscard]] std::optional<ElementType> First() const {
    if (container.empty()) return std::nullopt;

    return container.front();
  }

  void Add(Transition<T>& transition) {
    container.emplace_back(std::ref(transition));
  }
  void Add(const ElementType& element) {
    using PureRef = Utils::detail::remove_cvref_t<decltype(element)>;
    container.emplace_back(PureRef(element));
  }

  [[nodiscard]]
  auto Where(const std::function<bool(const Transition<T>&)>& pred) const {
    return container | ranges::views::filter(pred) |
           ranges::to<TransitionsReference<T>>();
  }
  template <bool Split = false,
            typename Result = std::conditional_t<
                Split, std::pair<TransitionsReference, TransitionsReference>,
                TransitionsReference>>
  [[nodiscard]]
  Result WhereMut(const std::function<bool(const Transition<T>&)>& pred) {
    auto adapted_pred = [&](const ElementType& el) {
      const auto tr = el.get();
      return pred(tr);
    };

    if constexpr (Split) {
      auto first = container | ranges::views::filter(adapted_pred) |
                   ranges::to<TransitionsReference<T>>();
      auto second = container | ranges::views::filter([&](const auto& elem) {
                      return !pred(elem.get());
                    }) |
                    ranges::to<TransitionsReference<T>>();
      return std::make_pair(first, second);
    } else {
      auto view = container | ranges::views::filter(adapted_pred) |
                  ranges::to<TransitionsReference<T>>();
      return view;
    }
  }

  auto WhereNone() {
    auto pred = [](const Transition<T>& tr) -> bool {
      if constexpr (std::is_same_v<T, std::string>)
        return tr.delay.empty() && tr.cond.empty() && tr.input.empty();
      return tr.delay.empty() && tr.input.empty() && tr.hasCondition;
    };
    return WhereMut<>(pred);
  }

  TransitionsReference WhereCond() {
    auto pred = [](const ElementType& tr) { return tr.get().hasCondition; };
    return WhereMut<>(pred);
  }

  template <bool Split = false,
            typename Result = std::conditional_t<
                Split, std::pair<TransitionsReference, TransitionsReference>,
                TransitionsReference>>
  Result WhereEvent() {
    auto pred = [](const ElementType& tr) { return !tr.get().input.empty(); };
    return WhereMut<Split>(pred);
  }

  /// Retrieves all transitions that have set timer (delay)
  ///
  /// @remark In case of using Split, first is what passed filtering and second what didn't
  ///
  /// @tparam Split Decides whether to discard what didn't pass filtering or keep it
  /// @tparam Result Depending on Split, is either pair of TransitionsReference or just TransitionsReference
  /// @return Result
  template <bool Split = false,
            typename Result = std::conditional_t<
                Split, std::pair<TransitionsReference, TransitionsReference>,
                TransitionsReference>>
  Result WhereTimer() {
    auto pred = [](const ElementType& tr) { return !tr.get().delay.empty(); };
    return WhereMut<Split>(pred);
  }

  template <typename U>
  TransitionsReference<U> operator&(const TransitionsReference<U>& lhs,
                                    const TransitionsReference<U>& rhs) {
    absl::flat_hash_set<unsigned> lhs_ids;
    lhs_ids.reserve(lhs.size());

    for (const auto& el : lhs) {
      lhs_ids.insert(el.get().Id);
    }

    TransitionsReference result;
    result.container.reserve(std::min(lhs.size(), rhs.size()));

    for (const auto& el : rhs) {
      if (const auto id = el.get().Id; lhs_ids.contains(id)) {
        result.Add(el);
      }
    }
    return result;
  }

  std::optional<ElementType> SmallestTimer() const {
    if (container.empty())
      return std::nullopt;

    auto min = container.front();
    for (auto& el : container) {
      if (el.get().delayInt < min.get().delayInt) {
        min = el;
      }
    }
    return min;
  }

  [[deprecated]]
  auto Delay() {
    auto delayFinder = [](const auto& tr) {
      using RawType = Utils::detail::remove_cvref_t<decltype(tr)>;
      if constexpr (is_smart_ptr<RawType> || std::is_pointer_v<RawType>) {
        return !tr->delay.empty();
      }
      return !tr.delay.empty();
    };
    return WhereMut(delayFinder);
  }

  [[deprecated]]
  auto FilterByState(const std::string_view view) {
    auto stateFinder = [view](const auto& tr) {
      using RawType = Utils::detail::remove_cvref_t<decltype(tr)>;
      if constexpr (is_smart_ptr<RawType> || std::is_pointer_v<RawType>) {
        return tr->from == view;
      }
      return tr.from == view;
    };
    return WhereMut(stateFinder);
  }

  auto TransformMut(
      const std::function<Transition<T>(Transition<T>&)>& transformer) {
    container | ranges::views::for_each(transformer);
    return *this;
  }

  template <typename Action>
  auto ForEach(Action action) {
    ranges::for_each(container, action);
    return *this;
  }

  TransitionsReference operator&(const TransitionsReference& event) const {
    return *this & event;
  }
};

/**
 * @class TransitionGroup
 * @brief Kolekce přechodů s nástroji pro manipulaci.
 */
template <typename T>
class TransitionGroup {
  using TGT = absl::btree_set<Transition<T>>;

 private:
  TGT<T> _transitions;
  absl::flat_hash_map<unsigned, Transition<>> primary{};
  absl::flat_hash_map<std::string, TransitionsReference<T>> index_by_from{};

  TGTMap<T> _transition_groups{};

 public:
  TransitionGroup() = default;
  TransitionGroup operator+=(const Transition<T>& tr) {
    _transitions.insert(tr);

    return *this;
  }
  TransitionGroup& operator<<(const Transition<T>& tr) {
    _transitions.insert(tr);
    return *this;
  }
  explicit TransitionGroup(absl::btree_set<Transition<T>> transitions)
      : _transitions(std::move(transitions)) {}

  [[nodiscard]] size_t Size() const { return _transitions.size(); }
  [[nodiscard]] bool Some() const { return Size() != 0; }
  [[nodiscard]] bool None() const { return Size() == 0; }
  [[nodiscard]] bool IsEmpty() const { return Size() == 0; }

  static TransitionGroup Empty() {
    auto tg = TransitionGroup{};
    tg._transition_groups = {};
    return tg;
  }

  [[deprecated]] auto Mutable() { return *this; }

  void Add(const Transition<T>&& transition) {
    // _transitions.insert(transition);

    //! for now, just use unsigned as IndexT
    // unsigned id = transition.Id;
    primary[transition.Id] = std::move(transition);
    index_by_from[primary.at(transition.Id).from].Add(
        primary.at(transition.Id));
  }

  TGTMap<T> GroupTransitions() {
    TGTMap<T> transitions;
    for (const auto& transition : _transitions) {
      transitions[transition.from].Add(transition);
    }
    _transition_groups = transitions;
    return transitions;
  }
  [[nodiscard]] absl::btree_set<Transition<T>> Get() const {
    return _transitions;
  };
  TransitionGroup Retrieve(const std::string& input) {
    if (_transition_groups.empty())
      GroupTransitions();

    if (_transition_groups.contains(input))
      return _transition_groups.at(input);

    return Empty();
  }

  TransitionsReference<T> RetrieveMut(const std::string_view input) {
    if (index_by_from.empty())
      return TransitionsReference<T>{};  // ERROR or nah?

    if (index_by_from.contains(input))
      return index_by_from.at(input);

    return TransitionsReference<T>{};
  }

  template <typename ResultT>
  TransitionGroup<ResultT> Transform(
      const std::function<Transition<ResultT>(Transition<T>)>& fun) {
    auto it = _transitions | ranges::views::transform(fun);
    const auto transitions = it | ranges::to<absl::btree_set<Transition<T>>>;
    return TransitionGroup<ResultT>(transitions);
  }
  void TransformAction(const std::function<Transition<T>(Transition<T>)>& fun) {
    TGT<T> n;
    for (auto& tr : _transitions) {
      n.insert(fun(tr));
    }
    _transitions = n;
  }

  [[nodiscard]] TransitionsReference<T> WhereMut(
      const std::function<bool(Transition<T>&)>& pred) {
    auto all = primary | ranges::views::values;
    auto f = all | ranges::views::filter(pred);
    auto ref_f = f | ranges::views::transform(
                         [](Transition<T>& t) { return std::ref(t); });
    return ref_f | ranges::to<TransitionsReference<T>>;
  }

  [[nodiscard]] TransitionGroup Where(
      const std::function<bool(Transition<T>)>& pred) const {
    auto all_transitions = primary | ranges::views::values;
    auto it = all_transitions | ranges::views::filter(pred);
    return TransitionGroup(it | ranges::to<decltype(_transitions)>);
  }
  [[nodiscard]] TransitionGroup WhereNoEvent() const {
    return Where([](const auto& tr) { return tr.input.empty(); });
  }
  [[nodiscard]] TransitionGroup WhereEvent() const {
    return Where([](const auto& tr) { return !tr.input.empty(); });
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
  [[nodiscard]] std::optional<Transition<T>> Smallest() const {
    const auto min = WhereTimer().First();
    if (!min.has_value())
      return std::nullopt;
    auto min_v = min.value().get();
    for (auto& tr : _transitions) {
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
  bool Contains(const Transition<T>& tr) {
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
  [[nodiscard]] std::optional<std::reference_wrapper<const Transition<T>>>
  First() const {
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
      ss << tr.to << " ";
    }
    return ss.str();
  }

  [[nodiscard]] auto begin() const {
    return ranges::begin(primary | ranges::views::values);
  }
  [[nodiscard]] auto end() const {
    return ranges::end(primary | ranges::views::values);
  }
  [[nodiscard]] auto empty() const { return _transitions.empty(); }

 private:
  static std::string Format(const std::string_view name,
                            const Transition<T>& transition) {
    auto [from, to, input, cond, delay] = transition.Tuple();
    return absl::StrFormat("%s.Add(%s, %s, %s, [](){ %s; }, %s);", name, from,
                           to, input, Utils::Quote(cond), delay);
  }
};
}  // namespace types