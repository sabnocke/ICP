#pragma once
#include <optional>
#include <string>
#include <tuple>

#include "AutomatLib.h"
#include "re2/re2.h"

namespace AutomatLib {
class Automat;
}

namespace Parser {
template <typename StringType>
struct TransitionRecord {
  static_assert(std::is_same_v<StringType, std::string> ||
                    std::is_same_v<StringType, std::string_view>,
                "TransitionRecord must be instantiated with either std::string "
                "or std::string_view");

  StringType from;
  StringType to;
  StringType input;
  StringType cond;
  StringType delay;

  static TransitionRecord Empty() { return {}; }
  [[nodiscard]] bool IsEmpty() const { return *this == Empty(); }
  bool operator==(const TransitionRecord &other) const {
    return from == other.from && to == other.to && input == other.input &&
           cond == other.cond && delay == other.delay;
  }
  bool operator!=(const TransitionRecord &other) const {
    return !(*this == other);
  }
};

template <typename StringType>
struct VariableRecord {
  static_assert(std::is_same_v<StringType, std::string> ||
                    std::is_same_v<StringType, std::string_view>,
                "VariableRecord must be instantiated with either std::string "
                "or std::string_view");

  StringType type;
  StringType name;
  StringType value;

  static VariableRecord Empty() { return {}; }
  [[nodiscard]] bool IsEmpty() const { return *this == Empty(); }
  bool operator==(const VariableRecord &other) const {
    return type == other.type && name == other.name && value == other.value;
  }
  bool operator!=(const VariableRecord &other) const {
    return !(*this == other);
  }
};

enum Section { Name, Comment, Variables, States, Transitions, Inputs, Outputs };

class parser {
 public:
  parser();
  std::optional<AutomatLib::Automat> parseAutomat(
      const std::string &file);  // TODO change it to optional
  std::optional<std::tuple<std::string, std::string>> parseState(
      const std::string &line) const;

 private:
  std::optional<VariableRecord<std::string>> parseVariable(
      const std::string &line) const;
  std::optional<TransitionRecord<std::string>> parseTransition(
      const std::string &line) const;
  std::optional<std::string> extractComment(const std::string &line) const;
  std::optional<std::string> extractName(const std::string &line);
  std::optional<std::string> parseSignal(const std::string &line) const;
  bool SectionHandler(const std::string &line);
  bool SectionHandler(std::string_view line);
  AutomatLib::Automat automat;

  Section ActualSection = Name;

  std::unique_ptr<RE2> name_pattern_;
  std::unique_ptr<RE2> comment_pattern_;
  std::unique_ptr<RE2> variables_pattern_;
  std::unique_ptr<RE2> states_pattern_;
  std::unique_ptr<RE2> transitions_pattern_;
  std::unique_ptr<RE2> input_pattern_;
  std::unique_ptr<RE2> output_pattern_;
};
}  // namespace Parser
