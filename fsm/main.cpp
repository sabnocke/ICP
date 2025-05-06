#include <chrono>
#include <iostream>
#include <string>
#include "AutomatLib.h"
#include "Stopwatch.h"
#include "parser.h"



int main() {
  Timer timer;
  timer.tick();
  const auto test = "IDLE --> ACTIVE: in [ atoi(valueof(\"in\")) == 1 ]";

  const auto result = Parser::parser::parseTransition(test);
  auto [from, to, input, cond, delay] = result.value_or(Parser::ParseTransitionRecord::Empty());
  std::cout << input << std::endl;

  timer.tock();
  std::cout << "Run time = " << timer.duration().count() << "ms\n";


}
