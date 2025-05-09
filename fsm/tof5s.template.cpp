#include <functional>
#include <list>
#include <string>

#include "AutomatRuntime.h"

namespace tof5s {
using std::string;
using std::string_view;
using namespace AutomatRuntime;
using AutomatRuntime::Runtime;

#define INPUT(name) Runtime::RegisterSignal(in_map, name)
#define OUTPUT(name) Runtime::RegisterSignal(out_map, name)

SignalsType in_map = {};
SignalsType out_map = {};
std::function strtn = [](const string_view value) {return StringToNumeric(value);};
std::function valueof = [&](const string& name) {return Runtime::Load(in_map, name);};
template <typename T> std::function output = [](const string& name, const T& value) {return Runtime::Store(out_map, value, name);};

inline bool terminate = false;

void ChangeState() {}

enum States {ACTIVE, IDLE, TIMING};
std::list<string> states = {"ACTIVE", "IDLE", "TIMING"};
string activeState = "IDLE";

#pragma region inputs
INPUT("in");
#pragma endregion

#pragma region outputs
OUTPUT("out");
#pragma endregion

#pragma region variables
int timeout = 5000;
#pragma endregion



#pragma region state actions
std::function idle = []() { output("out", 0); };
std::function active = []() { output("out", 1); };
std::function timing = []() {};
#pragma endregion

#pragma region transitions
void Idle2Active() {
  //TODO how to register change
  if (strtn(valueof("in")) == 1) {
    activeState = "ACTIVE";
    active();
  }
}
void Active2Timing() {
  if (strtn(valueof("in")) == 0) {
    activeState = "TIMING";
    timing();
  }
}
#pragma endregion

void execute() {
  TransitionGroup group;
  group.Add(
      "IDLE", "ACTIVE", "in", []() { return strtn(valueof("in")) == 1; }, "");
  group.Add(
      "ACTIVE", "TIMING", "in", []() { return strtn(valueof("in")) == 0; }, "");
  group.Add(
      "TIMING", "ACTIVE", "in", []() { return strtn(valueof("in")) == 1; }, "");
  group.Add("ACTIVE", "IDLE", "", std::nullopt, "timeout");
  group.GroupTransitions();
  //TODO pass transitions and states from AutomatLib to AutomatRuntime
  //TODO AutomatRuntime should define a filter that retrieves applicable states


  while (!terminate) {
    auto res = group.Retrieve(activeState);
    if (res.Some()) {   //* alternatively "independent" means any viable
      if (auto res2 = res.WhereTimer(); res2.Some()) {
        //TODO there are timers to be set
      } else if (auto res3 = res.WhereCond(); res3.Some()) {
        //TODO there are immediate transitions
      }
    }
    //TODO remove timers of inactive states, if there are any
    //? first has to maintain which timers were set in which state
    //TODO wait for a timer or input event
    //? i need to be able to do this...
    if (false /*TODO if timeout happened find out which one it was and move there */);
    if (false /*TODO if it is input event, then find the correct new state */);
      if (false /*TODO if it is non-delayed transit, then move there */);
      if (false /*TODO if it is delayed transit, then wait */);
    if (false /*TODO if the input event is to terminate, then do so */);
  }
}

}  // namespace tof5s
