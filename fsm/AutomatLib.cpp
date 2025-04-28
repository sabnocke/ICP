//
// Created by ReWyn on 28.04.2025.
//

#include "AutomatLib.h"
#include <bits/ios_base.h>
#include <chrono>
#include <functional>
#include <iostream>
#include <optional>
#include <ostream>
#include <string>
#include <vector>


// #include "Stopwatch.h" // will be useful later or elsewhere
using std::string;
using namespace std::chrono;

void Automat::addState(const string &name, std::function<void()> action) {
  states[name] = State{name, action};
  if (firstRun) {
    firstRun = false;
    currentState = name;
  }
}

void Automat::addTransition(const string &from, const string &to, std::function<bool()> condition, const int timeoutMs) {
  transitions.push_back({from, to, condition, timeoutMs});
}

void Automat::runOnce() {
  if (states.find(currentState) == states.end()) {
    std::cerr << "AutomatLib::runOnce(): State not found" << std::endl; //TODO this should output to gui
    return;
  }

  for (const auto &transition : transitions) {
    if (transition.from == currentState && transition.condition()) {
      std::cout << "Transition from: " << transition.from << " to " << transition.to << std::endl;
      currentState = transition.to;
      states[currentState].action();
      return;
    }
  }
}

void Automat::setInput(const std::string &name, const string &value) {
  inputs[name] = value;
}
std::string Automat::getInput(const std::string &name) {
  return inputs[name];
}

std::optional<std::string> Automat::valueof(const std::string &name) {
  if (const auto input = inputs.find(name); input != inputs.end()) {
    return input->second;
  }
  return std::nullopt;
}