
#include <absl/strings/str_format.h>
#include <iostream>
#include <fstream>
#include "ParserLib.h"
#include "Timing.h"
#include "external/sol.hpp"


void parserTest() {
  ParserLib::Parser parser;
  if (auto res = parser.parseState("IDLE : [ output(\"out\", 0) ]");
      res.has_value()) {
    auto [name, cond] = res.value();
    std::cout << absl::StrFormat("state %s has action %s\n", name, cond);
  }
  if (auto res = parser.parseVariable("    int timeout = 5000");
      res.has_value()) {
    std::cout << res.value() << std::endl;
  }
  if (auto res = parser.parseTransition(
          "    IDLE --> ACTIVE: in [ atoi(valueof(\"in\")) == 1 ]");
      res.has_value()) {
    std::cout << res.value() << std::endl;
  }
  std::cout << Utils::Quote("IDLE : { output(\"out\", 0) }") << std::endl;

  //? Get is necessary for getting the value if conv was directly printed it would output correctly 10
  auto conv = Utils::StringToNumeric<int>("10");
  if (auto c = conv.Get<int>(); c.has_value()) {
    std::cout << c.value() << std::endl;
  }
}
//
//   auto r = parser.parseAutomat("fsmDefinition.txt");
//   std::cout << r.states.Size() << std::endl;
//   std::cout << r.states << std::endl;
//   std::cout << "IN: " << absl::StrJoin(r.inputs, ", ") << std::endl;
//   std::cout << "OUT: " << absl::StrJoin(r.outputs, ", ") << std::endl;
//   std::cout << r.variables << std::endl;
//   std::cout << r.transitions << std::endl;
// }
//
void luaTest() {
  sol::state lua{};
  int x = 0;
  lua.set_function("beep", [&]() { ++x; });
  lua.script("beep()");
  assert(x == 1);
  std::cout << x << std::endl;
}

int main(int argc, char** argv) {
  if (argc != 2) {
    std::cerr << "Requires path to valid fsm definition" << std::endl;
  }
  auto parser = ParserLib::Parser();
  auto automat = parser.parseAutomat(argv[1]);
  automat.PrepareUtilsFunctions();
  automat.PrepareStateActions();
  automat.PrepareVariables();
  automat.PrepareSignals();
  automat.LinkDelays();

  std::cout << automat.transitions << std::endl;

  return 0;
}
