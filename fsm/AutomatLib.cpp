#include "AutomatLib.h"

#include <fstream>
#include <string>
#include <vector>

namespace AutomatLib {
//TODO move following three functions into header and remove the PrepareStateActions()

void Automat::addInput(const std::string &name) { inputs.push_back(name); }

void Automat::addOutput(const std::string &name) { outputs.push_back(name); }

}  // namespace AutomatLib