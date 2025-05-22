#include "ParserLib.h"

#include <absl/strings/str_split.h>
#include <re2/re2.h>
#include <spdlog/spdlog.h>

#include <fstream>
#include <iostream>

#include "AutomatLib.h"
#include "Utils.h"

//TODO remove NOTHING macro
//TODO move DEFAULT_NAME as a const field into parser class
//TODO remove commented out code
//TODO change std::cerr to ProgramTermination + spdlog
//TODO that might simplify ProgramTermination itself

#define DEFAULT_NAME "Unknown"
#define NOTHING std::nullopt

namespace ParserLib {

Parser::Parser() {
  RE2::Options options;
  options.set_case_sensitive(false);
  name_pattern_ = std::make_unique<RE2>(R"(^\s*Name\s*:\s*(?<c>.*)$)", options);
  comment_pattern_ = std::make_unique<RE2>(R"(^.*?:\s*?(?<c>.*)$)", options);
  variables_pattern_ = std::make_unique<RE2>(
      R"((?<type>\w+) (?<name>\w+) = (?<value>\w+))", options);
  states_pattern_ =
      std::make_unique<RE2>(R"(state (?<name>\w+) *\[(?<code>.*)\])", options);
  transitions_pattern_ = std::make_unique<RE2>(
      R"(^\s*(?<from>\w+)\s*-->\s*(?<to>\w+)\s*:\s*(?:(?<input>\w*)?\s*(?<cond>\[.*\])?\s*@?\s*(\w*)?)$)",
      options);
  input_pattern_ = std::make_unique<RE2>(R"(^.*?:?\s*(?<name>\w*)$)", options);
  output_pattern_ = std::make_unique<RE2>(R"(^.*?:?\s*(?<name>\w*)$)", options);

  if (!name_pattern_->ok() || !comment_pattern_->ok() ||
      !variables_pattern_->ok() || !states_pattern_->ok() ||
      !transitions_pattern_->ok()) {
    spdlog::critical("Failed parsing regex patterns");
    throw Utils::ProgramTermination();
  }
}

AutomatLib::Automat Parser::parseAutomat(const std::string &file) {
  AutomatLib::Automat automat;
  std::ifstream ifs(file);
  if (!ifs) {
    std::cerr << "Can't open file " << file << std::endl;
    return std::move(automat);
  }
  std::string line;

  while (std::getline(ifs, line)) {
    //TODO remove this
    std::cout << line << std::endl;  //* <-- line printing <--
    lineNumber++;
    if (line.empty())
      continue;

    if (Utils::Contains(line, "Name")) {
      ActualSection = Name;
      SectionHandler(line, automat);  // In case of: Name: name
      continue;
    }
    if (Utils::Contains(line, '#')) {
      // Comments are ignored
      continue;
    }
    if (Utils::Contains(line, "Variable")) {
      ActualSection = Variables;
      continue;
    }
    if (Utils::Contains(line, "State")) {
      ActualSection = States;
      std::cerr << "Parsing states" << std::endl;
      SectionHandler(line, automat);
      continue;
    }
    if (Utils::Contains(line, "Transition")) {
      ActualSection = Transitions;
      continue;
    }
    if (Utils::Contains(line, "Input")) {
      ActualSection = Inputs;
      SectionHandler(line, automat);
      continue;
    }
    if (Utils::Contains(line, "Output")) {
      ActualSection = Outputs;
      SectionHandler(line, automat);
      continue;
    }
    SectionHandler(line, automat);
  }
  return std::move(automat);
}

bool Parser::SectionHandler(const std::string &line,
                            AutomatLib::Automat &automat) const {
  switch (ActualSection) {
    case Name:
      automat.Name = extractName(line);
      return true;
      /*if (const auto result = ; result.has_value()) {


      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;*/
    case Comment:
      return true;
    case States: {
      automat.addState(parseState(line));
      return true;
      /*if (const auto result = ; result.has_value()) {

      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;*/
    }
    case Transitions: {
      automat.addTransition(parseTransition(line));
      return true;
      /*spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;*/
    }
    case Variables:
      automat.addVariable(parseVariable(line));
      return true;
      /*if (const auto result = ; result.has_value()) {

        return true;
      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;*/
    case Inputs:
      automat.addInput(parseSignal(line));
      return true;
      /*if (const auto result = ; result.has_value()) {

      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;*/
    case Outputs:
      automat.addOutput(parseSignal(line));
      return true;
      /*if (const auto result = ; result.has_value()) {

      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;*/
  }

  spdlog::warn("Reached SectionHandler but no valid section was chosen.");
  // throw Utils::ProgramTermination();
  return false;
}

State<> Parser::parseState(const std::string &line) const {
  const auto trimmed = Utils::Trim(line);
  std::string name, code;
  //! this works for single line state definition
  //TODO test how this works with [] being split on different lines

  /*const auto res = Utils::FindAll(trimmed, '[', ']');
  std::cout << trimmed << " " << std::boolalpha << res << std::endl;*/

  /*if (!res) {
    // Failsafe in case of brackets being on different lines
    std::cerr << absl::StrFormat(
        "Missing opening/closing bracket for state definition\n", trimmed);
    return NOTHING;
  }*/
  if (RE2::FullMatch(trimmed, *states_pattern_, &name, &code)) {
    return {Utils::Trim(name), Utils::Trim(code)};
  }

  spdlog::error("[{}] Malformed state definition: {}", lineNumber, line);
  throw Utils::ProgramTermination();
}

std::string Parser::extractName(const std::string &line) const {
  std::string name;

  if (RE2::FullMatch(line, *name_pattern_, &name) && !name.empty()) {
    if (auto trimmed = Utils::Trim(name); !trimmed.empty()) {
      return trimmed;
    }
  }

  spdlog::error("[{}] Malformed name definition: {}", lineNumber, line);
  throw Utils::ProgramTermination();
}

/*[[deprecated("Comments are ignored")]]
std::optional<std::string> Parser::extractComment(
    const std::string &line) const {
  std::string comment;
  RE2::Options options;
  options.set_case_sensitive(false);

  if (RE2::FullMatch(line, *comment_pattern_, &comment) && !comment.empty()) {
    if (auto trimmed = Utils::Trim(comment); !trimmed.empty())
      return trimmed;
  }  // there is possibility of "Comment:\n<comment>" which won't pass this
  if (auto trimmed = Utils::Trim(line); !trimmed.empty())
    return trimmed;

  return NOTHING;
}*/

Variable Parser::parseVariable(const std::string &line) const {
  const auto trimmed = Utils::Trim(line);
  std::string type, name, value;
  if (RE2::FullMatch(trimmed, *variables_pattern_, &type, &name, &value)) {
    return Variable{std::move(type), std::move(name), std::move(value)};
  }

  spdlog::error("[{}] Malformed variable definition: {}", lineNumber, line);
  throw Utils::ProgramTermination();
  /*return NOTHING;*/
}

Transition<> Parser::parseTransition(const std::string &line) const {
  std::string from, to, input, cond, delay;

  if (RE2::FullMatch(line, *transitions_pattern_, &from, &to, &input, &cond,
                     &delay)) {
    const auto cond2 = Utils::Remove(cond, '[');
    const auto cond3 = Utils::Remove(cond2, ']');
    return Transition{std::move(from), std::move(to), std::move(input),
                      Utils::Trim(cond3), std::move(delay)};
  }

  spdlog::error("{} Malformed transition definition: {}", lineNumber, line);
  throw Utils::ProgramTermination();
  /*return NOTHING;*/
}

std::string Parser::parseSignal(const std::string &line) const {
  std::string name;
  if (RE2::FullMatch(line, *input_pattern_, &name) && !name.empty()) {
    if (auto trimmed = Utils::Trim(name); !trimmed.empty()) {
      return trimmed;
    }
  }

  spdlog::error("[{}] Malformed signal definition: {}", lineNumber, line);
  throw Utils::ProgramTermination();
  /*return NOTHING;*/
}

}  // namespace ParserLib
