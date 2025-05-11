#include "AutomatLib.h"

#include <absl/strings/match.h>

#include <fstream>
#include <optional>
#include <ostream>
#include <string>
#include <utility>
#include <vector>

#include "../vcpkg/buildtrees/abseil/src/20250127.1-a0a219bf72.clean/absl/strings/match.h"
#include "ParserLib.h"
#include "Utils.h"
#include "range/v3/view/map.hpp"
#include "types/all_types.h"


namespace AutomatLib {

void Automat::addTransition(const Transition &result) { transitions << result; }

void Automat::addInput(const std::string &name) { inputs.push_back(name); }

void Automat::addOutput(const std::string &name) { outputs.push_back(name); }





void Automat::PrepareStateActions() const {
  for (const auto &[Name, Action] : states.GetPairs()) {
    if (Action.empty())
      continue;
    std::cerr << "New action: " << Action << "for state: " << Name << std::endl;
  }
}





void Automat::PrepareHelperVariables() {
}

void Automat::PrepareExecuteFunction() {
}

void Automat::Create() {
}

}  // namespace AutomatLib