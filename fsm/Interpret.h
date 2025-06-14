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
  /// @brief Lua state for executing state actions and interpreting transition conditions
  ///
  /// @attention Important Sol2 lifetime management \n \n
  /// 'sol::state lua' MUST be declared BEFORE all members that store (and thus is destroyed after all members that store ...)
  /// 'sol::function' or 'sol::protected_function' objects (e.g., stateGroupFunction, transitionGroupFunction). \n \n
  /// RATIONALE: C++ destructs member variables in the REVERSE order of their declaration.
  /// 'lua' owns the underlying Lua state (lua_State*). If 'lua' is destroyed
  /// BEFORE any 'sol::function'/'sol::protected_function' objects that reference it,
  /// those objects will try to access a destroyed Lua state during their destruction,
  /// leading to a use-after-free crash (0xC0000005).
  sol::state lua{};

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

  /// Seznam registrovaných výstupních signálů
  std::vector<std::string> outputs = _automat.outputs;

  void ChangeState(const TransitionGroup& tg);
  bool WaitShortestTimer(const TransitionGroup& group);

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

  TransitionGroup WhenConditionTrue(const TransitionGroup& group);

 public:
  /// Skupina přechodů vybraná k aktuálnímu zpracování
  mutable TransitionGroup transitionGroup{};

  /**
   * @brief Provede kompletní přípravu interpretu voláním přípravných metod.
   */
  void Prepare();

  explicit Interpret(const AutomatLib::Automat& automat);

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
