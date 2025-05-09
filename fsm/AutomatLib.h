#pragma once

#include <absl/strings/str_format.h>

#include <functional>
#include <ostream>
#include <sstream>
#include <string>

#include "ParserLib.h"

namespace ParserLib {
struct TransitionRecord;
struct VariableRecord;
}  // namespace ParserLib

namespace AutomatLib {
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

  bool operator==(const Transition &transition) const {
    return from == transition.from && to == transition.to &&
           input == transition.input && cond == transition.cond &&
           delay == transition.delay;
  }
  bool operator!=(const Transition &transition) const {
    return !(*this == transition);
  }
  bool operator<(const Transition &other) const {
    return std::tie(from, to, input, cond, delay) <
           std::tie(other.from, other.to, other.input, other.cond, other.delay);
  }
  friend std::ostream &operator<<(std::ostream &os,
                                  const Transition &transition) {
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
struct State {
  std::string name;
  std::string action;
};

class Automat {
 public:
  void addState(const std::tuple<std::string, std::string> &result);
  void addTransition(const ParserLib::TransitionRecord &result);
  void addVariable(const ParserLib::VariableRecord &result);
  void addInput(const std::string &name);
  void addOutput(const std::string &name);
  std::string Name;
  std::string Comment;
  void PrepareHelperVariables();
  void PrepareIncludes();
  std::unordered_map<std::string, State> states;
  std::vector<Transition> transitions;
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
  std::unordered_map<std::string, std::pair<std::string, std::string>>
      variables;
  std::string currentState;
  bool firstRun = true;
  std::ostringstream oss;

 private:
};

}  // namespace AutomatLib
