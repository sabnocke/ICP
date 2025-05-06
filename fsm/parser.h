#pragma once
#include <optional>
#include <string>
#include <tuple>

#include "AutomatLib.h"

namespace AutomatLib {
  class Automat;
}

namespace Parser {

  struct ParseTransitionRecord {
    std::string from;
    std::string to;
    std::string input;
    std::string cond;
    std::string delay;
    static ParseTransitionRecord Empty() {return {"", "", "", "", ""};}
    [[nodiscard]] bool IsEmpty() const {return *this == Empty();}
    bool operator==(const ParseTransitionRecord &other) const {
      return from == other.from && to == other.to && input == other.input && cond == other.cond && delay == other.delay;
    }
    bool operator!=(const ParseTransitionRecord &other) const {
      return !(*this == other);
    }
  };



  struct ParseVariableRecord {
    std::string type;
    std::string name;
    std::string value;
    static ParseVariableRecord Empty() {return {"", "", ""};}
    [[nodiscard]] bool IsEmpty() const { return *this == Empty();}
    bool operator==(const ParseVariableRecord &other) const {
      return type == other.type && name == other.name && value == other.value;
    }
    bool operator!=(const ParseVariableRecord &other) const {
      return !(*this == other);
    }
  };

  class parser {
  public:
    AutomatLib::Automat parseAutomat(const std::string &file); // TODO change it to optional
    static std::optional<ParseTransitionRecord> parseTransition(const std::string &line);

  private:
    static std::optional<ParseVariableRecord> parseVariable(const std::string &line);
    static std::optional<std::tuple<std::string, std::string>> parseState(const std::string &line);

    static std::optional<std::string> extractName(const std::string &line);
    bool SectionHandler(const std::string &line);
    AutomatLib::Automat automat;
    enum Section { Name, Comment, Variables, States, Transitions };
    Section ActualSection = Name;
  };
}
