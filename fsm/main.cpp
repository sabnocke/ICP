
#include <absl/strings/str_format.h>
#include <absl/log/absl_log.h>
#include <absl/log/initialize.h>
#include <absl/flags/flag.h>
#include <absl/log/globals.h>
#include <spdlog/spdlog.h>
#include <absl/flags/flag.h>

#include <cassert>
#include <fstream>
#include <iostream>

#include "Interpret.h"
#include "ParserLib.h"
#include "external/sol.hpp"

//TODO add log sink to a file

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
  if (auto c = conv; c.has_value()) {
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

int main(const int argc, char** argv) {
  if (argc < 2) {
    spdlog::error("Requires path to valid fsm definition");
    return 1;
  }
  try {
    spdlog::info("Hello World");
  } catch (const Utils::ProgramTermination& pt) {
    if (pt.lineNumber == -1)
      spdlog::error(pt.what());
    spdlog::error(absl::StrFormat("%s at %llu", pt.what(), pt.lineNumber));
  }
  // sol::state lua{};
  // lua.open_libraries(sol::lib::base, sol::lib::package);
  // auto parser = ParserLib::Parser();
  // auto automat = parser.parseAutomat(argv[1]);
  // Interpreter::Interpret interpreter(automat);
  // interpreter.Prepare();
  // std::cout << automat.transitions << std::endl;
  // std::cout << interpreter.transitionGroup << std::endl;
  // interpreter.Execute(false);
  // Interpreter::Interpret<std::string>::simpleExample();


  return 0;
}
