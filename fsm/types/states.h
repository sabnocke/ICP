#pragma once
#include <range/v3/view.hpp>
#include <string>
#include <vector>

#include "absl/strings/str_format.h"

namespace types {
struct State {
  std::string Name;
  std::string Action;

  bool operator==(const State &state) const {
    return Name == state.Name && Action == state.Action;
  }
  bool operator!=(const State &state) const { return !(*this == state); }
  bool operator<(const State &state) const {
    return Name < state.Name && Action < state.Action;
  }
  friend std::ostream &operator<<(std::ostream &os, const State &state) {
    os << absl::StrFormat("State: Name: %s; Action: %s", state.Name, state.Action);
    return os;
  }
};

struct StateGroup {
private:
  std::vector<State> states;

public:
  StateGroup() = default;
  explicit StateGroup(std::vector<State> states) : states(std::move(states)) {}
  [[nodiscard]] std::vector<std::string> GetNames() const {
    auto it = states | ranges::views::transform([](const State& state) {return state.Name;});
    return it | ranges::to<std::vector<std::string>>;
  }
  [[nodiscard]] std::vector<std::string> GetActions() const {
    auto it = states | ranges::views::transform([](const State& state) {return state.Action;});
    return it | ranges::to<std::vector<std::string>>;
  }
  [[nodiscard]] std::vector<std::pair<std::string, std::string>> GetPairs() const {
    auto it = states | ranges::views::transform([](const State& state){return std::make_pair(state.Name, state.Action);});
    return it | ranges::to<std::vector<std::pair<std::string, std::string>>>;
  }
  State First() {
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

    return StateGroup{std::move(s)};
  }
  size_t Size() const {
    return states.size();
  }
  StateGroup Add(const State &state) {
    states.emplace_back(state);
    return *this;
  }

  StateGroup &operator<<(const State &state) {
    states.emplace_back(state);
    return *this;
  }

  friend std::ostream& operator<<(std::ostream& os, const StateGroup &state) {
    for (const State &s : state.states) {
      os << s << std::endl;
    }
    return os;
  }
};
}