#include "AutomatLib.h"

#include <fstream>
#include <string>
#include <utility>
#include <vector>
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