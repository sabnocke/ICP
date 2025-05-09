//
// Created by ReWyn on 09.05.2025.
//

#pragma once

#include <string>
#include <absl/strings/str_format.h>
#include <absl/container/btree_set.h>


namespace types {
struct Variable {
  Variable() = default;
  Variable(const std::string& type, const std::string& name, const std::string& value) {
    Name = name;
    Type = type;
    Value = value;
  }
  std::string Name;
  std::string Value;
  std::string Type;
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

  VariableGroup &operator+=(const Variable &other) {
    vars.insert(other);
    return *this;
  }
  absl::btree_set<std::string> Format();
};
}

