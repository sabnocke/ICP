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
#include <utility>

#include "../Utils.h"

namespace types {

/**
 * @class Variable
 * @brief Uchovává informace o jedné proměnné.
 */
class Variable {
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
  Variable(std::string &&type, std::string &&name, std::string &&value)
      : Type(std::move(type)), Name(std::move(name)), Value(std::move(value)) {}

  std::string Type;   /**< Datový typ proměnné. */
  std::string Name;   /**< Název proměnné. */
  std::string Value; /**< Hodnota proměnné jako string. */

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
      return Utils::StringToNumeric<int>(Value);
    } else if constexpr (T == LLONG) {
      return Utils::StringToNumeric<long long>(Value);
    } else if constexpr (T == DOUBLE) {
      return Utils::StringToNumeric<double>(Value);
    } else if constexpr (T == STRING) {
      return Value;
    }
  }

  /**
   * @brief Vrací tuple {Type, Name, Value}.
   */
  [[nodiscard]] std::tuple<std::string, std::string, std::string> Tuple() const {
    return std::make_tuple(Type, Name, Value);
  }

  /**
   * @brief Porovnání proměnných podle Name, Type a Value.
   */
  bool operator<(const Variable &other) const {
    if (Name != other.Name)
      return Name < other.Name;
    if (Type != other.Type)
      return Type < other.Type;
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
 * @class VariableGroup
 * @brief Kolekce proměnných s možností vkládání a vyhledávání.
 */
class VariableGroup {
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

  auto begin() { return vars_.begin(); }
  auto end() { return vars_.end(); }
  auto size() const { return vars_.size(); }

  VariableGroup Add(Variable &&var) {
    vars_.insert(std::move(var));
    return *this;
  }

  /**
   * @brief Přidá proměnnou do skupiny.
   */
  VariableGroup &operator<<(Variable &&var) {
    vars_.insert(std::move(var));
    return *this;
  }

  VariableGroup &operator<<(const Variable &&var) {
    vars_.insert(var);
    return *this;
  }

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
      if (v.Name == name)
        return v;
    }
    return std::nullopt;
  }
  [[nodiscard]] bool Contains(const Variable &name) const {
    return vars_.contains(name);
  }
};

}  // namespace types
