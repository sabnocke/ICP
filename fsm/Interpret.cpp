//
// Created by ReWyn on 10.05.2025.
//

#include "Interpret.h"

#include <re2/re2.h>

#include <thread>

#include "Utils.h"
#include "absl/log/absl_check.h"
#include "absl/strings/match.h"
#include "external/sol.hpp"

//TODO change the transition (and potentially states) to functions, compile them ahead via lua.load(...)
//TODO and store the as protected_functions in some friend container (TransitionGroup if remade with generics is good contender)
//TODO same for state actions
//TODO remove macros, they are C filth
//TODO maybe try to split the Execute logic
//TODO try to implement ability to watch multiple events (input, timer), try to figure out rxcpp (or something similar)
//TODO remove the std::move for lua state, state will be build directly in interpreter
//TODO do a more robust error detection and recovery
//TODO ^ for all parts of project (automat, parser, interpreter)
//TODO try implementing parsing for code blocks ([] on multiple lines); should be question of parsing
//TODO fast_float could either be moved to vcpkg or entirely removed, depending on how replaceable it is with std::from_chars

#define LUA_SCRIPT(out, code)           \
  try {                                 \
    lua.script(code);                   \
  } catch (const std::exception& e) {   \
    std::cerr << e.what() << std::endl; \
  }

namespace Interpreter {
void Interpret::simpleExample() {
  sol::state lua;
  int x = 0;
  if (const auto res = lua.safe_script("return 0 == 0"); res.valid()) {
    std::cout << res.get<bool>() << std::endl;
  }
}

template <typename T>
std::optional<T> TestAndSetValue(const std::string& _value) {
  if constexpr (std::is_same_v<T, std::string>) {
    return _value;
  }
  if constexpr (Utils::detail::IsNumeric<T>) {
    return Utils::StringToNumeric<T>(_value);
  }
  return std::nullopt;
}

Interpret::Interpret(AutomatLib::Automat& automat) {
  activeState = automat.states.First().Name;
  stateGroup = automat.states;
  variableGroup = automat.variables;
  transitionGroup = automat.transitions;
  inputs = automat.inputs;
  outputs = automat.outputs;
  sol::state lua = std::move(automat.lua);
}

void Interpret::ChangeState(const TransitionGroup& tg) {
  if (const auto t = tg.First(); t.has_value()) {
    activeState = t.value().get().to;
    std::cout << "STATE: " << activeState << std::endl;
    const auto [Name, Action] = stateGroup.Find(activeState).First();
    try {
      auto result = lua.script(Action);
    } catch (const std::exception& e) {
      std::cerr << e.what() << std::endl;
    }
  }
}

void Interpret::LinkDelays() {
  auto hasDelay = transitionGroup.WhereTimer();

  if (hasDelay.None())
    return;

  for (Variable var : variableGroup.Get()) {
    auto mod = hasDelay.Where(
        [&](const Transition& tr) { return tr.delay == var.Name; });
    if (absl::EqualsIgnoreCase(var.Type, "int")) {
      if (auto val = TestAndSetValue<int>(var.Value); val.has_value()) {
        mod.TransformAction([&](const Transition& tr) {
        Transition ntr(tr);
        ntr.delayInt = std::abs(val.value());
        return ntr;
      });
      transitionGroup.Merge(mod);
      }
    } else {
      //* There really isn't much of a reason to have to automat wait for more than int MAX_VALUE
      //* BTW, int MAX_VALUE = 2,147,483,647 => cca. 24 days
      std::cerr << absl::StrFormat(
                       "Expected delay of type 'int' but received '%s'",
                       var.Type)
                << std::endl;
    }
  }
}

void Interpret::PrepareVariables() {
  for (const auto& variable : variableGroup.Get()) {
    auto [Type, Name, Value] = variable.Tuple();
    std::cerr << "Variable: " << Type << " " << Name << std::endl;

    if (absl::EqualsIgnoreCase(Type, "int")) {
      if (auto val = TestAndSetValue<int>(Value); val.has_value())
        lua[Name] = val.value();
    } else if (absl::EqualsIgnoreCase(Type, "float")) {
      if (auto val = TestAndSetValue<float>(Value); val.has_value())
        lua[Name] = val.value();
    } else if (absl::EqualsIgnoreCase(Type, "double")) {
      if (auto val = TestAndSetValue<double>(Value); val.has_value())
        lua[Name] = val.value();
      else
        lua[Name] = Value;
    } else if (absl::EqualsIgnoreCase(Type, "bool") ||
               absl::EqualsIgnoreCase(Type, "string")) {
      lua[Name] = Value;
    }
  }
}

void Interpret::PrepareTransitions() {
  TransitionGroup ntg;
  for (auto& transition : transitionGroup) {
    auto ntr = Transition(transition);
    if (transition.cond.empty())
      continue;
    // ntr.cond = absl::StrFormat("return %s", transition.cond);

    ntg.Add(ntr);
  }
  transitionGroup = ntg;
}

void Interpret::PrepareSignals() {
  for (auto& signal : inputs) {
    // auto c = absl::StrFormat("%s = ''", signal);
    lua[signal] = "";
  }
  std::cout << std::endl;
  for (auto& signal : outputs) {
    // auto c = absl::StrFormat("%s = ''", signal);
    lua[signal] = "";
  }
}

void Interpret::Prepare() {
  LinkDelays();
  PrepareVariables();
  PrepareTransitions();
  PrepareSignals();
}

bool StringToBool(const std::string& str) {
  const auto lower = Utils::ToLower(str);
  std::istringstream stream(lower);
  bool val;
  stream >> std::boolalpha >> val;
  return val;
}

int Interpret::Execute(bool once = false) {
  lua.open_libraries(sol::lib::base);
  try {
    while (true && !once) {
      // First find all reachable transitions
      std::cout << "Active state: " << activeState << std::endl;
      auto transitions = transitionGroup.Retrieve(activeState);
      std::cout << "Possible next states: " << std::endl
                << transitions.Out() << std::endl;
      // Find all free transitions
      if (auto r = transitionGroup.WhereNone(); r.Some()) {
        // move and continue
        ChangeState(r);
        continue;
      }
      auto transitionsCond = transitions.WhereCond();

      for (auto& transition : transitionsCond) {
        std::string cond = transition.cond;
        std::cout << cond << std::endl;
        if (auto result = lua.safe_script(cond); result.valid()) {
          std::cout << result.get<std::string>() << std::endl;
        }
      }

      // Find all transitions that have valid condition, no timer and no event
      // if (auto r = transitionsCond.WhereNoTimer().WhereNoEvent(); lua_out && r.Some()) {
      //   ChangeState(r);
      //   continue;
      // }
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
          ChangeState(r);
          continue;
        }
      }
      if (auto r = transitionsCond.WhereEvent(); r.Some()) {
        // "INPUT: in1 = 1"
        std::string line;
        std::string signalName, sigVal;
        std::getline(std::cin, line);
        if (Utils::Contains(line, "input") &&
            !RE2::FullMatch(line, R"(INPUT: (?<sig>\w+) = (?<val>\w+))",
                            &signalName, &sigVal)) {
          std::cerr << "ERROR: " << line << std::endl;
          continue;
        }
        if (Utils::Contains(line, "STOP")) {
          return 0;
        }
        std::cerr << line << " " << signalName << " " << sigVal << std::endl;
        try {
          lua[signalName] = sigVal;
        } catch (const std::exception& e) {
          std::cerr << e.what() << std::endl;
        }
        try {
          std::string smt = lua[signalName];
          std::cout << smt << std::endl;
        } catch (const std::exception& e) {
          std::cerr << e.what() << std::endl;
        }

        std::cout << transitionsCond.WhereNoTimer() << std::endl;

        if (auto r1 = transitionsCond.WhereNoTimer(); r.Some()) {
          // All transitions with input but no timer
          auto inputs = r1.Where([signalName](const Transition& tr) {
            return tr.input == signalName;
          });
          std::cerr << inputs.Out() << std::endl;
          ChangeState(inputs);
          continue;
        }
        if (auto r2 = transitionsCond.WhereTimer(); r2.Some()) {
          // All transitions with input and timer
          auto inputs = r2.Where([signalName](const Transition& tr) {
            return tr.input == signalName;
          });
          std::cerr << inputs.Out() << std::endl;
          ChangeState(inputs);
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
  } catch (const std::exception& e) {
    std::cerr << "ERROR: " << e.what() << std::endl;
    return 1;
  }
  return 0;
}

}  // namespace Interpreter