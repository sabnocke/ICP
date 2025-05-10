//
// Created by ReWyn on 10.05.2025.
//

#include "Interpret.h"

#include <sol/sol.hpp>

#include "absl/log/absl_check.h"


namespace Interpreter {
void Interpret::simpleExample() {
  sol::state lua;
  int x = 0;
  lua.set_function("beep", [&]() { ++x; });
  lua.script("beep()");
  ABSL_CHECK_EQ(x, 1);
}

Interpret::Interpret(AutomatLib::Automat automat) : _automat(std::move(automat)) {
  activeState = _automat.states.First().Name;
  stateGroup = _automat.states;
  variableGroup = _automat.variables;
  transitionGroup = _automat.transitions;
  inputs = _automat.inputs;
  outputs = _automat.outputs;
}

void Interpret::Execute(bool once = false) {
  while (true) {
    // First find all reachable transitions
    auto transitions = transitionGroup.Retrieve(activeState);
    // Find all free transitions
    if (auto r = transitionGroup.WhereNone(); r.Some()) {
      // move and continue
    }
    // Find all transitions that have condition, but no input
    if (auto r = transitions.WhereCond().WhereNoTimer(); r.Some()) {
      // arm timers for all transitions
    }
    // wait for timers (can do) or input event (can't do)
    // run timers
  }
}

}  // namespace Interpreter