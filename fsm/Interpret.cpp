#include "Interpret.h"

#include <absl/log/absl_log.h>
#include <absl/strings/match.h>
#include <re2/re2.h>

#include <thread>

#include "Utils.h"
#include "external/sol.hpp"

#pragma region counter init
template <>
std::atomic<unsigned> types::Transition<>::counter = 0;

template <>
std::atomic<unsigned> types::Transition<sol::protected_function>::counter = 0;
#pragma endregion

namespace Interpreter {
void Interpret::simpleExample() {
  sol::state lua;
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

Interpret::Interpret(AutomatLib::Automat&& automat)
    : _automat(std::move(automat)) {
  lua.open_libraries(sol::lib::base);
  lua["elapsed"] = [&]() { return timer.elapsed(); };
  if (const auto file = lua.script_file("stdlib.lua"); !file.valid()) {
    const sol::error err = file;
    ABSL_LOG(ERROR) << absl::StrFormat("Failed to open stdlib.lua: %v",
                                       err.what());
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
  ABSL_LOG(ERROR) << "Unknown result type";
  throw Utils::ProgramTermination();
}

void Interpret::ChangeState(
    const TransitionGroup<sol::protected_function>& tg) {
  timer.tock();

  if (const auto t = tg.First(); t.has_value()) {
    activeState = t.value().to;
  } else {
    ABSL_LOG(FATAL) << "No next state found, but expected one";
    throw Utils::ProgramTermination();
  }
  std::cout << "STATE: " << activeState << std::endl;
  const auto [Name, Action] = stateGroupFunction.Find(activeState).First();
  if (const auto result = Action(); result.valid()) {
    auto r = sol::object(result[0]);
    auto i_result = InterpretResult(r);
    //TODO what to do with the i_result?
  } else {
    const sol::error err = result;
    ABSL_LOG(ERROR) << absl::StrFormat("Action execution error: %v",
                                       err.what());
    throw Utils::ProgramTermination();
  }
}

void Interpret::LinkDelays() {
  const auto hasDelay = transitionGroup.Where(
      [](const Transition<>& tr) { return !tr.delay.empty(); });

  if (hasDelay.None()) {
    ABSL_LOG(INFO) << "No delay found";
    return;
  }

  for (auto& v : variableGroup) {
    auto mod = hasDelay.Where(
        [v](const Transition<>& tr) { return tr.delay == v.Name; });

    if (mod.None())
      continue;

    if (auto val = TestAndSetValue<int>(v.Value); val.has_value()) {
      for (auto item : mod) {
        item.delayInt = val.value();
      }
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

void Interpret::
    PrepareStates() {  // NOLINT(*-convert-member-functions-to-static)
  StateGroup<sol::protected_function> tg;
  const auto new_states =
      stateGroup.Transform<sol::protected_function>([this](const State<>& tr) {
        State<sol::protected_function> n{};
        n.Name = tr.Name;
        if (const auto a = TestAndSet(tr.Action); a.has_value())
          n.Action = a.value();
        else {
          n.Action = sol::protected_function{};
        }
        return n;
      });
  stateGroupFunction = new_states;
}

std::optional<sol::protected_function> Interpret::TestAndSet(
    const std::string& _cond) {
  if (_cond.empty()) {
    if (const auto zero = lua.load("return true"); zero.valid()) {
      return zero.get<sol::protected_function>();
    }
    return std::nullopt;
  }
  if (const auto primary = lua.load(_cond); primary.valid()) {
    return primary.get<sol::protected_function>();
  } else {
    const sol::error primary_error = primary;
    ABSL_LOG(WARNING) << primary_error.what();  //! this might not work
    if (const auto secondary = lua.load("return true"); secondary.valid()) {
      return secondary.get<sol::protected_function>();
    } else {
      const sol::error secondary_error = secondary;
      ABSL_LOG(ERROR) << secondary_error.what();
      return std::nullopt;
    }
  }
}

bool Interpret::ExtractBool(const sol::protected_function_result& result) {
  if (!result.valid()) {
    const sol::error r_error = result;
    ABSL_LOG(ERROR) << absl::StrFormat(
        "Lua runtime error during function call: %v", r_error.what());
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
  TransitionGroup<sol::protected_function> ntg;
  for (auto& transition : transitionGroup) {
    if (transition.cond.empty())
      continue;

    if (auto r = TestAndSet(transition.cond);
        r.has_value() && r.value().valid()) {
      auto res = r.value();
      auto ntr = Transition<decltype(res)>::Convert(transition);
      ntr.cond = res;
      ntr.hasCondition = true;
      ntg.Add(ntr);
    } else {
      ABSL_LOG(ERROR) << absl::StrFormat(
          "Transition: %v -> %v; Error in lua runtime or missing correct "
          "definition",
          transition.from, transition.to);
      throw Utils::ProgramTermination();
    }
  }
  transitionGroupFunction = ntg;
}

void Interpret::PrepareSignals() {
  for (auto& signal : inputs) {
    lua["Inputs"][signal] = "";
  }
  std::cout << std::endl;
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
  auto l = Utils::RemovePrefix(line, "input:", Utils::StringComparison::Lazy);
  // should be <name> = <value>
  const auto s = Utils::Split(line, '=');
  if (s.size() != 2) {
    ABSL_LOG(ERROR) << absl::StrFormat("Possibly malformed input: %v", line);
    return {};  //TODO terminate?
  }
  auto name = Utils::Trim(s[0]);
  auto value = Utils::Trim(s[1]);
  if (!inputs.contains(name)) {
    ABSL_LOG(ERROR) << "Cannot dynamically define new signals";
    return {};  //TODO terminate or not?
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
  if (Utils::Contains(line, "input")) {
    std::string signalName = ExtractInput(line);
    return {1, ExtractInput(line)};
  }
  if (Utils::Contains(line, "stop")) {
    return {-1, ""};
  }
  if (Utils::Contains(line, "log")) {
    //! Currently not implemented
    //TODO should i do it?
  }
  return {0, ""};
}

int Interpret::Execute() {
  //TODO split into separate procedure for clarity
  lua.open_libraries(sol::lib::base);
  while (running) {
    // First find all reachable transitions from current transition
    std::cout << "Active state: " << activeState << std::endl;
    auto transitions = transitionGroupFunction.Retrieve(activeState);
    if (!transitions.has_value())  //TODO add error
      break;
    const auto& transitions_v = transitions.value();
    if (transitions_v.None())
      break;

    // Find all free transitions
    if (auto r = transitions_v.WhereNone(); r.Some()) {
      ChangeState(r);
      continue;
    }

    auto transitionsCond = WhenConditionTrue(transitions_v);

    auto [event_true, event_false] = transitionsCond.WhereEvent<true>();
    auto [timer_true, timer_false] = transitionsCond.WhereTimer<true>();
    // Find all transitions that have condition, but no input
    if (auto r = event_false; r.Some()) {
      // set timer for smallest delay transition
      if (auto smallest = r.SmallestTimer(); smallest.has_value()) {
        WaitShortestTimer(r);
        continue;
      }
    }
    if (auto& r = event_true; r.Some()) {
      std::string line;
      std::getline(std::cin, line);
      auto [code, signalName] = ParseStdinInput(line);
      if (code == -1)
        break;
      if (code == 0)
        continue;

      if (auto r1 = event_true & timer_false; r.Some()) {
        // All transitions with input but no timer
        auto inputs = r1.Where(
            [signalName](const Transition<sol::protected_function>& tr) {
              return tr.input == signalName;
            });

        ChangeState(inputs);
        continue;
      }
      if (auto r2 = event_true & timer_true; r2.Some()) {
        // All transitions with input and timer
        auto inputs = r2.Where(
            [signalName](const Transition<sol::protected_function>& tr) {
              return tr.input == signalName;
            });
        WaitShortestTimer(inputs);
      }
    }
  }
  return 0;
}
}  // namespace Interpreter