/**
 * @file   numeric.h
 * @brief  Definuje univerzální typ Numeric pro uchovávání různých číselných hodnot.
 * @author xzelni06 Robert Zelníček
 * @details
 * Používá std::variant pro uložení std::monostate, int, long long, double nebo std::string.
 * Umožňuje explicitní konstrukci, přiřazení, bezpečné získání uložené hodnoty,
 * porovnání a výpis s pevnou přesností u double.
 * @date   2025-05-11
 */

#pragma once

#include <fmt/base.h>

#include <iomanip>
#include <optional>
#include <ostream>
#include <string>
#include <tuple>
#include <type_traits>
#include <variant>

#include "../Utils.h"
#include "Utils.h"

namespace types {

/**
 * @brief Metafunkce kontrolující, zda je typ T jednou z alternativ ve std::variant.
 */
template <typename T, typename Variant>
struct is_in_variant;

template <typename T, typename... Alternatives>
struct is_in_variant<T, std::variant<Alternatives...>>
    : std::disjunction<std::is_same<T, Alternatives>...> {};


/**
 * @brief Konstantní výraz, zda je T jeden z alternativ ve variantě.
 */
template <typename T, typename Variant>
constexpr bool is_in_variant_v = is_in_variant<T, Variant>::value;

/**
 * @class Numeric
 * @brief Typ pro uchování a manipulaci s numerickými hodnotami a stringem.
 * @details
 * Interně používá std::variant<std::monostate, int, long long, double, std::string>.
 */
[[deprecated]]
struct Numeric {
private:
  std::variant<std::monostate, int, long long, double, std::string> _numeric;
  using VariantType = decltype(_numeric);

public:
  /**
   * @brief Výchozí konstruktor, inicializuje prázdnou hodnotu.
   */
  Numeric() = default;

  /**
   * @brief Vytvoří prázdnou (monostate) hodnotu.
   * @return Numeric obsahující monostate.
   */
  static Numeric Empty() { return Numeric{std::monostate{}}; }

  /**
   * @brief Explicitní konstruktor pro podporované typy.
   * @tparam T Jeden z typů ve variantě.
   * @param value Vkládaná hodnota.
   */
  template <typename T, typename = std::enable_if_t<
                            is_in_variant_v<Utils::detail::remove_cvref_t<T>, VariantType>>>
  explicit Numeric(T&& value) : _numeric(std::forward<T>(value)) {}

  /**
   * @brief Operátor přiřazení pro podporované typy.
   */
  template <typename T, typename = std::enable_if_t<
                            is_in_variant_v<remove_cvref_t<T>, VariantType>>>
  Numeric& operator=(T&& value) {
    _numeric = std::forward<T>(value);
    Utils::detail::remove_cvref_t
    return *this;
  }

  /**
   * @brief Bezpečně získá hodnotu z varianty, pokud typ odpovídá.
   * @tparam T Žádaný typ.
   * @return std::optional<T> s hodnotou nebo nullopt.
   */
  template <typename T>
  std::optional<remove_cvref_t<T>> Get() const {
    if constexpr (is_in_variant_v<remove_cvref_t<T>, VariantType>) {
      if (auto *ptr = std::get_if<remove_cvref_t<T>>(&_numeric)) {
        return *ptr;
      }
    }
    return std::nullopt;
  }

  //=== Operátory porovnání ===//
  bool operator==(const Numeric& other) const { return _numeric == other._numeric; }
  bool operator!=(const Numeric& other) const { return _numeric != other._numeric; }

  friend std::ostream& operator<<(std::ostream& os, const Numeric& num) {
    std::visit(
      [&](auto const& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, std::monostate>) {
          os << "none";
        } else if constexpr (std::is_integral_v<T>) {
          os << arg;
        } else if constexpr (std::is_same_v<T, double>) {
          os << std::fixed << std::setprecision(5) << arg << std::defaultfloat;
        } else {
          os << arg;
        }
      },
      num._numeric);
    return os;
  }
};

}  // namespace types
