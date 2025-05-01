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
#include <utility>
#include <vector>

#include "Utils.h"


// #include "Stopwatch.h" // will be useful later or elsewhere
using namespace std::chrono;

void Automat::addState(const std::string &name, const std::string &action) {
  states[name] = State{name, action};
}

void Automat::addTransition(const std::string &from, const std::string &to, const std::string &input, const std::string &cond, const std::string &delay) {
  transitions.push_back({from, to, input, cond, delay});
}

void Automat::addVariable(const std::string &type, const std::string &name, const std::string &value) {
  variables[name] = std::make_pair(type, value);
}

std::optional<std::string> Automat::valueof(const std::string &name) {
  if (inputs.contains(name)) {
    return inputs[name];
  }
  return std::nullopt;
}

template <typename T>
bool Automat::output(const std::string &outputName, const T &value) {
  if (outputs.contains(outputName)) {
    outputs[outputName] = Utils::ToStringOpt(value);
    return true;
  }
  return false;
}