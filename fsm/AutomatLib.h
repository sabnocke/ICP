/**
 * @file   AutomatLib.h
 * @brief  Deklaruje třídu Automat pro reprezentaci a vykonání konečného automatu.
 * @author xhlochm00 Michal Hloch
 * @author xzelni06 Robert Zelníček
 * @details
 * Tato třída uchovává stavy, přechody, vstupy, výstupy a proměnné automatu.
 * Poskytuje metody pro připravení a spuštění generovaných funkcí, napojení zpoždění,
 * vytvoření a propojení potřebných Lua helper funkcí a kontejnerů pro generovaný kód.
 * @date   2025-05-11
 */

#pragma once

#include <string>

#include "external/sol.hpp"
#include "types/all_types.h"

namespace AutomatLib {
using namespace types;

/**
   * @class Automat
   * @brief Reprezentuje konečný automat s jeho daty a generovanou implementací.
   */
class Automat {
 public:
  /**
     * @brief Přidá nový stav podle jména a akce.
     * @param result dvojice (název stavu, text akce)
     */
  void addState(const State<> &result) { states << result; }

  /**
     * @brief Přidá nový přechod.
     * @param result Záznam Transition.
     */
  void addTransition(Transition&& result) { transitions.Add(std::move(result)); }

  /**
     * @brief Přidá novou proměnnou.
     * @param result Záznam Variable.
     */
  void addVariable(const Variable &result) { variables << result; }

  /**
     * @brief Registruje vstupní signál.
     * @param name Název vstupu.
     */
  void addInput(const std::string &name) { inputs.emplace_back(name); }

  /**
     * @brief Registruje výstupní signál.
     * @param name Název výstupu.
     */
  void addOutput(const std::string &name) { outputs.emplace_back(name); }

  /// Název automatu
  std::string Name;

  /// Kontejner stavů
  StateGroup<> states;

  /// Kontejner přechodů
  TransitionGroup transitions;

  /// Seznam vstupů
  std::vector<std::string> inputs;

  /// Seznam výstupů
  std::vector<std::string> outputs;

  /// Kontejner proměnných
  VariableGroup variables;

  /// Název aktuálního stavu v době běhu
  std::string currentState;

};

}  // namespace AutomatLib
