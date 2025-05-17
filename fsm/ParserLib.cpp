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
//TODO change std::cerr to ProgramTermination

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
    throw Utils::ProgramTermination("Failed parsing regex patterns");
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
      if (const auto result = extractName(line); result.has_value()) {
        automat.Name = result.value();
        return true;
      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;
    case Comment:
      return true;
    case States: {
      if (const auto result = parseState(line); result.has_value()) {
        automat.addState(result.value());
        return true;
      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;
    }
    case Transitions: {
      if (const auto result = parseTransition(line); result.has_value()) {
        automat.addTransition(result.value());
        return true;
      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;
    }
    case Variables:
      if (const auto result = parseVariable(line); result.has_value()) {
        automat.addVariable(result.value());
        return true;
      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;
    case Inputs:
      if (const auto result = parseSignal(line); result.has_value()) {
        automat.addInput(result.value());
        return true;
      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;
    case Outputs:
      if (const auto result = parseSignal(line); result.has_value()) {
        automat.addOutput(result.value());
        return true;
      }
      spdlog::warn("Reached section '{}' but no name was extracted",
                   ActualSection);
      return false;
  }
  throw Utils::ProgramTermination(
      "Reached SectionHandler but no valid section was chosen.");
}

std::optional<State<std::string>> Parser::parseState(
    const std::string &line) const {
  const auto trimmed = Utils::Trim(line);
  std::string name, code;
  //! this works for single line state definition
  //TODO test how this works with [] being split on different lines

  // const auto res = Utils::FindAll(trimmed, '[', ']');
  // std::cout << trimmed << " " << std::boolalpha << res << std::endl;

  // if (!res) {
  //   // Failsafe in case of brackets being on different lines
  //   std::cerr << absl::StrFormat(
  //       "Missing opening/closing bracket for state definition\n", trimmed);
  //   return NOTHING;
  // }
  if (RE2::FullMatch(trimmed, *states_pattern_, &name, &code)) {
    return State<std::string>(Utils::Trim(name), Utils::Trim(code));
  }

  throw Utils::ProgramTermination(
      absl::StrFormat("Malformed state definition: %s", trimmed));
}

std::optional<std::string> Parser::extractName(const std::string &line) const {
  std::string name;

  if (RE2::FullMatch(line, *name_pattern_, &name) && !name.empty()) {
    if (auto trimmed = Utils::Trim(name); !trimmed.empty()) {
      return trimmed;
    }
  }
  // spdlog::error("", line);
  throw Utils::ProgramTermination("[%lu] Malformed name definition: %s",
                                  lineNumber, line);
}

[[deprecated("Comments are ignored")]]
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
}

std::optional<Variable> Parser::parseVariable(const std::string &line) const {
  const auto trimmed = Utils::Trim(line);
  std::string type, name, value;
  if (RE2::FullMatch(trimmed, *variables_pattern_, &type, &name, &value)) {
    return Variable{(std::move(type)), (std::move(name)), (std::move(value))};
  }
  // spdlog::error("Malformed variable definition: {}", trimmed);
  throw new Utils::ProgramTermination("[%lu] Malformed variable definition: %s",
                                      lineNumber, line);
  // return NOTHING;
}

[[deprecated]]
std::optional<VariableRecord> parseVariablePlain(const std::string &line) {
  // expects format type name = value;
  auto split = Utils::Split(line, '=');
  if (split.empty() || split.size() != 2) {
    return NOTHING;
  }
  split = Utils::TrimEach(split);
  // after this split = ["type name", "value]
  const auto value = split.at(1);
  split = Utils::Split(split.at(0), ' ');
  if (split.empty() || split.size() != 2) {
    return NOTHING;
  }
  split = Utils::TrimEach(split);
  auto res = VariableRecord{split.at(0), split.at(1), value};
  return res;
}

std::optional<Transition> Parser::parseTransition(
    const std::string &line) const {
  std::string from, to, input, cond, delay;

  if (RE2::FullMatch(line, *transitions_pattern_, &from, &to, &input, &cond,
                     &delay)) {
    const auto cond2 = Utils::Remove(cond, '[');
    const auto cond3 = Utils::Remove(cond2, ']');
    return Transition{from, to, input, Utils::Trim(cond3), delay};
  }

  throw Utils::ProgramTermination("[%lu] Malformed transition definition: %s",
                                  lineNumber, line);
  return NOTHING;
}

std::optional<std::string> Parser::parseSignal(const std::string &line) const {
  std::string name;
  if (RE2::FullMatch(line, *input_pattern_, &name) && !name.empty()) {
    if (auto trimmed = Utils::Trim(name); !trimmed.empty()) {
      return trimmed;
    }
  }

  throw Utils::ProgramTermination("[%lu] Malformed signal definition: %s",
                                  lineNumber, line);
  return NOTHING;
}

}  // namespace ParserLib
