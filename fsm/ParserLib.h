/**
 * @file   ParserLib.h
 * @brief  Deklaruje parser pro načítání popisu automatu ze souboru.
 * @author xhlochm00 Michal Hloch
 * @details
 * Obsahuje struktury pro záznam proměnných a přechodů (TransitionRecord, VariableRecord),
 * výčet sekcí, a třídu Parser pro načtení a zpracování textového formátu automatu.
 * @date   2025-05-09
 */

#pragma once

#include <re2/re2.h>
#include <string>

#include "AutomatLib.h"
#include "types/all_types.h"

namespace ParserLib {
  using namespace types;

  /**
   * @enum Section
   * @brief Výčet kapitol ve vstupním souboru automatu.
   */
  enum Section {
    Name,
    Comment,
    Variables,
    States,
    Transitions,
    Inputs,
    Outputs
  };

  /**
   * @class Parser
   * @brief Načítá řádky z textového souboru a převádí je na struktury automatu.
   */
  class Parser {
  public:
    /** @brief Vytvoří parser a sestaví regexové vzory. */
    Parser();

    /**
     * @brief Načte celý soubor a vrátí vytvořený objekt Automat.
     * @param file Cesta k souboru.
     * @return Instance Automat.
     */
    AutomatLib::Automat parseAutomat(const std::string &file);

    /**
     * @brief Zkusí rozparsovat stav ze zadaného řádku.
     * @param line Text řádku.
     * @return pair{název, kód akce} nebo nullopt.
     */
    [[nodiscard]] State<> parseState(const std::string &line) const;

    /**
     * @brief Zkusí rozparsovat proměnnou.
     * @param line Text řádku.
     */
    [[nodiscard]] Variable parseVariable(const std::string &line) const;

    /**
     * @brief Zkusí rozparsovat přechod.
     * @param line Text řádku.
     */
    [[nodiscard]] Transition<>
      parseTransition(const std::string &line) const;

    /**
     * @brief Extrahuje jméno (Name nebo jakýkoliv text).
     */
    [[nodiscard]] std::string extractName(const std::string &line) const;

    /**
     * @brief Zparsuje signál (vstup nebo výstup).
     */
    [[nodiscard]] std::vector<std::string> parseSignal(
        const std::string &line) const;

  private:
    /**
     * @brief Interní handler na aktuální sekci, volá konkrétní parse*.
     */
    bool SectionHandler(const std::string &line,
                        AutomatLib::Automat &automat) const;

    Section ActualSection = Name;               /**< Aktuální zpracovávaná sekce */
    size_t lineNumber = 0;

    std::unique_ptr<RE2> name_pattern_{};       /**< Regex pro jméno */
    std::unique_ptr<RE2> comment_pattern_{};    /**< Regex pro komentář */
    std::unique_ptr<RE2> variables_pattern_{};  /**< Regex pro proměnné */
    std::unique_ptr<RE2> states_pattern_{};     /**< Regex pro stavy */
    std::unique_ptr<RE2> transitions_pattern_{};/**< Regex pro přechody */
    std::unique_ptr<RE2> input_pattern_{};      /**< Regex pro vstupy */
    std::unique_ptr<RE2> output_pattern_{};     /**< Regex pro výstupy */
  };

}  // namespace ParserLib
