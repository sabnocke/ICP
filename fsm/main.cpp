#include <chrono>
#include <iostream>
#include <string>
#include "AutomatLib.h"
#include "Stopwatch.h"
#include "absl/log/initialize.h"
#include "parser.h"
#include "absl/strings/str_format.h"
#include "Utils.h"


int main() {
  absl::InitializeLog();
  Timer timer;
  timer.tick();
  const auto test = "IDLE --> ACTIVE: in [ atoi(valueof(\"in\")) == 1 ]";

  const std::string tt = "  ";
  const auto r1  = Utils::Trim(tt);
  std::cout << "result: " << r1 << std::endl;

  timer.tock();
  std::cout << "Run time = " << timer.duration().count() << "ms\n";
  std::cout << absl::StrFormat("Run time = %dms\n", timer.duration().count());


}
