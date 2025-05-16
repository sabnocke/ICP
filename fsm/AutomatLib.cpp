#include "AutomatLib.h"

#include <fstream>
#include <string>
#include <utility>
#include <vector>
#include "types/all_types.h"


namespace AutomatLib {
//TODO move following three functions into header and remove the PrepareStateActions()

void Automat::addInput(const std::string &name) { inputs.push_back(name); }

void Automat::addOutput(const std::string &name) { outputs.push_back(name); }

[[deprecated("Moved to interpreter")]]
void Automat::PrepareStateActions() const {
  for (const auto &[Name, Action] : states.GetPairs()) {
    if (Action.empty())
      continue;
    std::cerr << "New action: " << Action << "for state: " << Name << std::endl;
  }
}

}  // namespace AutomatLib