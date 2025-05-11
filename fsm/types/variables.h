//
// Created by ReWyn on 09.05.2025.
//

#pragma once

#include <absl/container/btree_set.h>
#include <absl/strings/str_format.h>

#include <optional>
#include <string>

#include "../Utils.h"
#include "numeric.h"

namespace types {
using TypeType = std::string;
using NameType = std::string;
using ValueType = std::string;

template <typename T, typename = void>
struct has_less : std::false_type {};

template <typename T>
struct has_less<
    T, std::void_t<decltype(std::declval<T &>() < std::declval<T &>())>>
    : std::true_type {};

template <typename T>
inline constexpr bool has_less_v = has_less<T>::value;

struct Variable {
private:
  std::variant<std::monostate, int, long long, float, double, std::string> var;

public:
  Variable() = default;
  Variable(const std::string &type, const std::string &name,
           const std::string &value) {
    Name = name;
    Type = type;
    Value = value;
    if (type != "string") {

    }
  }
  TypeType Type;
  NameType Name;
  ValueType Value;

  enum knownTypes {
    INT,
    LLONG,
    FLOAT,
    DOUBLE,
    STRING,
  };

  template <knownTypes T>
  auto ExtractValue() {
    if constexpr (T == INT) {
      return Utils::StringToNumeric<int>(Value).Get<int>();
    } else if constexpr (T == LLONG) {
      return Utils::StringToNumeric<long long>(Value).Get<long long>();
    } /*else if constexpr (T == FLOAT) {
      return Utils::StringToNumeric<float>(Value).Get<float>();
    }*/
    else if constexpr (T == DOUBLE) {
      return Utils::StringToNumeric<double>(Value).Get<double>();
    } else if constexpr (T == STRING) {
      return Value;
    }
  }

  [[nodiscard]] std::tuple<TypeType, NameType, ValueType> Tuple() const {
    return std::make_tuple(Type, Name, Value);
  }

  bool operator<(Variable const& o) const {
    if (Name != o.Name)  return Name < o.Name;
    if (Type != o.Type)  return Type < o.Type;
    return Value < o.Value;
  }

  friend std::ostream &operator<<(std::ostream &os, const Variable &var) {
    os << absl::StrFormat("%s: %s := %s", var.Name, var.Type, var.Value);
    return os;
  }
};

struct VariableGroup {
 private:
  absl::btree_set<Variable> vars;

 public:
  VariableGroup() = default;
  [[nodiscard]] absl::btree_set<Variable> Get() const {
    return vars;
  }

  VariableGroup &operator<<(const Variable &other) {
    vars.insert(other);
    return *this;
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const VariableGroup &group) {
    for (const auto &var : group.vars) {
      os << var << std::endl;
    }
    return os;
  }

  [[nodiscard]] std::optional<Variable> Find(const std::string& name) const {
    for (const auto& var : vars) {
      if (var.Name == name) {
        return var;
      }
    }
    return std::nullopt;
  }
};
}  // namespace types
