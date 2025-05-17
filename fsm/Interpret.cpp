#include "Interpret.h"

#include <re2/re2.h>
#include <spdlog/spdlog.h>
#include <thread>

#include "Utils.h"
#include "absl/log/absl_check.h"
#include "absl/strings/match.h"
#include "external/sol.hpp"

//TODO change the transition (and potentially states) to functions, compile them ahead via lua.load(...)
//TODO and store the as protected_functions in some friend container (TransitionGroup if remade with generics is good contender)
//TODO same for state actions
//TODO maybe try to split the Execute logic
//TODO try to implement ability to watch multiple events (input, timer)
//TODO remove the std::move for lua state, state will be build directly in interpreter
//TODO do a more robust error detection and recovery
//TODO ^ for all parts of project (automat, parser, interpreter)
//TODO create a lua file that would contain useful functions (in a way, behave like a stdlib)

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

Interpret::Interpret(AutomatLib::Automat&& automat)
    : _automat(std::move(automat)) {
  lua.open_libraries(sol::lib::base);
  lua["elapsed"] = [&]() { return timer.elapsed(); };
  if (auto file = lua.script_file("stdlib.lua"); !file.valid) {
    const sol::error err = file;
    spdlog::error("Failed to open stdlib.lua: {}", err.what());
    exit(1);
  }
}

void Interpret::InterpretResult(sol::object& result) {
  auto type = result.get_type();
  spdlog::debug("Detected type is {}", type);

  if (result.is<bool>) {
    bool value = result.as<bool>();
    spdlog::debug("Interpreted as bool: {}", value);
  } else if (result.is<int>) {
    int value = result.as<int>();
    spdlog::debug("Interpreted as int: {}", value);
  } else if (result.is<double>) {
    spdlog::debug("Interpreted as double: {}", result.as<double>());
  } else if (result.is<std::string>) {
    spdlog::debug("Interpreted as string: {}", result.as<std::string>());
  } else if (result.is<sol::table>) {
    spdlog::debug("Interpreted as table");
  } else if (result.is<sol::function>) {
    spdlog::debug("Interpreted as function");
  } else if (result.is<sol::type::nil>) {
    spdlog::debug("Interpreted as nil");
  } else {
    spdlog::debug("Interpretation undefined");
  }
}
//TODO modify this to accept reference instead of group
void Interpret::ChangeState(
    const TransitionGroup<sol::protected_function>& tg) {
  if (const auto t = tg.First(); t.has_value()) {
    timer.tock();
    activeState = t.value().get().to;
    std::cout << "STATE: " << activeState << std::endl;
    const auto [Name, Action] = stateGroupFunction.Find(activeState).First();
    if (const auto result = Action(); result.valid()) {
      // yay
      auto r = sol::object(result[0]);
      InterpretResult(r);
    } else {
      // aww
      spdlog::debug("Action undefined");
    }
  }
}

void Interpret::ChangeState(TransitionsReference<sol::protected_function>& tr) {
  const auto first = tr.First();
  timer.tock();
  activeState = first.get().to;
  spdlog::info("STATE: {}", activeState);
  const auto [Name, Action] = stateGroupFunction.Find(activeState).First();
  if (const auto result = Action(); result.valid()) {
    // yay
    auto r = sol::object(result[0]);
    InterpretResult(r);
  } else {
    // aww
    const sol::error err(result);
    spdlog::error("Action execution error: {}", err.what());
  }
}

void Interpret::LinkDelays() {
  //TODO finish this
  auto hasDelay = transitionGroup.WhereMut(
      [](const Transition<>& tr) { return !tr.delay.empty(); });

  if (hasDelay.None())
    return;

  for (auto v : variableGroup.Get()) {
    auto mod = hasDelay.WhereMut(
        [v](const Transition<>& tr) { return tr.delay == v.Name; });

    if (auto val = TestAndSetValue<int>(v.Value); val.has_value()) {
      mod.TransformMut([val](auto& tr) {
        using RawType = Utils::detail::remove_cvref_t<decltype(tr)>;
        if constexpr (std::is_pointer_v<RawType> || is_smart_ptr<RawType>) {
          tr->delayInt = val.value();
          return tr;
        }
        tr.delayInt = val.value();
        return tr;
      });
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

void Interpret::PrepareStates() { // NOLINT(*-convert-member-functions-to-static)
  StateGroup<sol::protected_function> tg;
  auto new_states = stateGroup.Transform<sol::protected_function>(
      [this](const State<std::string>& tr) {
        State<sol::protected_function> n{};
        n.Name = tr.Name;
        if (const auto a = TestAndSet(tr.Action); a.has_value())
          n.Action = a.value();
        else {
          n.Action = sol::protected_function{};
        }
        return n;
      });
  //TODO store the result somewhere
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
    spdlog::warn(primary_error.what());
    if (const auto secondary = lua.load("return true"); secondary.valid()) {
      return secondary.get<sol::protected_function>();
    } else {
      const sol::error secondary_error = secondary;
      spdlog::error(secondary_error.what());

      return std::nullopt;
    }
  }
}

/**
 *
 * @param result
 * @return
 */
auto Interpret::ExtractBool(const sol::protected_function_result& result) {
  if (!result.valid()) {
    sol::error r_error = result;
    spdlog::error("Lua runtime error during function call: {}", r_error);
    return false;
  }

  const auto ret = result[0];
  auto val_type = ret.get_type();
  spdlog::debug("Detected type: {}", val_type);

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
      auto ntr =
          Transition<decltype(res)>::Convert /*<Transition<>>*/ (transition);
      ntr.cond = res;
      ntr.hasCondition = true;
      ntg.Add(std::move(ntr));
    } else {
      //! in case of complete failure skip the transition? or stop completely?
      spdlog::warn(
          "Skipping transition: {} -> {}; Error in lua runtime or missing "
          "correct definition",
          transition.from, transition.to);
    }
  }
  transitionGroupFunction = ntg;
}

void Interpret::PrepareSignals() {
  for (auto& signal : inputs) {
    lua["Inputs"][signal] = "";
    // inputsValues[signal] = ""; // can do this without using external containers
  }
  std::cout << std::endl;
  for (auto& signal : outputs) {
    lua["Outputs"][signal] = "";
    // outputsValues[signal] = "";
  }
}

void Interpret::Prepare() {
  LinkDelays();
  PrepareVariables();
  PrepareTransitions();
  PrepareStates();
  PrepareSignals();
}

bool StringToBool(const std::string& str) {
  const auto lower = Utils::ToLower(str);
  std::istringstream stream(lower);
  bool val;
  stream >> std::boolalpha >> val;
  return val;
}

std::string Interpret::ExtractInput(const std::string& line) {
  auto l = Utils::RemovePrefix(line, "input:", Utils::StringComparison::Lazy);
  // should be <name> = <value>
  const auto s = Utils::Split(line, '=');
  if (s.size() != 2) {
    spdlog::error("Possibly malformed input: {}", line);
    return {};
  }
  auto name = Utils::Trim(s[0]);
  auto value = Utils::Trim(s[1]);
  if (!inputs.contains(name)) {
    spdlog::error("Cannot dynamically define new signals");
    return {};
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
  }
  return {0, ""};
}

int Interpret::Execute() {
  //TODO split into separate procedure for clarity
  //? Placing it here leads to better results
  lua.open_libraries(sol::lib::base);
  try {
    while (running) {
      // First find all reachable transitions from current transition
      std::cout << "Active state: " << activeState << std::endl;
      auto transitions = transitionGroupFunction.RetrieveMut(activeState);
      if (transitions.None())
        continue;  // exit is better
      // std::cout << "Possible next states: " << std::endl
      //           << transitions.Out() << std::endl;
      // Find all free transitions
      if (auto r = transitions.WhereNone(); r.Some()) {
        // move and continue
        ChangeState(r);
        continue;
      }
      auto transitionsCond = transitions.WhereCond();

      auto condTrue =
          TransitionsReference<decltype(transitionsCond)::PureType>{};
      for (auto& transition : transitionsCond) {
        if (auto cond = transition.get().cond();
            cond.valid() && ExtractBool(cond)) {
          condTrue.Add(transition.get());
        } else {
          spdlog::warn("Error retrieving value from function");
          continue;
        }
      }

      auto [event_true, event_false] = condTrue.WhereEvent<true>();
      auto [timer_true, timer_false] = condTrue.WhereTimer<true>();
      // Find all transitions that have condition, but no input
      if (auto r = event_false; r.Some()) {
        // set timer for smallest delay transition
        /*
      * can the timer influence where to go?
      * if timer starts and input is registered should I continue with timer or input?
      */
        /*
      * this is a trick as according to above timer is decisive and
      * at any time there would be more than one timer
      * and both are started at the same time, the timer with smaller delay will always end first.
      *
      * because of these two assumptions, I am allowed to do this
      */
        auto smallest = r.SmallestTimer().get();
        auto duration = std::chrono::milliseconds(smallest.delayInt);
        std::this_thread::sleep_for(duration);
        ChangeState(r);
        continue;
        /*if (auto smallest = r.Smallest(); smallest.has_value()) {
          auto duration = std::chrono::milliseconds(smallest.value().delayInt);
          std::this_thread::sleep_for(duration);
          ChangeState(r);
          continue;
        }*/
      }
      if (auto r = event_true; r.Some()) {
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
          spdlog::debug("TransitionGroup: {}", inputs.Out());
          ChangeState(inputs);
          continue;
        }
        if (auto r2 = event_true & timer_true; r2.Some()) {
          // All transitions with input and timer
          //TODO missing waiting
          auto inputs = r2.Where(
              [signalName](const Transition<sol::protected_function>& tr) {
                return tr.input == signalName;
              });
          spdlog::debug("TransitionGroup: {}", inputs.Out());
          ChangeState(inputs);
        }
      }
    }
  } catch (const std::exception& e) {
    spdlog::error(e.what());
    return 1;
  }
  return 0;
}

}  // namespace Interpreter