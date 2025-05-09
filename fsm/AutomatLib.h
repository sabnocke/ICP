#pragma once

#include <absl/container/btree_set.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_join.h>

#include <functional>
#include <iomanip>
#include <sstream>
#include <string>

#include "ParserLib.h"
#include "Utils.h"
#include <range/v3/view.hpp>
#include "types/all_types.h"


namespace ParserLib {
struct TransitionRecord;
struct VariableRecord;
}  // namespace ParserLib

namespace AutomatLib {
using namespace types;




class Automat {
 public:
  void addState(const std::tuple<std::string, std::string> &result);
  void addTransition(const Transition &result);
  void addVariable(const Variable &result);
  void addInput(const std::string &name);
  void addOutput(const std::string &name);
  std::string Name;
  std::string Comment;
  void PrepareHelperVariables();
  void PrepareIncludes();
  void PrepareExecuteFunction();
  StateGroup states;
  TransitionGroup transitions;
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
  VariableGroup variables;
  std::string currentState;
  bool firstRun = true;
  std::ostringstream oss;

 private:
};

}  // namespace AutomatLib
