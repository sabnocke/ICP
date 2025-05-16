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
#include "types/all_types.h"
#include "Stopwatch.h"


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
  StateGroup<std::string> stateGroup = _automat.states;
  StateGroup<sol::protected_function> stateGroupFunction;

  /// Aktuální název stavu
  std::string activeState = stateGroup.First().Name;

  /// Kolekce všech proměnných automatu
  VariableGroup variableGroup = _automat.variables;

  /// Seznam registrovaných vstupních signálů
  absl::btree_set<std::string> inputs = absl::btree_set<std::string>(_automat.inputs.begin(), _automat.inputs.end());
  absl::node_hash_map<std::string, std::string> inputsValues;   //! might be unnecessary

  /// Seznam registrovaných výstupních signálů
  std::vector<std::string> outputs = _automat.outputs;
  absl::node_hash_map<std::string, std::string> outputsValues;  //! might be unnecessary

  void ChangeState(const TransitionGroup<sol::protected_function>& tg);
  void LinkDelays();

  static void InterpretResult(sol::object& result);

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

  std::optional<sol::protected_function> Interpret::TestAndSet(
      const std::string& _cond);
  static bool ExtractBool(const sol::protected_function_result& result);

  Timer<> timer{};

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
   * @param automat Reference na instanci třídy Automat.
   */
  explicit Interpret(AutomatLib::Automat&& automat);

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
