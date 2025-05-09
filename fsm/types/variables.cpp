//
// Created by ReWyn on 09.05.2025.
//

#include "variables.h"
#include "../Utils.h"

namespace types {
absl::btree_set<std::string> VariableGroup::Format() {
  absl::btree_set<std::string> coll = {};
  for (const auto &[Name, Value, Type] : vars) {
    if (Utils::Contains(Type, "string")) {
      Utils::Remove(Value, "\"");
      auto s = absl::StrFormat("%s %s = \"%s\"", Type, Name, Value);
      coll.insert(s);
    } else {
      coll.insert(absl::StrFormat("%s %s = %s", Type, Name, Value));
    }
  }
  return coll;
}
}