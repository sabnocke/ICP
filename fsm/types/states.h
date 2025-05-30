#pragma once
#include <range/v3/view.hpp>
#include <string>
#include <vector>

#include "../external/sol.hpp"
#include "absl/strings/str_format.h"

namespace types {
template <typename T = std::string>
struct State {
  static_assert(std::disjunction_v<std::is_same<T, std::string>,
                                   std::is_same<T, sol::protected_function>>,
                "State can only be configured with either 'string' or "
                "'protected_function");
  using ContainedType = T;
  std::string Name;
  T Action;

  bool operator==(const State &state) const {
    return Name == state.Name && Action == state.Action;
  }
  bool operator!=(const State &state) const { return !(*this == state); }
  bool operator<(const State &state) const {
    return Name < state.Name && Action < state.Action;
  }
  friend std::ostream &operator<<(std::ostream &os, const State &state) {
    os << absl::StrFormat("State: Name: %s; Action: %s", state.Name,
                          state.Action);
    return os;
  }
  State() = default;
  State(const State& other) = default;
  State(State&& other) noexcept = default;
  State(std::string &&name, std::string &&action)
      : Name(std::move(name)), Action(std::move(action)) {}

  State& operator=(const State &state) = default;
  State& operator=(State &&state) noexcept {
    if (this != &state) {
      Name = std::move(state.Name);
      Action = std::move(state.Action);
    }
    return *this;
  }
};

template <typename T = std::string>
struct StateGroup {
 private:
  std::vector<State<T>> states;
  std::vector<std::shared_ptr<State<T>>> secondary;

 public:
  StateGroup() = default;
  explicit StateGroup(std::vector<State<T>> states)
      : states(std::move(states)) {}
  StateGroup(StateGroup &&group) noexcept
      : states(std::move(group.states)) {}
  StateGroup(const StateGroup &group) noexcept
      : states(group.states) {}

  void swap(StateGroup& other) noexcept{
    using std::swap;
    swap(states, other.states);
  }

  StateGroup &operator=(const StateGroup &group) {
    if (this != &group) {
      StateGroup tmp(group);
      swap(tmp);
    }
    return *this;
  }
  StateGroup &operator=(StateGroup &&other) noexcept {
    if (this != &other) {
      swap(other);
    }
    return *this;
  }

  [[nodiscard]] std::vector<std::string> GetNames() const {
    auto it = states | ranges::views::transform(
                           [](const State<T> &state) { return state.Name; });
    return it | ranges::to<std::vector<std::string>>;
  }
  [[nodiscard]] std::vector<std::string> GetActions() const {
    auto it = states | ranges::views::transform(
                           [](const State<T> &state) { return state.Action; });
    return it | ranges::to<std::vector<std::string>>;
  }
  [[nodiscard]] std::vector<std::pair<std::string, std::string>> GetPairs()
      const {
    auto it = states | ranges::views::transform([](const State<T> &state) {
                return std::make_pair(state.Name, state.Action);
              });
    return it | ranges::to<std::vector<std::pair<std::string, std::string>>>;
  }

  template <typename ResultT>
  StateGroup<ResultT> Transform(std::function<State<ResultT>(State<T>)> fun) {
    auto it = states | ranges::views::transform(fun);
    const auto new_states = it | ranges::to<std::vector<State<ResultT>>>;
    return StateGroup<ResultT>(new_states);
  }

  StateGroup Find(std::string name) const {
    auto it = states | ranges::views::filter([name](const State<T> &state) {
                return state.Name == name;
              });
    return StateGroup(it | ranges::to<std::vector<State<T>>>());
  }

  [[nodiscard]] State<T> First() const {
    if (states.empty()) {
      return {};
    }
    return states.front();
  }

  StateGroup Rest() {
    if (states.empty()) {
      return {};
    }
    std::vector s(states.begin() + 1, states.end());

    return StateGroup{s};
  }
  [[nodiscard]] size_t Size() const { return states.size(); }

  StateGroup& Add(const State<T> &state) {
    states.emplace_back(state);
    return *this;
  }

  // StateGroup& AddShared()

  StateGroup &operator<<(const State<T> &state) {
    states.emplace_back(state);
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os, const StateGroup &state) {
    for (const State<T> &s : state.states) {
      os << s << std::endl;
    }
    return os;
  }
  [[nodiscard]] auto begin() { return states.begin(); }
  [[nodiscard]] auto end() { return states.end(); }

  [[nodiscard]] auto cbegin() const { return states.cbegin(); }
  [[nodiscard]] auto cend() const { return states.cend(); }

  [[nodiscard]] auto empty() const { return states.empty(); }
};
}  // namespace types