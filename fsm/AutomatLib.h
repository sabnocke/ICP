#pragma once
#include <absl/strings/str_join.h>

#include <functional>
#include <iomanip>
#include <sstream>
#include <string>

#include "ParserLib.h"
#include "Utils.h"
#include <range/v3/view.hpp>
#include "types/all_types.h"
#include "external/sol.hpp"


namespace ParserLib {
struct TransitionRecord;
struct VariableRecord;
}  // namespace ParserLib

namespace AutomatLib {
using namespace types;




class Automat {
 public:
  void addState(const std::tuple<std::string, std::string> &result) {
    states << State{std::get<0>(result), std::get<1>(result)};
  }
  void addTransition(const Transition &result);
  void addVariable(const Variable &result)  {
    variables << result;
  }
  void addInput(const std::string &name);
  void addOutput(const std::string &name);
  std::string Name;
  std::string Comment;
  void PrepareHelperVariables();
  void PrepareStateActions() const;
  void PrepareVariables();
  void PrepareExecuteFunction();
  void PrepareUtilsFunctions();
  void PrepareSignals();
  void LinkDelays();
  void Create();
  StateGroup states;
  TransitionGroup transitions;
  std::vector<std::string> inputs;
  std::vector<std::string> outputs;
  VariableGroup variables;
  std::string currentState;
  bool firstRun = true;
  std::ostringstream oss;
  sol::state lua{};

 private:

};

}  // namespace AutomatLib
