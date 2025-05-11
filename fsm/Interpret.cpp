//
// Created by ReWyn on 10.05.2025.
//

#include "Interpret.h"

#include <thread>

#include "absl/log/absl_check.h"
#include "external/sol.hpp"
#include <re2/re2.h>

namespace Interpreter {
void Interpret::simpleExample() {
  sol::state lua;
  int x = 0;
  lua.set_function("beep", [&]() { ++x; });
  lua.script("beep()");
  ABSL_CHECK_EQ(x, 1);
}

Interpret::Interpret(AutomatLib::Automat automat)
    : _automat(std::move(automat)) {
  activeState = _automat.states.First().Name;
  stateGroup = _automat.states;
  variableGroup = _automat.variables;
  transitionGroup = _automat.transitions;
  inputs = _automat.inputs;
  outputs = _automat.outputs;
  sol::state lua = std::move(automat.lua);
}

void Interpret::Execute(bool once = false) {
  while (true && !once) {
    // First find all reachable transitions
    std::cout << activeState << std::endl;
    auto transitions = transitionGroup.Retrieve(activeState);
    std::cout << transitions.Out() << std::endl;
    // Find all free transitions
    if (auto r = transitionGroup.WhereNone(); r.Some()) {
      // move and continue
      if (auto t = r.First(); t.has_value()) {
        activeState = t.value().get().to;
        std::cout << "STATE: " << activeState << std::endl;
      }
      continue;
    }
    auto transitionsCond = transitions.WhereCond();
    // Find all transitions that have valid condition, no timer and no event
    if (auto r = transitionsCond.WhereNoTimer().WhereNoEvent(); r.Some()) {
      if (auto t = r.First(); t.has_value()) {
        activeState = t.value().get().to;
        std::cout << "STATE: " << activeState << std::endl;
      }
    }
    // Find all transitions that have condition, but no input
    if (auto r = transitionsCond.WhereNoEvent(); r.Some()) {
      // set timer for smallest delay transition
      /*
      * can the timer influence where to go?
      * if timer starts and input is registered should i continue with timer or input?
      * current implementation uses simple approach, once timer is set it is decided
      */
      /*
      * this is a trick as according to above timer is decisive and
      * at any time there would be more than one timer
      * and both are started at the same time, the timer with smaller delay will always end first.
      *
      * because of these two assumptions, I am allowed to do this
      */
      if (auto smallest = r.Smallest(); smallest.has_value()) {
        auto duration = std::chrono::milliseconds(smallest.value().delayInt);
        std::this_thread::sleep_for(duration);
        activeState = smallest.value().to;
        std::cout << "STATE: " << activeState << std::endl;
        continue;
      }
    }
    if (auto r = transitionsCond.WhereEvent(); r.Some()) {
      // "INPUT: in1 = 1"
      std::string line;
      std::string signalName, sigVal;
      std::getline(std::cin, line);
      if (!RE2::FullMatch(line, R"(INPUT: (?<sig>\w+) = (?<val>\w+))", &signalName, &sigVal)) {
        std::cerr << "ERROR: " << line << std::endl;
        continue;
      }
      std::cerr << line << " " << signalName << " " << sigVal << std::endl;
      lua[signalName] = sigVal;
      if (Utils::Contains(line, "STOP")) {
        break;
      }
      if (auto r1 = transitionsCond.WhereNoTimer(); r.Some()) {
        // All transitions with input but no timer
        auto inputs = r1.Where([signalName](const Transition& tr){return tr.input == signalName;});
        if (auto f = inputs.First(); f.has_value()) {
          activeState = f.value().get().to;
          std::cout << "STATE: " << activeState << std::endl;
          continue;
        }
      }
      if (auto r2 = transitionsCond.WhereTimer(); r2.Some()) {
        // All transitions with input and timer
        auto inputs = r2.Where([signalName](const Transition& tr){return tr.input == signalName;});
        if (auto f = inputs.First(); f.has_value()) {
          activeState = f.value().get().to;
          std::cout << "STATE: " << activeState << std::endl;
        }
      }
    }

    // Find all
    // wait for timers (can do) or input event (can't do)
    // run timers
    // auto result = timer.Start();
    // if (result.has_value()) {
    //   // move there?
    // }
  }
}

}  // namespace Interpreter