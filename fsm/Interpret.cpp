#include "Interpret.h"

#include <absl/log/log.h>
#include <absl/strings/match.h>
#include <absl/strings/str_join.h>
#include <absl/time/time.h>
#include <re2/re2.h>

#include <algorithm>
#include <thread>
#include <variant>

#include "Utils.h"
#include "external/sol.hpp"

#pragma region counter init
std::atomic<unsigned> types::Transition::counter = 0;
#pragma endregion

namespace Interpreter {

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

Interpret::Interpret(const AutomatLib::Automat& automat) {
  transitionGroup.primary.reserve(automat.transitions.Size() +
                                  5);  // Some reserve

  transitionGroup = automat.transitions;
  stateGroup = automat.states;
  variableGroup = automat.variables;
  inputs = automat.inputs;
  outputs = automat.outputs;
  activeState = stateGroup.First().Name;

  lua.open_libraries(sol::lib::base);
  lua["elapsed"] = [&]() { return timer.elapsed<>(); };
  if (const auto file = lua.script_file("stdlib.lua"); !file.valid()) {
    const sol::error err = file;
    LOG(ERROR) << absl::StrFormat("Failed to open stdlib.lua: %v", err.what());
    throw Utils::ProgramTermination();
  }
}

Interpret::InterpretedValue Interpret::InterpretResult(
    const sol::object& result) {
  if (result.is<bool>()) {
    return result.as<bool>();
  }
  if (result.is<int>()) {
    return result.as<int>();
  }
  if (result.is<double>()) {
    return result.as<double>();
  }
  if (result.is<std::string>()) {
    return result.as<std::string>();
  }
  if (result.is<sol::table>()) {
    return std::monostate{};
  }
  if (result.is<sol::function>()) {
    return std::monostate{};
  }
  if (result.is<sol::nil_t>()) {
    return std::monostate{};
  }
  LOG(ERROR) << "Unknown result type";
  throw Utils::ProgramTermination();
}

void Interpret::ChangeState(const TransitionGroup& tg) {
  timer.tock();

  const auto cond_true = WhenConditionTrue(tg);

  if (const auto t = cond_true.First(); t.has_value()) {
    activeState = t.value().to;
  } else {
    LOG(ERROR) << "No next state found, but expected one";
    throw Utils::ProgramTermination();
  }
  std::cout << "STATE: " << activeState << std::endl;
  const auto [Name, Action] = stateGroupFunction.Find(activeState).First();
  if (const auto result = Action(); result.valid()) {
    const auto r = sol::object(result[0]);
    const auto i_result = InterpretResult(r);
    std::visit(
        Utils::detail::Overloaded{
            [](const std::string& val) {
              std::cout << "OUTPUT (string): " << val << std::endl;
            },
            [](const bool val) {
              std::cout << "OUTPUT (bool): " << val << std::endl;
            },
            [](const int val) { std::cout << "OUTPUT: " << val << std::endl; },
            [](const double val) {
              std::cout << "OUTPUT: " << val << std::endl;
            },
            [](std::monostate val) {
              LOG(ERROR) << "Result interpretation failed";
              throw Utils::ProgramTermination();
            }},
        i_result);
  } else {
    const sol::error err = result;
    LOG(ERROR) << err.what();
    throw Utils::ProgramTermination();
  }
}

bool Interpret::WaitShortestTimer(const TransitionGroup& group) {
  std::cerr << "Waiting for shortest timer..." << std::endl;
  std::cerr << group << std::endl;
  if (const auto shortest = group.SmallestTimer(); shortest.has_value()) {
    const auto duration =
        std::chrono::milliseconds(shortest.value().delayInt);
    std::cerr << absl::StrFormat("Going to sleep at %s\n", absl::FormatTime(absl::Now(), absl::UTCTimeZone()));
    std::this_thread::sleep_for(duration);
    std::cerr << absl::StrFormat("Returned from sleep at %s\n", absl::FormatTime(absl::Now(), absl::UTCTimeZone()));

    ChangeState(group);
    return true;
  }
  return false;
}

void Interpret::LinkDelays() {
  auto hasDelay = transitionGroup.Where(
      [](const Transition& tr) { return !tr.delay.empty(); });

  if (hasDelay.None()) {
    LOG(INFO) << "No delay found";
    return;
  }

  for (auto& v : variableGroup) {
    auto mod = hasDelay.Where(
        [v](const Transition& tr) { return tr.delay == v.Name; });
    if (mod.None())
      continue;

    if (auto val = TestAndSetValue<int>(v.Value); val.has_value()) {
      for (auto& [_, transition] : mod) {
        transition.delayInt = val.value();
      }
    }
    transitionGroup = transitionGroup.Merge(mod);
  }

  for (auto& [id, tr]: hasDelay) {
    if (tr.delayInt != 0)
      continue;

    if (auto val = Utils::StringToNumeric<int>(tr.delay); val.has_value()) {
      tr.delayInt = val.value();
    } else {
      continue;
    }
  }
  transitionGroup = transitionGroup.Merge(hasDelay);

}

void Interpret::PrepareVariables() {
  for (const auto& variable : variableGroup.Get()) {
    auto [Type, Name, Value] = variable.Tuple();
    /*std::cerr << "Variable: " << Type << " " << Name << std::endl;*/

    if (absl::EqualsIgnoreCase(Type, "int")) {
      if (auto val = TestAndSetValue<int>(Value); val.has_value()) {
        /*std::cerr << Name << ": " << val.value() << std::endl;*/
        lua[Name] = val.value();
      }

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

void Interpret::PrepareStates() {
  StateGroup<sol::protected_function> tg;
  for (const auto& state : stateGroup) {
    std::cout << state << std::endl;
    State<sol::protected_function> n{};
    n.Name = state.Name;
    if (const auto a = TestAndSet(state.Action);
        a.has_value() && a.value().valid()) {
      n.Action = a.value();
    } else {
      LOG(ERROR) << "Unexpected error while setting state action";
      throw Utils::ProgramTermination();
    }

    stateGroupFunction.Add(n);
  }
}

TransitionGroup Interpret::WhenConditionTrue(const TransitionGroup& group) {
  TransitionGroup on_true;
  for (const auto& [id, transition] : group.primary) {
    if (!transition.hasCondition)
      continue;

    if (auto r = transition.function(); r.valid() && ExtractBool(r)) {
      on_true.primary[id] = transition;
    } else if (r.valid()) {
    } else {
      if (const auto something = TestAndSet(transition.condition);
          something.has_value() && something.value().valid()) {
        auto result = something.value()();
      }
      const sol::error err = r;
      LOG(ERROR) << err.what();
      throw Utils::ProgramTermination();
    }
  }
  return on_true;
}

std::optional<sol::protected_function> Interpret::TestAndSet(
    const std::string& _cond) {
  if (_cond.empty()) {
    if (const auto zero = lua.load("return true"); zero.valid()) {
      return zero.get<sol::protected_function>();
    }
    return std::nullopt;
  }
  std::string chunk_to_load;
  if (!Utils::Contains(_cond, "return")) {
    chunk_to_load = "return " + _cond;
  } else {
    chunk_to_load = _cond;
  }

  /*std::cerr << chunk_to_load << std::endl;*/
  if (const auto primary = lua.load(chunk_to_load); primary.valid()) {
    return primary.get<sol::protected_function>();
  } else {
    const sol::error primary_error = primary;
    LOG(ERROR) << primary_error.what();
    if (const auto secondary = lua.load("return true"); secondary.valid()) {
      return secondary.get<sol::protected_function>();
    } else {
      const sol::error secondary_error = secondary;
      LOG(ERROR) << secondary_error.what();
      return std::nullopt;
    }
  }
}

bool Interpret::ExtractBool(const sol::protected_function_result& result) {
  /*std::cerr << "ExtractBool" << std::endl;*/
  if (!result.valid()) {
    const sol::error r_error = result;
    LOG(ERROR) << absl::StrFormat("Lua runtime error during function call: %v",
                                  r_error.what());
    return false;
  }

  const auto ret = result[0];
  const auto val_type = ret.get_type();

  if (val_type == sol::type::nil) {
    return false;
  }
  if (val_type == sol::type::boolean) {
    return ret.get<bool>();
  }
  //? Anything that isn't 'nil' or 'false' is automatically true
  return true;
}

void Interpret::PrepareTransitions() {
  for (auto& [id, transition] : transitionGroup) {
    if (auto r = TestAndSet(transition.condition);
        r.has_value() && r.value().valid()) {
      transition.function = r.value();
      transition.hasCondition = transition.function.valid();
    } else {
      LOG(ERROR) << absl::StrFormat(
          "Transition: %v -> %v; Error in lua runtime or missing correct "
          "definition",
          transition.from, transition.to);
      throw Utils::ProgramTermination();
    }
  }
}

void Interpret::PrepareSignals() {
  for (auto& signal : inputs) {
    /*std::cerr << "Signal: " << signal << std::endl;*/
    lua["Inputs"][signal] = "";
  }
  /*std::cout << std::endl;*/
  for (auto& signal : outputs) {
    lua["Outputs"][signal] = "";
  }
}

void Interpret::Prepare() {
  LinkDelays();
  PrepareVariables();
  PrepareTransitions();
  PrepareStates();
  PrepareSignals();
}

std::string Interpret::ExtractInput(const std::string& line) {
  const auto l = Utils::RemovePrefix<false>(line, "input:");
  // should be <name> = <value>
  const auto s = Utils::Split(l, '=');
  if (s.size() != 2) {
    LOG(ERROR) << absl::StrFormat("Possibly malformed input: %v", line);
    throw Utils::ProgramTermination();
  }
  auto name = Utils::Trim(s[0]);
  auto value = Utils::Trim(s[1]);

  if (const auto pos = std::find(inputs.begin(), inputs.end(), name);
      pos == inputs.end()) {
    LOG(ERROR) << "Cannot dynamically define new signals or required signal is missing";
    throw Utils::ProgramTermination();
  }
  lua["Inputs"][name] = value;
  return name;
}

bool Interpret::ExtractCommand(const std::string& line) {
  const auto l =
      Utils::RemovePrefix(line, "cmd:", Utils::StringComparison::Lazy);
  // Commands should be ideally single-worded
  const auto cmd = Utils::Trim(l);
  if (cmd.empty())
    return false;
  if (Utils::Contains(cmd, "stop")) {
    running = false;
  }
  return true;
}

std::pair<int, std::string> Interpret::ParseStdinInput(
    const std::string& line) {
  // INPUT, CMD, LOG
  std::cerr << "Line: " << line << std::endl;
  if (Utils::Contains(line, "input")) {
    std::string signalName = ExtractInput(line);
    return {1, ExtractInput(line)};
  }
  if (Utils::Contains(line, "stop")) {
    return {-1, ""};
  }
  if (Utils::Contains(line, "log")) {
    LOG(ERROR) << "Function 'log' is not implemented";
    throw Utils::ProgramTermination();
  }
  return {0, ""};
}

int Interpret::Execute() {
  transitionGroup.GroupTransitions();
  while (true) {
    std::cerr << "Active state: " << activeState << std::endl;

    // First find all reachable transitions from current transition
    auto transitions = transitionGroup.Retrieve(activeState);
    if (!transitions.has_value()) {
      LOG(ERROR) << "No transitions for state: " << activeState << std::endl;
      break;
    }

    auto& transitions_v = transitions.value();
    if (transitions_v.None()) {
      LOG(ERROR) << "No transitions for state: " << activeState << std::endl;
      break;
    }

    // Find all free transitions
    if (auto r = transitions_v.WhereNone(); r.Some()) {
      ChangeState(r);
      continue;
    }

    TransitionGroup event_true;
    TransitionGroup event_false;
    TransitionGroup timer_true;
    TransitionGroup timer_false;

    for (const auto& [fst, snd] : transitions_v) {
      if (!snd.input.empty())
        event_true << snd;
      else {
        event_false << snd;
        event_false << snd;
      }
      if (snd.delayInt == 0)
        timer_false << snd;
      else
        timer_true << snd;
    }

    if (auto zero = timer_false & event_false; zero.Some()) {
      ChangeState(zero);
      continue;
    }
    if (auto first = timer_true & event_false; first.Some()) {
      if (WaitShortestTimer(first))
        continue;
    }

    auto items = event_true.YieldInputNames();
    std::cout << "REQUEST_INPUTS: "  << absl::StrJoin(items, ", ") << std::endl;

    std::string line;
    std::getline(std::cin, line);
    auto [code, signalName] = ParseStdinInput(line);
    if (code == -1)
      break;
    if (code == 0)
      continue;

    if (auto second = event_true & timer_false; second.Some()) {
      auto inputs = second.Where([signalName](const Transition& tr) {
        return tr.input == signalName;
      });
      ChangeState(inputs);
      continue;
    }
    if (auto third = event_true & timer_true; third.Some()) {
      auto inputs = third.Where([signalName](const Transition& tr) {
        return tr.input == signalName;
      });
      WaitShortestTimer(inputs);
    }
  }
  return 0;
}
}  // namespace Interpreter