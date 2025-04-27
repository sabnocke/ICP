//
// Created by ReWyn on 27.04.2025.
//

#include "StateMachine.h"
#include <chrono>
#include <functional>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <vector>
using std::string;

struct Transition;

struct State {
  string name;
  std::function<void()> onEnter;
  std::function<void()> onExit;
  std::vector<Transition *> transitions;
};

struct Transition {
  State *from;
  State *to;

  Transition(State *from, State *to) {
    this->from = from;
    this->to = to;
  }
  // std::function<bool()> condition;
  std::optional<string> inputEvent;
  std::optional<string> condition;
  std::optional<int> delayTimeMs;
  // bool isTimeout;
};

class Automaton {
public:
  std::map<std::string, State> states;
  std::vector<Transition> transitions;
  std::map<string, string> variables;

  State *currentState = nullptr;
  bool timerRunning = false;
  std::chrono::steady_clock::time_point timerEnd;

  bool trySetValue(const string &varName, string varValue) {
    if (const auto item = variables.find(varName); item != variables.end()) {
      variables[varName] = std::move(varValue);
      return true;
    }
    return false;
  }
  bool tryGetValue(const string &varName, string &varValue) {
    if (const auto item = variables.find(varName); item != variables.end()) {
      varValue = std::move(variables[varName]);
      return true;
    }
    return false;
  }
};
