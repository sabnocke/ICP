#include <functional>
#include <list>
#include <string>

#include "AutomatRuntime.h"
#include "Timing.h"

namespace tof5s {
using std::string;
using std::string_view;
using namespace AutomatRuntime;

using std::function;
using AutomatRuntime::Runtime;

#define INPUT(name) Runtime::RegisterSignal(in_map, name)
#define OUTPUT(name) Runtime::RegisterSignal(out_map, name)


SignalsType in_map = {};
SignalsType out_map = {};
function strtn = [](const string_view value) {return StringToNumeric(value);};
function valueof = [&](const string& name) {return Runtime::Load(in_map, name);};
template <typename T> function output = [](const string& name, const T& value) {return Runtime::Store(out_map, value, name);};

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
function idle = []() { output("out", 0); };
function active = []() { output("out", 1); };
function timing = []() {};
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
  bool terminate = false;
  TransitionGroup group;
  Timing::Timer timer;
  group.Add(
      "IDLE", "ACTIVE", "in", []() { return strtn(valueof("in")) == 1; }, timeout);
  group.Add(
      "ACTIVE", "TIMING", "in", []() { return strtn(valueof("in")) == 0; }, timeout);
  group.Add(
      "TIMING", "ACTIVE", "in", []() { return strtn(valueof("in")) == 1; }, timeout);
  group.Add("ACTIVE", "IDLE", "", std::nullopt, timeout);
  group.RegisterAction("IDLE", idle);
  group.RegisterAction("ACTIVE", active);
  group.RegisterAction("TIMING", timing);
  group.GroupTransitions();

  while (!terminate) {
    auto res = group.Retrieve(activeState);
    if (res.IsEmpty()) {   //* alternatively "independent" means any viable
      continue; // or end?
    }
    if (auto res2 = res.WhereCondTimer(); res2.Some()) {
      timer.SetTimers(res2);
    } else if (auto res3 = res.WhereCond(); res3.Some()) {
      if (const auto tr = res3.First(); tr.has_value()) {
        auto t = tr.value().get().to;
        std::cerr << "New state: " << t << std::endl;
      }
      // Else won't be needed if group was empty it wouldn't get this far
    }
    //TODO remove timers of inactive states, if there are any
    //* Unnecessary Timer terminates any leftover timers before returning
    //TODO wait for a timer or input event
    //* I can do timers
    //* Input relies on communication method
    if (std::optional<Transition> result = timer.Start(); result.has_value()) {
      const auto& t = result.value();
      std::cerr << "New state: " << t.to << std::endl;
    }
    if (false /*TODO if it is input event, then find the correct new state */);
      if (false /*TODO if it is non-delayed transit, then move there */);
      if (false /*TODO if it is delayed transit, then wait */);
    if (false /*TODO if the input event is to terminate, then do so */);
  }
}

}  // namespace tof5s
