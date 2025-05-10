//
// Created by ReWyn on 09.05.2025.
//

#pragma once

#include <absl/container/btree_set.h>
#include <absl/strings/str_format.h>

#include <optional>
#include <string>

#include "numeric.h"

namespace types {
using TypeType = std::string;
using NameType = std::string;
using ValueType = std::string;

template <typename T, typename = void>
struct has_less : std::false_type {};

template <typename T>
struct has_less<T,
std::void_t<decltype(std::declval<T&>() < std::declval<T&>())>> : std::true_type{};

template <typename T>
inline constexpr bool has_less_v = has_less<T>::value;

struct Variable {
  Variable() = default;
  Variable(const std::string& type, const std::string& name, const std::string& value) {
    Name = name;
    Type = type;
    Value = value;
  }
  TypeType Type;
  NameType Name;
  ValueType Value;

  std::tuple<TypeType, NameType, ValueType> Tuple() const {
    return std::make_tuple(Type, Name, Value);
  }

  bool operator<(const Variable &other) const {
    return Name < other.Name && Value < other.Value && Type < other.Type;
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
 std::optional<int> InterpretInt();
  void InterpretFloat();
  void InterpretString();
  void InterpretBool();
  void InterpretArray();

  VariableGroup &operator<<(const Variable &other) {
    vars.insert(other);
    return *this;
  }

  friend std::ostream& operator<<(std::ostream &os, const VariableGroup &group) {
    for (const auto &var : group.vars) {
      os << var << std::endl;
    }
    return os;
  }

  absl::btree_set<std::string> Format();
};
}

