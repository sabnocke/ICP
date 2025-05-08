//
// Created by ReWyn on 28.04.2025.
//

#pragma once

#include <functional>
#include <optional>
#include <string>
#include <sstream>

// #include "absl/synchronization/internal/waiter_base.h"

namespace Parser {
  template<typename StringType>
  struct TransitionRecord;
  template<typename StringType>
  struct VariableRecord;
}

namespace AutomatLib {
  struct Transition {
    std::string from;
    std::string to;
    std::string input;
    std::string cond;
    std::string delay;
  };
  struct State {
    std::string name;
    std::string action;
  };

  class Automat {
  public:
    [[deprecated("Use tuple version")]] void addState(const std::string &name, const std::string &action);
    void addState(const std::tuple<std::string, std::string> &result);
    [[deprecated("Use TransitionRecord")]] void addTransition(const std::string &from, const std::string &to, const std::string &input, const std::string &cond, const std::string &delay);
    template<typename T>
    void Automat::addTransition(const Parser::TransitionRecord<T> &result);
    [[deprecated("Use VariableRecord")]] void addVariable(const std::string &type, const std::string &name,
                                                          const std::string &value);
    template<class T>
    void addVariable(const Parser::VariableRecord<T> &result);
    void addInput(const std::string &name);
    void addOutput(const std::string &name);
    // template<typename T>
    // void addVariable(Parser::VariableRecord<T> &result);
    [[deprecated("Will be moved elsewhere")]] std::optional<std::string> valueof(const std::string &name);
    template <typename T>
    [[deprecated("Will be moved elsewhere")]] bool output(const std::string &outputName, const T &value);
    std::string Name;
    std::string Comment;
    void PrepareHelperVariables();
    void PrepareIncludes();
  private:
    std::unordered_map<std::string, State> states;
    std::vector<Transition> transitions;
    std::vector<std::string> inputs;
    std::vector<std::string> outputs;
    // std::unordered_map<std::string, std::string> inputs;
    // std::unordered_map<std::string, std::string> outputs;
    std::unordered_map<std::string, std::pair<std::string, std::string>> variables;
    std::string currentState;
    bool firstRun = true;
    std::ostringstream oss;
  };

}
