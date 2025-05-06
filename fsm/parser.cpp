#include "parser.h"
#include <absl/log/absl_check.h>
#include <absl/strings/str_format.h>
#include <absl/strings/str_split.h>
#include "absl/log/log.h"
#include <fstream>
#include <re2/re2.h>
#include <iostream>
#include "AutomatLib.h"
#include "Utils.h"

#define DEFAULT_NAME "Unknown"
#define NOTHING std::nullopt

namespace Parser {
  AutomatLib::Automat parser::parseAutomat(const std::string &file) {
    std::ifstream ifs(file);
    if (!ifs) {
      std::cerr << "Can't open file " << file << std::endl;
      return automat;
    }
    std::string line;

    // TODO what if there isn't linefeed between section name and its content
    // TODO leave comments alone + allow multiple of them
    while (std::getline(ifs, line)) {
      if (absl::StrContains(line, "Name")) {
        ActualSection = Name;
      } else if (absl::StrContains(line, "Comment")) {
        ActualSection = Comment;
      } else if (absl::StrContains(line, "Variables")) {
        ActualSection = Variables;
      } else if (absl::StrContains(line, "States")) {
        ActualSection = States;
      } else if (absl::StrContains(line, "Transitions")) {
        ActualSection = Transitions;
      } else {
        SectionHandler(line);
      }
    }
    return automat;
  }

  bool parser::SectionHandler(const std::string &line) {
    switch (ActualSection) {
      case Name:
        automat.Name = line; break;
      case Comment:
        automat.Comment = line; break;
      case States: {
        const auto result = parseState(line);
        if (!result.has_value())
          return false;
        automat.addState(*result);
        break;
      }
      case Transitions: {
        const auto result = parseTransition(line);
        if (!result.has_value()) return false;
        automat.addTransition(*result);
        break;
      }
      case Variables: {
        const auto result = parseVariable(line);
        if (!result.has_value()) return false;
        automat.addVariable(*result);
        break;
      }
    }
    return true;
  }

  std::optional<std::tuple<std::string, std::string>> parser::parseState(const std::string &line) {
    const auto trimmed = Utils::Trim(line);
    std::string name, code;
    if (!RE2::FullMatch(trimmed, R"({.*})")) { // Failsafe in case of brackets being on different lines
      LOG(WARNING) << absl::StrFormat("Expected regular expression '{.*}' to find match with '%s'", trimmed)
                   << std::endl;
      return NOTHING;
    }
    if (!RE2::FullMatch(trimmed, R"((?<name>\w+) *: *\{(?<code>.+)\})", &name, &code)) {
      return NOTHING;
    }
    return std::make_tuple(name, code);
    // TODO can the code be on multiple lines?
    // TODO if so, then it cannot be detected via regex this easily, would need more parsing and accumulation
  }

  /// Extracts name from line, expecting format of "Name:<name>".
  /// The string should be purified prior to calling this function
  /// @param line Line from which name will be extracted
  /// @return String containing name, otherwise nothing
  std::optional<std::string> parser::extractName(const std::string &line) {
    const std::string pureLine = Utils::Purify(line);  // Purify might be unnecessary as it may remove important formatting
    if (const auto colon = pureLine.find(':'); colon != std::string::npos) {
      return line.substr(colon + 1);
    }
    return NOTHING;
  }

  std::optional<ParseVariableRecord> parser::parseVariable(const std::string &line) {
    const auto trimmed = Utils::Trim(line);
    std::string type, name, value;
    if (RE2::FullMatch(trimmed, R"((?<type>\w+) (?<name>\w+) = (?<value>\w+))", &type, &name, &value)) {
      return ParseVariableRecord{type, name, value};
    }
    return NOTHING;
  }

  //? Is it better than having one regex for the whole thing
  std::optional<ParseTransitionRecord> parser::parseTransition(const std::string &line) {
    std::string from, to, input, cond, delay;

    const std::vector<std::string> result = absl::StrSplit(line, ':');
    auto r1 = Utils::Trim(result[0]);

    ABSL_CHECK(RE2::FullMatch(Utils::Trim(result[0]), R"((?<from>\w+) *--> *(?<to>\w+))", &from, &to));
    std::cout << "from: " << from << " to: " << to << std::endl;
    if (!RE2::FullMatch(Utils::Trim(result[0]), R"((?<from>\w+) *--> *(?<to>\w+))", &from, &to)) {
      return NOTHING;
    }
    // General format: input_event[condition]@delay
    if (!RE2::FullMatch(Utils::Trim(result[1]), R"(^\s*(?<input>\w*)\s*(?:\[\s*(?<cond>.*?)\s*\])?\s*(?:@\s*(?<delay>\w+))?\s*$)", &input, &cond,
                        &delay)) {
      return NOTHING;
    }

    return ParseTransitionRecord{from, to, input, cond, delay};
  }
} // namespace Parser
