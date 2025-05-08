#include <absl/log/initialize.h>
#include <absl/log/log.h>

#include <charconv>
#include <functional>
#include <map>
#include <string>

#include "AutomatRuntime.h"
#include "Utils.h"

namespace tof5s {
using std::string;
using std::string_view;
using namespace AutomatRuntime;
using AutomatRuntime::Runtime;


std::function strtn = [](const string_view value) {return StringToNumeric(value);};
extern std::function<string(string)> valueof;

void ChangeState() {}

#pragma region inputs
// string in = {};
std::map<string, string> in_map = {{"in", ""}};
#pragma endregion

#pragma region outputs
// string out = {};
std::map<string, string> out_map = {{"out", ""}};  // <-- preferred way
#pragma endregion

#pragma region variables
int timeout = 5000;
#pragma endregion

template <typename T>
string output(const string& name, const T& value) {
  if (auto result = Utils::ToStringOpt(value); result.has_value()) {
    out_map[name] = result.value();
    return result.value();
  }
  LOG(WARNING) << "Failed to stringify: " << value << std::endl;
  return "";
}

#pragma region state actions
std::function idle = []() { output("out", 0); };
std::function active = []() { output("out", 1); };
std::function timing = []() {};
#pragma endregion

#pragma region transitions
void Idle2Active() {
  //TODO how to register change
  if (strtn(valueof("in")) == 1) {
    ChangeState();
  }
}
#pragma endregion

std::function<string(string)> valueof {
  [](const string& value) {
    if (in_map.contains(value)) return in_map[value];
    return string("");
  }
};

}  // namespace tof5s
