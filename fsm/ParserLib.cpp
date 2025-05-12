#include "ParserLib.h"

#include <absl/log/absl_check.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include <re2/re2.h>

#include <fstream>
#include <iostream>

#include "AutomatLib.h"
#include "Utils.h"
#include "range/v3/view/transform.hpp"

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
    // LOG(FATAL)
    //     << "Failed to compile one or more essential parser regex patterns.";
    //TODO replace with something else
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
    std::cout << line << std::endl;                     //* <-- line printing <--
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
  //? return false in each case might be unnecessary as it cannot fall through multiple cases
  // std::cerr << "SectionHandler received: " << line << std::endl;
  // std::cerr << "Actual Section: " << ActualSection << std::endl;
  switch (ActualSection) {
    case Name:
      if (const auto result = extractName(line); result.has_value()) {
        automat.Name = result.value();
        return true;
      }
      return false;
    case Comment:
      return true;
    case States: {

      if (const auto result = parseState(line); result.has_value()) {
        automat.addState(result.value());
        return true;
      }
      return false;
    }
    case Transitions: {
      if (const auto result = parseTransition(line); result.has_value()) {
        automat.addTransition(result.value());
        return true;
      }
      return false;
    }
    case Variables:
      if (const auto result = parseVariable(line); result.has_value()) {
        automat.addVariable(result.value());
        return true;
      }
      return false;
    case Inputs:
      if (const auto result = parseSignal(line); result.has_value()) {
        automat.addInput(result.value());
        return true;
      }
      return false;
    case Outputs:
      if (const auto result = parseSignal(line); result.has_value()) {
        automat.addOutput(result.value());
        return true;
      }
      return false;
  }
  return false;
}

std::optional<std::tuple<std::string, std::string>> Parser::parseState(
    const std::string &line) const {
  // std::cerr << "Received line: " << line << std::endl;
  const auto trimmed = Utils::Trim(line);
  std::string name, code;
  // const auto res = Utils::FindAll(trimmed, '[', ']');
  // std::cout << trimmed << " " << std::boolalpha << res << std::endl;

  // if (!res) {
  //   // Failsafe in case of brackets being on different lines
  //   std::cerr << absl::StrFormat(
  //       "Missing opening/closing bracket for state definition\n", trimmed);
  //   return NOTHING;
  // }
  if (!RE2::FullMatch(trimmed, *states_pattern_, &name, &code)) {
    // LOG(WARNING) << "Didn't find match" << std::endl;
    //TODO replace with something else
    return NOTHING;
  }
  return std::make_tuple(Utils::Trim(name), Utils::Trim(code));
}

std::optional<std::string> Parser::extractName(const std::string &line) const {
  std::string name;

  if (RE2::FullMatch(line, *name_pattern_, &name) && !name.empty()) {
    if (auto trimmed = Utils::Trim(name); !trimmed.empty()) {
      return trimmed;
    }
  }
  if (auto trimmed = Utils::Trim(line); !trimmed.empty()) {
    return trimmed;
  }
  return NOTHING;
}

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

std::optional<Variable> Parser::parseVariable(
    const std::string &line) const {
  const auto trimmed = Utils::Trim(line);
  std::string type, name, value;
  if (RE2::FullMatch(trimmed, *variables_pattern_, &type, &name, &value)) {
    return Variable{type, name, value};
  }
  return NOTHING;
}

[[deprecated]] std::optional<VariableRecord> parseVariablePlain(
    const std::string &line) {
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
    auto cond2 = Utils::Remove(cond, '[');
    auto cond3 = Utils::Remove(cond2, ']');
    return Transition{from, to, input, Utils::Trim(cond3), delay};
  }

  return NOTHING;
}

std::optional<std::string> Parser::parseSignal(const std::string &line) const {
  std::string name;
  if (RE2::FullMatch(line, *input_pattern_, &name) && !name.empty()) {
    if (auto trimmed = Utils::Trim(name); !trimmed.empty()) {
      return trimmed;
    }
  }
  return NOTHING;
}

}  // namespace ParserLib
