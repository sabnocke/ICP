#include <absl/flags/flag.h>
#include <absl/log/absl_log.h>
#include <absl/log/initialize.h>
#include <absl/strings/str_format.h>

#include <fstream>
#include <iostream>

#include "Interpret.h"
#include "ParserLib.h"
#include "external/sol.hpp"

int main(const int argc, char** argv) {
  if (argc < 2) {
    ABSL_LOG(ERROR) << "Requires path to valid fsm definition";
    return 1;
  }

  absl::InitializeLog();

  try {
    Timer<> timer;
    timer.tick();
    auto parser = ParserLib::Parser();
    const auto automat = parser.parseAutomat(argv[1]);
    auto interpret = Interpreter::Interpret(automat);
    interpret.Prepare();
    timer.tock();
    std::cerr << "Parser and interpret creation took " << timer.duration<>().count() << "ms." << std::endl;
    timer.tick();
    interpret.Execute();
    timer.tock();
    perror("Error");

    std::cerr << "Execute took " << timer.duration<std::chrono::seconds>().count() << "s." << std::endl;

  } catch (const Utils::ProgramTermination&) {
    return 1;
  } catch (const std::exception& e) {
    std::cerr << e.what() << std::endl;
    return 1;
  }
  return 0;
}
