#include <functional>
#include <string>
#include "AutomatRuntime.h"

#define BUILDING

#ifdef BUILDING
extern std::function<std::string()> Action;
extern std::function<void(int s)> Move;
extern std::function<bool(std::string&)> HasChanged;
extern int newState;
#define type std::any;
#define value 0
#define ActionSymbol return "0"

#endif

namespace Name {} // place everything into a namespace

using std::string;
using namespace AutomatRuntime;

/* TODO create separate class for helper functions
 * output
 * valueof
 * move
 * wait() ~ could return bool
 * elapsed() ~ will need a way of measuring how long is current state active
 * hasChanged(signal) ~ have to remember that change happened
 * */

#pragma region helper variables
string CurrentState = {};
bool terminate = false;

#pragma endregion

#pragma region inputs
string i = {};
// as many times as there is input signals
// can they be like this or should i use struct? or something similar?
#pragma endregion

#pragma region outputs
string o = {};

#pragma endregion

#pragma region state functions
// these are all functions that don't accept anything and return string
std::function<std::string()> stateName = []() {
  ActionSymbol;
};  // the action itself isn't verified (by developer, compiler does verify it)

#pragma endregion

#pragma region variables
std::string a = "a";

#pragma endregion

// Transition
// Any transition is of one type from t1a, t1b, t1c, t2a, t2b, t2c, t3
// Transition could be a lambda, albeit there is no benefit for it
// where is newState stored? a closure?
void t1a(std::string input) {
  if (HasChanged(input))
    Move(newState);
}
void t1b(const std::function<bool()>& cond) {
  if (cond()) {
    Move(newState);
  }
}
void t1c(const int delay) {
  Runtime::WaitFor(delay);
  Move(newState);
}
void t2a(std::string& input, const std::function<bool()>& cond) {
  if (HasChanged(input) && cond())
    Move(newState);
}

void t2b(std::string& input, const int delay) {
  if (HasChanged(input)) {
    Runtime::WaitFor(delay);
    Move(newState);
  }
}
void t2c(const std::function<bool()>& cond, const int delay) {
  if (cond()) {
    Runtime::WaitFor(delay);
    Move(newState);
  }
}

// use string? or some custom object?
void t3(std::string input, const std::function<bool()>& cond, const int delay) {
  if (HasChanged(input) && cond()) {
    Runtime::WaitFor(delay);
    Move(newState);
  }
}

int main_entry() {
  // Main should just run the execution in while(true) loop
  return 0;
}