#include <absl/flags/flag.h>
#include <absl/log/absl_log.h>
#include <absl/log/globals.h>
#include <absl/log/initialize.h>
#include <absl/strings/str_format.h>

#include <cassert>
#include <fstream>
#include <iostream>

#include "Interpret.h"
#include "ParserLib.h"
#include "external/sol.hpp"


void parserTest() {
  ParserLib::Parser parser;

  auto [name, cond] = parser.parseState("state IDLE [ output(\"out\", 0) ]");
  std::cout << absl::StrFormat("state %s has action %s\n", name, cond);

  std::cout << parser.parseVariable("    int timeout = 5000") << std::endl;

  auto res = parser.parseTransition(
      "    IDLE --> ACTIVE: in [ atoi(valueof(\"in\")) == 1 ]");
  std::cout << res << std::endl;

  std::cout << Utils::Quote("IDLE : { output(\"out\", 0) }") << std::endl;

  auto conv = Utils::StringToNumeric<int>("10");
  if (auto c = conv; c.has_value()) {
    std::cout << c.value() << std::endl;
  }
}

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
    ABSL_LOG(ERROR) << "Requires path to valid fsm definition";
    return 1;
  }

  absl::InitializeLog();
  // absl::SetMinLogLevel(absl::LogSeverityAtLeast::kInfo); //<-- this doesn't do anything of use

  try {
    ABSL_LOG(INFO) << "Hello, World!";
    parserTest();
  } catch (const Utils::ProgramTermination&) {
    return 1;
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
