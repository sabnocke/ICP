//
// Created by ReWyn on 10.05.2025.
#pragma once
#include "AutomatLib.h"
#include "types/all_types.h"


namespace Interpreter {
using namespace types;

class Interpret {
  AutomatLib::Automat _automat;
  std::string activeState;
  StateGroup stateGroup;

  VariableGroup variableGroup;
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
  void ChangeState(const TransitionGroup& tg);
  void LinkDelays();
  void PrepareVariables();
  void PrepareTransitions();
  void PrepareSignals();

 public:
  TransitionGroup transitionGroup;
  void Prepare();
  Interpret() = default;
  // explicit Interpret(AutomatLib::Automat automat);
  static void simpleExample();
  explicit Interpret(AutomatLib::Automat& automat);
  sol::state lua{};

  int Execute(bool once);
};

} // Interpreter
