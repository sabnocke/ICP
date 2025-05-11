/**
 * @file   Stopwatch.h
 * @brief  Šablonová třída Timer pro snadné měření uplynulého času.
 * @author xhlochm00 Michal Hloch
 * @details
 * Používá šablonové parametry DT (typ intervalu, default std::chrono::milliseconds)
 * a ClockT (typ hodin, default std::chrono::steady_clock). Metody tick() a tock()
 * slouží k označení začátku a konce měření, duration() vrací rozdíl.
 * @date   2025-05-09
 */
#pragma once

#include <chrono>

/**
 * @class Timer
 * @brief Jednoduchý stopwatch pro měření doby běhu kódu.
 * @tparam DT    Typ výsledného časového intervalu (např. std::chrono::milliseconds).
 * @tparam ClockT Typ hodin (např. std::chrono::steady_clock).
 */
template <class DT = std::chrono::milliseconds,
          class ClockT = std::chrono::steady_clock>
class Timer {
public:
  /// Alias pro typ časového bodu
  using time_p_t = typename ClockT::time_point;

private:
  time_p_t _start = ClockT::now(); /**< Čas začátku měření. */
  time_p_t _end{};                 /**< Čas ukončení měření. */

public:
  /**
   * @brief Nastaví nový startovací bod a vynuluje předchozí end.
   */
  void tick() {
    _end = time_p_t{};
    _start = ClockT::now();
  }

  /**
   * @brief Označí konec měření, uloží aktuální čas do _end.
   */
  void tock() {
    _end = ClockT::now();
  }

  /**
   * @brief Vrátí uplynulý čas mezi tick() a tock().
   * @tparam T Volitelný re-typ pracovního intervalu (jiný než DT).
   * @return Časový interval typu T.
   */
  template <class T = DT>
  auto duration() const {
    return std::chrono::duration_cast<T>(_end - _start);
  }
};
