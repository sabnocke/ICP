#include <string>
#include <functional>
#include <helperClass>

/* TODO create separate class for helper functions
 * output
 * valueof
 * move
 * wait() ~ could return bool
 * elapsed() ~ will need a way of measuring how long is current state active
 * hasChanged(signal) ~ have to remember that change happened
 * */

#pragma region helper variables
std::string CurrentState = {};
bool terminate = false;

#pragma endregion

#pragma region inputs
std::string $0 = {};
// as many times as there is input signals
// can they be like this or should i use struct? or something similar?
#pragma endregion

#pragma region outputs
std::string $0 = {};

#pragma endregion

#pragma region state functions
// these are all functions that don't accept anything and return string
std::function<std::string()> ($stateName)Function = [](){ $action }; // the action itself isn't verified (by developer, compiler does verify it)

#pragma endregion

#pragma region variables
$0 $1 = $2;

#pragma endregion

// Transition
// Any transition is of one type from t1a, t1b, t1c, t2a, t2b, t2c, t3
// Transition could be a lambda, albeit there is no benefit for it
// where is newState stored? a closure?
void t1a(std::string input) {
  if (hasChanged(input)) move(newState);
}
void t1b(std::function<bool()> cond) {
  if (cond()) {
    move(newState);
  }
}
void t1c(int delay) {
  wait(delay);
  move(newState);
}
void t2a(std::string input, std::function<bool()> cond) {
  if (hasChanged(input) && cond()) move(newState);
}

void t2b(std::string input, int delay) {
  if (hasChanged(input) {
    wait(delay);
    move(newState);
  }
}
void t2c(std::function<bool()> cond, int delay) {
    if (cond()) {
      wait(delay);
      move(newState);
    }
}

// use string? or some custom object?
void t3(std::string input, std::function<bool()> cond, int delay) {
    if (hasChanged(input) && cond()) {
      wait(delay);
      move(newState);
    }
}

int main(void) {
    // Main should just run the execution in while(true) loop

}