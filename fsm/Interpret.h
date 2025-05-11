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


namespace Interpreter {
using namespace types;

/**
 * @class Interpret
 * @brief Spouští a interpretuje běh konečného automatu.
 */
class Interpret {
private:
  /// Generovaný automat, který se bude interpretovat
  AutomatLib::Automat _automat;

  /// Aktuální název stavu
  std::string activeState;

  /// Kolekce všech stavů automatu
  StateGroup stateGroup;

  /// Kolekce všech proměnných automatu
  VariableGroup variableGroup;

  /// Seznam registrovaných vstupních signálů
  std::vector<std::string> inputs;

  /// Seznam registrovaných výstupních signálů
  std::vector<std::string> outputs;
  void ChangeState(const TransitionGroup& tg);
  void LinkDelays();

  /**
   * @brief Připraví proměnné v rámci Lua prostředí automatu.
   */
  void PrepareVariables();

  /**
   * @brief Načte a připraví přechodová pravidla z modelu automatu.
   */
  void PrepareTransitions();

  /**
   * @brief Inicializuje vstupní a výstupní signály před spuštěním.
   */
  void PrepareSignals();

public:
  /// Skupina přechodů vybraná k aktuálnímu zpracování
  TransitionGroup transitionGroup;

  /**
   * @brief Provede kompletní přípravu interpretu voláním přípravných metod.
   */
  void Prepare();

  /**
   * @brief Konstruktor přijímající existující model automatu.
   * @param automat Reference na instanci třídy Automat.
   */
  explicit Interpret(AutomatLib::Automat &automat);

  /**
   * @brief Statická ukázková metoda demonstrující jednoduché použití interpretu.
   */
  static void simpleExample();

  /// Interní Lua stav pro vykonání akcí automatu
  sol::state lua{};

  /**
   * @brief Spustí vykonání automatu.
   * @param once Pokud true, provede pouze jeden krok a vrátí výsledek.
   * @return Výstupní kód nebo hodnota výsledku provedení.
   */
  int Execute(bool once);
};

}  // namespace Interpreter
