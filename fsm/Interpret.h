//
// Created by ReWyn on 10.05.2025.
#pragma once
#include "AutomatLib.h"
#include "types/all_types.h"
#include "timing.h"

namespace Interpreter {
using namespace types;

class Interpret {
  AutomatLib::Automat _automat;
  std::string activeState;
  StateGroup stateGroup;
  TransitionGroup transitionGroup;
  VariableGroup variableGroup;
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
  Timing::Timer timer{};

public:
  Interpret() = default;
  explicit Interpret(AutomatLib::Automat automat);
  static void simpleExample();
  sol::state lua{};

  void Execute(bool once);
};

} // Interpreter
