#include "AutomatLib.h"

#include <absl/strings/match.h>

#include <fstream>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "../vcpkg/buildtrees/abseil/src/20250127.1-a0a219bf72.clean/absl/strings/match.h"
#include "ParserLib.h"
#include "Utils.h"
#include "range/v3/view/map.hpp"
#include "types/all_types.h"

#define TestAndSet(type, name, _value)                      \
  do {                                                      \
    auto n = Utils::StringToNumeric<type>(_value);          \
    if (auto i = n.Get<type>(); i.has_value()) {            \
      std::cout << "New value: " << i.value() << std::endl; \
      lua[name] = i.value();                                \
    }                                                       \
  } while (0)

#define TestAndSetValue(assign, type, _value)      \
  do {                                             \
    auto n = Utils::StringToNumeric<type>(_value); \
    if (auto i = n.Get<type>(); i.has_value()) {   \
      assign = i.value();                          \
    }                                              \
  } while (0)

namespace AutomatLib {

void Automat::addTransition(const Transition &result) { transitions << result; }

void Automat::addInput(const std::string &name) { inputs.push_back(name); }

void Automat::addOutput(const std::string &name) { outputs.push_back(name); }

void Automat::PrepareUtilsFunctions() {
  lua.script(R"(
  function template(init)
    local value = init
    function get_set(...)
      local args = select('#', ...)
      if args > 0 then
        arg = select(1, ...)
        value = arg
        return arg
      else
        return value
      end
    end

    return get_set
  end
)");
}

void Automat::LinkDelays() {
  auto hasDelay = transitions.WhereTimer();

  if (hasDelay.None())
    return;

  for (Variable var : variables.Get()) {
    auto mod = hasDelay.Where(
        [&](const Transition &tr) { return tr.delay == var.Name; });
    if (absl::EqualsIgnoreCase(var.Type, "int")) {
      int val;
      TestAndSetValue(val, int, var.Value);
      mod.TransformAction([&](const Transition &tr) {
        Transition ntr(tr);
        ntr.delayInt = std::abs(val);
        return ntr;
      });
      transitions.Merge(mod);
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

void Automat::PrepareSignals() {
  for (auto &signal : inputs) {
    auto c = absl::StrFormat("%s = template(\"\")", signal);
    lua.script(c);
  }
  std::cout << std::endl;
  for (auto &signal : outputs) {
    auto c = absl::StrFormat("%s = template(\"\")", signal);
    lua.script(c);
  }
}

void Automat::PrepareStateActions() const {
  for (const auto &[Name, Action] : states.GetPairs()) {
    //     function template(init)
    //      local value = init
    //      function get_set(...)
    //        local args = select('#', ...)
    //        if args > 0 then
    //          arg = select(1, ...)
    //          value = arg
    //          return arg
    //        else
    //          return value
    //        end
    //      end
    //
    //      return get_set
    //    end
    // any signal then can be made like this: signalName = template("")
    // with getting the value like this: signalName()
    // and setting it like this: signalName(<value>)
    if (Action.empty())
      continue;
    std::cerr << "New action: " << Action << "for state: " << Name << std::endl;
  }
}

void Automat::PrepareVariables() {
  //TODO requires lua/sol
  for (const auto &variable : variables.Get()) {
    auto [Type, Name, Value] = variable.Tuple();
    std::cerr << "Variable: " << Type << " " << Name << std::endl;
    if (absl::EqualsIgnoreCase(Type, "int")) {
      TestAndSet(int, Name, Value);
    } /*else if (absl::EqualsIgnoreCase(Type, "float")) {
      TestAndSet(float, Name, Value);
    }*/
    else if (absl::EqualsIgnoreCase(Type, "double")) {
      TestAndSet(double, Name, Value);
    } else if (absl::EqualsIgnoreCase(Type, "bool") ||
               absl::EqualsIgnoreCase(Type, "string")) {
      lua[Name] = Value;
    }
  }
}

void Automat::PrepareHelperVariables() {
}

void Automat::PrepareExecuteFunction() {
}

void Automat::Create() {
}

}  // namespace AutomatLib