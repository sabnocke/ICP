/**
 * @file   Interpret.h
 * @brief  Deklaruje třídu Interpret pro provádění a interpretaci konečného automatu.
 * @author xhlochm00 Michal Hloch
 * @author xzelni06 Robert Zelníček
 * @details
 * Třída Interpret uchovává instanci generovaného automatu, jeho stavy, přechody,
 * proměnné, vstupy a výstupy. Pomocí časovače z modulu Timing a Lua stavu řízení
 * běhu automatu zpracovává signály a přepíná mezi stavy.
 * @date   2025-05-10
 */
#pragma once

#include <absl/log/log.h>

#include <thread>

#include "AutomatLib.h"
#include "Stopwatch.h"
#include "types/all_types.h"

namespace Interpreter {
using namespace types;

/**
 * @class Interpret
 * @brief Spouští a interpretuje běh konečného automatu.
 */
class Interpret {
  /// Generovaný automat, který se bude interpretovat
  AutomatLib::Automat _automat;
  bool running = true;

  /// Kolekce všech stavů automatu
  StateGroup<> stateGroup{};
  StateGroup<sol::protected_function> stateGroupFunction{};

  /// Aktuální název stavu
  std::string activeState = stateGroup.First().Name;

  /// Kolekce všech proměnných automatu
  VariableGroup variableGroup = _automat.variables;

  /// Seznam registrovaných vstupních signálů
  std::vector<std::string> inputs = _automat.inputs;
  [[deprecated]] absl::node_hash_map<std::string, std::string> inputsValues;

  /// Seznam registrovaných výstupních signálů
  std::vector<std::string> outputs = _automat.outputs;
  [[deprecated]] absl::node_hash_map<std::string, std::string> outputsValues;

  void ChangeState(const TransitionGroup<sol::protected_function>& tg);
  /*void ChangeState(TransitionsReference<sol::protected_function>& tr);*/

  void LinkDelays();

  using InterpretedValue =
      std::variant<std::monostate, bool, int, double, std::string>;

  static InterpretedValue InterpretResult(const sol::object& result);

  /**
   * @brief Připraví proměnné v rámci Lua prostředí automatu.
   */
  void PrepareVariables();
  void PrepareStates();

  /**
   * @brief Načte a připraví přechodová pravidla z modelu automatu.
   */
  void PrepareTransitions();

  /**
   * @brief Inicializuje vstupní a výstupní signály před spuštěním.
   */
  void PrepareSignals();

  std::optional<sol::protected_function> TestAndSet(const std::string& _cond);

  static bool ExtractBool(const sol::protected_function_result& result);

  Timer<> timer{};

  template <typename T>
  void WaitShortestTimer(const TransitionGroup<T>& group) {
    if (auto shortest = group.SmallestTimer(); shortest.has_value()) {
      const auto duration =
          std::chrono::milliseconds(shortest.value().delayInt);
      std::this_thread::sleep_for(duration);

      ChangeState(group);
    }
  }

  static TransitionGroup<sol::protected_function> WhenConditionTrue(
      const TransitionGroup<sol::protected_function>& group);

 public:
  /// Skupina přechodů vybraná k aktuálnímu zpracování
  TransitionGroup<std::string> transitionGroup{};
  TransitionGroup<sol::protected_function> transitionGroupFunction{};

  /**
   * @brief Provede kompletní přípravu interpretu voláním přípravných metod.
   */
  void Prepare();

  /**
   * @brief Konstruktor přijímající existující model automatu.
   * @param automat R-hodnota instance třídy Automat.
   */
  explicit Interpret(AutomatLib::Automat&& automat);
  explicit Interpret(const AutomatLib::Automat& automat) {
    transitionGroup = automat.transitions;
    transitionGroupFunction = TransitionGroup<sol::protected_function>();
    stateGroup = automat.states;
    stateGroupFunction = StateGroup<sol::protected_function>();
    inputs = automat.inputs;
    outputs = automat.outputs;

    lua.open_libraries(sol::lib::base);
    lua["elapsed"] = [&]() { return timer.elapsed<>(); };
    if (const auto file = lua.script_file("stdlib.lua"); !file.valid()) {
      const sol::error err = file;
      LOG(ERROR) << absl::StrFormat("Failed to open stdlib.lua: %v",
                                    err.what());
      throw Utils::ProgramTermination();
    }
  }

  /**
   * @brief Statická ukázková metoda demonstrující jednoduché použití interpretu.
   */
  static void simpleExample();

  /// Interní Lua stav pro vykonání akcí automatu
  sol::state lua{};

  std::string ExtractInput(const std::string& line);
  bool ExtractCommand(const std::string& line);
  std::pair<int, std::string> ParseStdinInput(const std::string& line);
  /**
   * @brief Spustí vykonání automatu.
   * @return Výstupní kód nebo hodnota výsledku provedení.
   */
  int Execute();
};

}  // namespace Interpreter
