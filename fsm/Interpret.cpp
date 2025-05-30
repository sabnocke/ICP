#include "Interpret.h"

#include <absl/log/log.h>
#include <absl/strings/match.h>
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
  /*std::cerr << "Entered interpret ctor" << std::endl;*/
  lua.open_libraries(sol::lib::base);
  lua["elapsed"] = [&]() { return timer.elapsed(); };
  if (const auto file = lua.script_file("stdlib.lua"); !file.valid()) {
    const sol::error err = file;
    LOG(ERROR) << absl::StrFormat("Failed to open stdlib.lua: %v", err.what());
    throw Utils::ProgramTermination();
  }
  activeState = stateGroup.First().Name;
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
    LOG(FATAL) << "No next state found, but expected one";
    throw Utils::ProgramTermination();
  }
  std::cout << "STATE: " << activeState << std::endl;
  const auto [Name, Action] = stateGroupFunction.Find(activeState).First();
  if (const auto result = Action(); result.valid()) {
    const auto r = sol::object(result[0]);
    /*std::cerr << "Result index: " << i_result.index() << std::endl;*/
    if (const auto i_result = InterpretResult(r); i_result.index() == 4) {
      std::cout << "OUTPUT: " << std::get<4>(i_result) << std::endl;
    } else if (i_result.index() == 1) {}
    else {
      LOG(ERROR) << "Invalid (or unexpected) result type index: " << i_result.index() << std::endl;
      //TODO what to do?
    }
  } else {
    const sol::error err = result;
    LOG(ERROR) << absl::StrFormat("Action execution error: %v", err.what());
    throw Utils::ProgramTermination();
  }
}

void Interpret::LinkDelays() {
  const auto hasDelay = transitionGroup.Where(
      [](const Transition& tr) { return !tr.delay.empty(); });

  if (hasDelay.None()) {
    LOG(INFO) << "No delay found";
    return;
  }
  /*std::cerr << hasDelay << std::endl;
  std::cerr << variableGroup << std::endl;*/
  for (auto& v : variableGroup) {
    /*std::cerr << v << std::endl;*/
    auto mod = hasDelay.Where(
        [v](const Transition& tr) { return tr.delay == v.Name; });
    /*std::cerr << "mod: " << mod << std::endl;*/
    if (mod.None())
      continue;

    if (auto val = TestAndSetValue<int>(v.Value); val.has_value()) {
      /*std::cerr << "value: " << val.value() << std::endl;*/
      for (auto& [_, transition] : mod) {
        transition.delayInt = val.value();
      }
    }
    /*std::cerr << "mod: " << mod << std::endl;*/
    transitionGroup = transitionGroup.Merge(mod);
    /*std::cerr << "transitionGroup after merge: " << transitionGroup << std::endl;*/
  }
}

void Interpret::PrepareVariables() {
  //TODO doest this work?
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
      //! this might be an error
      //TODO add error and sol error
      n.Action = sol::protected_function{};
    }

    stateGroupFunction.Add(n);
  }
}

TransitionGroup Interpret::WhenConditionTrue(const TransitionGroup& group) {
  std::cerr << "Entered WhenConditionTrue" << std::endl;
  std::cerr << group << std::endl;
  TransitionGroup on_true;
  for (const auto& [id, transition] : group.primary) {
    if (!transition.hasCondition)
      continue;

    if (auto r = transition.function(); r.valid() && ExtractBool(r)) {
      on_true.primary[id] = transition;
    } else if (r.valid()) {
    } else {
      const sol::error err = r;
      LOG(ERROR) << err.what();
      throw Utils::ProgramTermination();
    }
  }
  return on_true;
}

std::optional<sol::protected_function> Interpret::TestAndSet(
    const std::string& _cond) {
  // auto q = Utils::Quote(_cond);
  if (_cond.empty()) {
    if (const auto zero = lua.load("return true"); zero.valid()) {
      return zero.get<sol::protected_function>();
    }
    return std::nullopt;
  }
  const auto chunk_to_load = "return " + _cond;
  if (const auto primary = lua.load(chunk_to_load); primary.valid()) {
    return primary.get<sol::protected_function>();
  } else {
    const sol::error primary_error = primary;
    /*LOG(WARNING) << primary_error.what();  //! this might not work*/
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
  std::cerr << "ExtractBool" << std::endl;
  if (!result.valid()) {
    const sol::error r_error = result;
    LOG(ERROR) << absl::StrFormat("Lua runtime error during function call: %v",
                                  r_error.what());
    return false;
  }

  const auto ret = result[0];
  const auto val_type = ret.get_type();
  auto val = InterpretResult(result);
  auto ind = val.index();
  std::cerr << "variant index: " << ind << std::endl;
  if (ind == 1)
    std::cerr << "Received: " << std::boolalpha << std::get<1>(val)
              << std::endl;
  if (ind == 2)
    std::cerr << "Received: " << std::get<2>(val) << std::endl;
  if (ind == 3)
    std::cerr << "Received: " << std::get<3>(val) << std::endl;
  if (ind == 4)
    std::cerr << "Received: " << std::get<4>(val) << std::endl;

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
  // TransitionGroup ntg;
  for (auto& [id, transition] : transitionGroup) {
    /*if (transition.condition.empty())
      continue;*/

    if (auto r = TestAndSet(transition.condition);
        r.has_value() && r.value().valid()) {
      std::cerr << r.value()().get<bool>() << std::endl;
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
    std::cerr << "Signal: " << signal << std::endl;
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
  auto l = Utils::RemovePrefix<false>(line, "input:");
  /*std::cerr << "Removed prefix: " << l << std::endl;*/
  // should be <name> = <value>
  const auto s = Utils::Split(l, '=');
  if (s.size() != 2) {
    LOG(ERROR) << absl::StrFormat("Possibly malformed input: %v", line);
    return {};  //TODO terminate?
  }
  auto name = Utils::Trim(s[0]);
  /*std::cerr << "name: " << name << std::endl;*/
  auto value = Utils::Trim(s[1]);
  /*std::cerr << "Value: " << value << std::endl;*/
  /*for (auto& item : inputs) {
    std::cerr << "Item: " << item << std::endl;
  }*/
  if (const auto pos = std::find(inputs.begin(), inputs.end(), name);
      pos == inputs.end()) {
    LOG(ERROR) << "Cannot dynamically define new signals";
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
  std::cerr << "Line: " << line << std::endl;
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
  std::cerr << "Entered execute" << std::endl;
  std::cerr << transitionGroup << std::endl;
  transitionGroup.GroupTransitions();
  std::cerr << transitionGroup << std::endl;
  while (true) {
    // First find all reachable transitions from current transition
    std::cout << "Active state: " << activeState << std::endl;
    auto transitions = transitionGroup.Retrieve(activeState);
    if (!transitions.has_value()) {
      LOG(ERROR) << "No transitions for state: " << activeState << std::endl;
      break;
    }
    auto& transitions_v = transitions.value();
    if (transitions_v.None())
      break;

    std::cerr << "Next states: " << std::endl << transitions_v << std::endl;
    /*auto f = transitions_v.First().value();
    auto F = transitions_v.primary.begin();*/
    // Find all free transitions
    if (auto r = transitions_v.WhereNone(); r.Some()) {
      ChangeState(r);
      continue;
    }

    /*auto transitionsCond = WhenConditionTrue(transitions_v);*/
    /*auto [condition_present, condition_missing] = transitions_v.WhereCondition<true>();*/
    /*std::cerr << "Condition present: " << condition_present << std::endl;
    std::cerr << "Condition missing: " << condition_missing << std::endl;*/
    auto [event_true, event_false] = transitions_v.WhereEvent<true>();
    auto [timer_true, timer_false] = transitions_v.WhereTimer<true>();
    std::cerr << event_true << std::endl;
    if (auto zero = event_false & timer_false; zero.Some()) {
      ChangeState(zero);
      continue;
    }
    if (auto first = event_false & timer_true; first.Some()) {
      if (WaitShortestTimer(first))
        continue;
    }

    std::string line;
    std::getline(std::cin, line);
    auto [code, signalName] = ParseStdinInput(line);
    if (code == -1)
      break;
    if (code == 0)
      continue;

    std::cerr << "Line parse result: " << code << " " << signalName << std::endl;

    if (auto second = event_true & timer_false; second.Some()) {
      std::cerr << "second: " << second << std::endl;
      auto inputs = second.Where([signalName](const Transition& tr) {
        return tr.input == signalName;
      });
      std::cerr << "Result: " << inputs << std::endl;
      ChangeState(inputs);
      continue;
    }
    if (auto third = event_true & timer_true; third.Some()) {
      auto inputs = third.Where([signalName](const Transition& tr) {
        return tr.input == signalName;
      });
      std::cerr << "Result: " << inputs << std::endl;
      WaitShortestTimer(inputs);
    }

    /*std::cerr << "zero: " << zero << std::endl;
    std::cerr << "first: " << first << std::endl;*/
    /*std::cerr << "second: " << second << std::endl;
    std::cerr << "third: " << third << std::endl;*/

    /*auto all = WhenConditionTrue(transitionGroup);
    std::cerr << all << std::endl;*/

    /*break;
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
        auto inputs = r1.Where([signalName](const Transition& tr) {
          return tr.input == signalName;
        });

        ChangeState(inputs);
        continue;
      }
      if (auto r2 = event_true & timer_true; r2.Some()) {
        // All transitions with input and timer
        auto inputs = r2.Where([signalName](const Transition& tr) {
          return tr.input == signalName;
        });
        WaitShortestTimer(inputs);
      }
    }*/
  }
  return 0;
}
}  // namespace Interpreter