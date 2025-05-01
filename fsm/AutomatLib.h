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
  std::function<void()> condition;
  int timeoutMs;
};
struct State {
  std::string name;
  std::function<void()> action;
};

class Automat {
public:
  void addState(const std::string &name, std::function<void()> action);
  void addTransition(const std::string &from, const std::string &to, std::function<bool()> condition, int timeoutMs = 0);
  void runOnce();
  void setInput(const std::string &name, const std::string &value);
  void addVariable(const std::string &type, const std::string &name, const std::string &value);
  std::string getInput(const std::string &name);
  std::optional<std::string> valueof(const std::string &name);
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

