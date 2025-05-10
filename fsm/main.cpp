#include <absl/log/check.h>
#include <absl/strings/str_format.h>
#include "absl/log/absl_check.h"
#include <cassert>

#include <iostream>

#include "AutomatLib.h"
#include "ParserLib.h"

#include "external/sol.hpp"
#include "lua.hpp"
#include "types/all_types.h"

void parserTest() {
  ParserLib::Parser parser;
  if (auto res = parser.parseState("IDLE : [ output(\"out\", 0) ]"); res.has_value()) {
    auto [name, cond] = res.value();
    std::cout << absl::StrFormat("state %s has action %s\n", name, cond);
  }
  if (auto res = parser.parseVariable("    int timeout = 5000"); res.has_value()) {
    std::cout << res.value() << std::endl;
  }
  if (auto res = parser.parseTransition("    IDLE --> ACTIVE: in [ atoi(valueof(\"in\")) == 1 ]"); res.has_value()) {
    std::cout << res.value() << std::endl;
  }
  std::cout << Utils::Quote("IDLE : { output(\"out\", 0) }") << std::endl;

  //? Get is necessary for getting the value if conv was directly printed it would output correctly 10
  auto conv = Utils::StringToNumeric<int>("10");
  if (auto c = conv.Get<int>(); c.has_value()) {
    std::cout << c.value() << std::endl;
  }

  auto r = parser.parseAutomat("fsmDefinition.txt");
  std::cout << r.states.Size() << std::endl;
  std::cout << r.states << std::endl;
  std::cout << "IN: " << absl::StrJoin(r.inputs, ", ") << std::endl;
  std::cout << "OUT: " << absl::StrJoin(r.outputs, ", ") << std::endl;
  std::cout << r.variables << std::endl;
  std::cout << r.transitions << std::endl;
}

int main() {

  sol::state lua{};
  int x = 0;
  lua.set_function("beep", [&](){++x;});
  lua.script("beep()");
  assert(x == 1);
  std::cout << x << std::endl;
  return 0;
}
