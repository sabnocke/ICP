#include <absl/log/initialize.h>
#include <absl/log/log.h>
#include <absl/strings/str_format.h>

#include <iostream>

#include "ParserLib.h"
#include "absl/flags/flag.h"


int main() {
  absl::InitializeLog();

  ParserLib::Parser parser;
  if (auto res = parser.parseState("IDLE : { output(\"out\", 0) }"); res.has_value()) {
    auto [name, cond] = res.value();
    std::cout << absl::StrFormat("state %s has condition %s\n", name, cond);
  }
  if (auto res = parser.parseVariable("    int timeout = 5000"); res.has_value()) {
    std::cout << res.value() << std::endl;
  }
  if (auto res = parser.parseTransition("    IDLE --> ACTIVE: in [ atoi(valueof(\"in\")) == 1 ]"); res.has_value()) {
    std::cout << res.value() << std::endl;
  }
  return 0;


}
