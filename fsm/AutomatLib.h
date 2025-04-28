//
// Created by ReWyn on 28.04.2025.
//

#pragma once

#include <functional>
#include <optional>
#include <string>
using std::string;

struct Transition {
  string from;
  string to;
  std::function<void()> condition;
  int timeoutMs;
};
struct State {
  string name;
  std::function<void()> action;
};

class Automat {
public:
  void addState(const string &name, std::function<void()> action);
  void addTransition(const string &from, const string &to, std::function<bool()> condition, int timeoutMs = 0);
  void runOnce();
  void setInput(const std::string &name, const string &value);
  std::string getInput(const std::string &name);
  std::optional<std::string> valueof(const std::string &name);

private:
  std::unordered_map<string, State> states;
  std::vector<Transition> transitions;
  std::unordered_map<string, string> inputs;
  std::unordered_map<string, string> outputs;
  string currentState;
  bool firstRun = true;
};

