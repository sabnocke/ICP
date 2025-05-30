#include "ParserLib.h"

#include <absl/log/absl_log.h>
#include <absl/strings/str_split.h>
#include <re2/re2.h>

#include <fstream>
#include <iostream>

#include "AutomatLib.h"
#include "Utils.h"

namespace ParserLib {

Parser::Parser() {
  RE2::Options options;
  options.set_case_sensitive(false);
  name_pattern_ =
      std::make_unique<RE2>(R"(^\s*Name\s*:?\s*(?<c>.*)$)", options);
  comment_pattern_ = std::make_unique<RE2>(R"(^.*?:\s*?(?<c>.*)$)", options);
  variables_pattern_ = std::make_unique<RE2>(
      R"(\s*(?<type>\w+)\s*(?<name>\w+)\s*=\s*(?<value>\w+)\s*)", options);
  states_pattern_ =
      std::make_unique<RE2>(R"(state (?<name>\w+) *\[(?<code>.*)\])", options);
  transitions_pattern_ = std::make_unique<RE2>(
      R"(^\s*(?<from>\w+)\s*-->\s*(?<to>\w+)\s*:\s*(?:(?<input>\w*)?\s*(?<cond>\[.*\])?\s*@?\s*(\w*)?)\s*$)",
      options);
  input_pattern_ = std::make_unique<RE2>(R"(^.*?:?\s*(?<name>\w*)$)", options);
  output_pattern_ = std::make_unique<RE2>(R"(^.*?:?\s*(?<name>\w*)$)", options);

  if (!name_pattern_->ok() || !comment_pattern_->ok() ||
      !variables_pattern_->ok() || !states_pattern_->ok() ||
      !transitions_pattern_->ok()) {
    ABSL_LOG(FATAL) << "Failed parsing regex patterns";
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
    if (Utils::Contains(line, '#')) {
      line = Utils::Split(line, '#')[0];
    }
    lineNumber++;
    if (line.empty())
      continue;

    if (Utils::Contains(line, "Name")) {
      ActualSection = Name;
      SectionHandler(line, automat);
      continue;
    }
    if (Utils::Contains(line, "comment")) {
      ActualSection = Comment;
      SectionHandler(line, automat);
      continue;
    }
    if (Utils::Contains(line, '#')) {
      // Comments are ignored
      continue;
    }
    if (Utils::Contains(line, "Variables")) {
      ActualSection = Variables;
      continue;
    }
    if (collecting || Utils::Contains(line, "State")) {
      if (absl::EqualsIgnoreCase(line, "states:"))
         continue;
      ActualSection = States;
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
  if (collecting) {
    LOG(ERROR) << "Automat is still collecting split definition of state";
    throw Utils::ProgramTermination();
  }
  return std::move(automat);
}

bool Parser::SectionHandler(const std::string &line,
                            AutomatLib::Automat &automat) const {
  switch (ActualSection) {
    case Name:
      automat.Name = extractName(line);
      return true;
    case Comment: {
      auto comment = Utils::RemovePrefix<false>(line, "comment:");
      auto trimmed = Utils::Trim(comment);
      if (trimmed.empty())
        return true;
      automat.Comment = trimmed;
      std::cerr << "Comment: " << automat.Comment << std::endl;
      return true;
    }
    case States: {
      if (const auto val = parseState(line); val.has_value()) {
        automat.addState(val.value());
      }
      // automat.addState(parseState(line));
      return true;
    }
    case Transitions: {
      automat.addTransition(parseTransition(line));
      /*std::cerr << "Current transitions: " << std::endl << automat.transitions << std::endl;*/
      return true;
    }
    case Variables:
      automat.addVariable(parseVariable(line));
      return true;
    case Inputs: {
      for (const auto &name : parseSignal<true>(line)) {
        // std::cerr << name << std::endl;
        automat.addInput(name);
      }
      // automat.addInput(parseSignal(line));
      return true;
    }

    case Outputs:
      for (const auto &name : parseSignal<false>(line)) {
        // std::cerr << name << std::endl;
        automat.addOutput(name);
      }
      // automat.addOutput(parseSignal(line));
      return true;
  }

  ABSL_LOG(ERROR) << "Reached SectionHandler but no valid section was chosen.";
  // throw Utils::ProgramTermination();
  return false;  //TODO or terminate?
}

std::optional<State<>> Parser::parseState(const std::string &line) const {

  auto trimmed = Utils::Trim(line);
  std::string name, code;
  const auto r1 = std::count(line.begin(), line.end(), '[');
  const auto r2 = std::count(line.begin(), line.end(), ']');
  bracketCounter += r1 - r2;
  /*std::cerr << "bracketCounter " << bracketCounter << std::endl;
  std::cerr << "line: " << line << std::endl;*/
  if (collecting && bracketCounter != 0) {
    /*std::cerr << "Split definition is still present" << std::endl;*/
    collector << line;
    return std::nullopt;
  }
  if (bracketCounter != 0) {
    /*std::cerr << "Detected split definition" << std::endl;*/
    collecting = true;
    collector << line;
    return std::nullopt;
  }
  if (collecting /*&& bracketCounter == 0*/) {
    collector << line;
    trimmed = Utils::Trim(collector.str());

    collector.str("");
    collector.clear();
    collecting = false;
  }

  if (RE2::FullMatch(trimmed, *states_pattern_, &name, &code)) {
    return State{Utils::Trim(name), Utils::Trim(code)};
  }

  ABSL_LOG(ERROR) << absl::StrFormat("[%lu] Malformed state definition: %s",
                                     lineNumber, line);
  throw Utils::ProgramTermination();
}

std::string Parser::extractName(const std::string &line) const {
  std::string name;

  if (RE2::FullMatch(line, *name_pattern_, &name) && !name.empty()) {
    if (auto trimmed = Utils::Trim(name); !trimmed.empty())
      return trimmed;
  }

  ABSL_LOG(ERROR) << absl::StrFormat("[%lu] Malformed name definition: %s",
                                     lineNumber, line);
  throw Utils::ProgramTermination();
}

Variable Parser::parseVariable(const std::string &line) const {
  const auto trimmed = Utils::Trim(line);
  std::string type, name, value;

  if (RE2::FullMatch(trimmed, *variables_pattern_, &type, &name, &value)) {
    /*std::cerr << "parseVariable returns " << type << ": " << name << " = " << value << std::endl;*/
    return Variable{std::move(type), std::move(name), std::move(value)};
  }

  ABSL_LOG(ERROR) << absl::StrFormat("[%lu] Malformed variable definition: %s",
                                     lineNumber, line);
  throw Utils::ProgramTermination();
}

Transition Parser::parseTransition(const std::string &line) const {
  std::string from, to, input, cond, delay;
  if (RE2::FullMatch(line, *transitions_pattern_, &from, &to, &input, &cond,
                     &delay)) {
    const auto cond2 = Utils::Remove(cond, '[');
    const auto cond3 = Utils::Remove(cond2, ']');
    auto t = Transition{from, to, input, std::move(Utils::Trim(cond3)), delay};
    return t;
  }

  ABSL_LOG(ERROR) << absl::StrFormat(
      "[%lu] Malformed transition definition: %s", lineNumber, line);
  throw Utils::ProgramTermination();
}

}  // namespace ParserLib
