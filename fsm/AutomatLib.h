//
// Created by ReWyn on 28.04.2025.
//

#pragma once

#include <functional>
#include <optional>
#include <string>


struct Transition {
  std::string from;
  std::string to;
  std::string input;
  std::string cond;
  std::string delay;
};
struct State {
  std::string name;
  // std::function<void()> action;
  std::string action;
};

class Automat {
public:
  void addState(const std::string &name, const std::string &action);
  void addTransition(const std::string &from, const std::string &to, const std::string &input, const std::string &cond, const std::string &delay);
  void addVariable(const std::string &type, const std::string &name, const std::string &value);
  std::optional<std::string> valueof(const std::string &name);
  template <typename T>
  bool Automat::output(const std::string &outputName, const T &value);
  std::string Name;
  std::string Comment;
private:
  std::unordered_map<std::string, State> states;
  std::vector<Transition> transitions;
  std::unordered_map<std::string, std::string> inputs;
  std::unordered_map<std::string, std::string> outputs;
  std::unordered_map<std::string, std::pair<std::string, std::string>> variables;
  std::string currentState;
  bool firstRun = true;
};

