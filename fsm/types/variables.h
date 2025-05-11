/**
 * @file   variables.h
 * @brief  Definuje typy Variable a VariableGroup pro správu proměnných automatu.
 * @author xhlochm00 Michal Hloch
 * @author xzelni06 Robert Zelníček
 * @details
 * Struktura Variable uchovává informace o typu, názvu a hodnotě proměnné.
 * VariableGroup poskytuje kolekci těchto proměnných a metody pro vkládání a vyhledávání.
 * @date   2025-05-09
 */

#pragma once

#include <absl/container/btree_set.h>
#include <absl/strings/str_format.h>
#include <optional>
#include <string>
#include <variant>

#include "../Utils.h"
#include "numeric.h"

namespace types {

using TypeType = std::string;   /**< Alias pro typ proměnné. */
using NameType = std::string;   /**< Alias pro název proměnné. */
using ValueType = std::string;  /**< Alias pro hodnotu proměnné jako string. */

/**
 * @brief Trait pro detekci operátoru < mezi typy.
 */
template <typename T, typename = void>
struct has_less : std::false_type {};

template <typename T>
struct has_less<
    T,
    std::void_t<decltype(std::declval<T &>() < std::declval<T &>())>>
    : std::true_type {};

template <typename T>
inline constexpr bool has_less_v = has_less<T>::value;

/**
 * @struct Variable
 * @brief Uchovává informace o jedné proměnné.
 */
struct Variable {
private:
  std::variant<std::monostate, int, long long, float, double, std::string> var_; /**< Interní hodnota */

public:
  /**
   * @brief Výchozí konstruktor.
   */
  Variable() = default;

  /**
   * @brief Konstruktor z řetězcového typu, názvu a hodnoty.
   * @param type  Datový typ jako string (např. "int").
   * @param name  Název proměnné.
   * @param value Hodnota proměnné jako string.
   */
  Variable(const std::string &type, const std::string &name,
           const std::string &value)
      : Type(type), Name(name), Value(value) {}

  TypeType Type;   /**< Datový typ proměnné. */
  NameType Name;   /**< Název proměnné. */
  ValueType Value; /**< Hodnota proměnné jako string. */

  /**
   * @brief Výčet podporovaných typů pro extrakci.
   */
  enum knownTypes { INT, LLONG, FLOAT, DOUBLE, STRING };

  /**
   * @brief Extrahuje hodnotu proměnné v daném typu.
   * @tparam T Jeden z knownTypes.
   * @return Optional s hodnotou nebo string pro STRING.
   */
  template <knownTypes T>
  auto ExtractValue() const {
    if constexpr (T == INT) {
      return Utils::StringToNumeric<int>(Value).Get<int>();
    } else if constexpr (T == LLONG) {
      return Utils::StringToNumeric<long long>(Value).Get<long long>();
    } else if constexpr (T == DOUBLE) {
      return Utils::StringToNumeric<double>(Value).Get<double>();
    } else if constexpr (T == STRING) {
      return Value;
    }
  }

  /**
   * @brief Vrací tuple {Type, Name, Value}.
   */
  [[nodiscard]] std::tuple<TypeType, NameType, ValueType> Tuple() const {
    return std::make_tuple(Type, Name, Value);
  }

  /**
   * @brief Porovnání proměnných podle Name, Type a Value.
   */
  bool operator<(const Variable &other) const {
    if (Name != other.Name) return Name < other.Name;
    if (Type != other.Type) return Type < other.Type;
    return Value < other.Value;
  }

  /**
   * @brief Výpis proměnné do streamu.
   */
  friend std::ostream &operator<<(std::ostream &os, const Variable &v) {
    os << absl::StrFormat("%s: %s := %s", v.Name, v.Type, v.Value);
    return os;
  }
};

/**
 * @struct VariableGroup
 * @brief Kolekce proměnných s možností vkládání a vyhledávání.
 */
struct VariableGroup {
private:
  absl::btree_set<Variable> vars_; /**< Množina proměnných. */

public:
  /**
   * @brief Výchozí konstruktor.
   */
  VariableGroup() = default;

  /**
   * @brief Vrací všechny proměnné.
   */
  [[nodiscard]] absl::btree_set<Variable> Get() const { return vars_; }

  /**
   * @brief Přidá proměnnou do skupiny.
   */
  VariableGroup &operator<<(const Variable &var) {
    vars_.insert(var);
    return *this;
  }

  /**
   * @brief Výpis všech proměnných do streamu.
   */
  friend std::ostream &operator<<(std::ostream &os,
                                  const VariableGroup &group) {
    for (const auto &v : group.vars_) os << v << std::endl;
    return os;
  }

  /**
   * @brief Hledá proměnnou podle jména.
   * @param name Jméno proměnné.
   * @return Optional s proměnnou nebo nullopt.
   */
  [[nodiscard]] std::optional<Variable> Find(const std::string &name) const {
    for (const auto &v : vars_) {
      if (v.Name == name) return v;
    }
    return std::nullopt;
  }
};

}  // namespace types
