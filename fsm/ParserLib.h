#pragma once
#include <re2/re2.h>

#include <optional>
#include <string>
#include <tuple>

#include "AutomatLib.h"

namespace AutomatLib {
class Automat;
}

namespace ParserLib {
struct TransitionRecord {
  std::string from;
  std::string to;
  std::string input;
  std::string cond;
  std::string delay;

  static TransitionRecord Empty() { return {}; }
  [[nodiscard]] bool IsEmpty() const { return *this == Empty(); }
  bool operator==(const TransitionRecord &other) const {
    return from == other.from && to == other.to && input == other.input &&
           cond == other.cond && delay == other.delay;
  }
  bool operator!=(const TransitionRecord &other) const {
    return !(*this == other);
  }

  friend std::ostream &operator<<(std::ostream &os,
                                  const TransitionRecord &tr) {
    os << absl::StrFormat("%s -> %s : on <%s> if <%s> after <%s>", tr.from, tr.to,
                          tr.input, tr.cond, tr.delay);
    return os;
  }
};

struct VariableRecord {
  std::string type;
  std::string name;
  std::string value;

  static VariableRecord Empty() { return {}; }
  [[nodiscard]] bool IsEmpty() const { return *this == Empty(); }
  bool operator==(const VariableRecord &other) const {
    return type == other.type && name == other.name && value == other.value;
  }
  bool operator!=(const VariableRecord &other) const {
    return !(*this == other);
  }

  friend std::ostream &operator<<(std::ostream &os, const VariableRecord &record) {
    os << absl::StrFormat("%s: %s := %s", record.name, record.type, record.value);
    return os;
  }
};

enum Section { Name, Comment, Variables, States, Transitions, Inputs, Outputs };

class Parser {
 public:
  Parser();
  AutomatLib::Automat parseAutomat(
      const std::string &file);  // TODO change it to optional
  [[nodiscard]] std::optional<std::tuple<std::string, std::string>> parseState(
      const std::string &line) const;
  [[nodiscard]] std::optional<VariableRecord> parseVariable(
      const std::string &line) const;
  [[nodiscard]] std::optional<TransitionRecord> parseTransition(
      const std::string &line) const;
  [[nodiscard]] std::optional<std::string> extractComment(
      const std::string &line) const;
  [[nodiscard]] std::optional<std::string> extractName(
      const std::string &line) const;
  [[nodiscard]] std::optional<std::string> parseSignal(
      const std::string &line) const;

 private:
  bool SectionHandler(const std::string &line,
                      AutomatLib::Automat &automat) const;

  Section ActualSection = Name;

  std::unique_ptr<RE2> name_pattern_{};
  std::unique_ptr<RE2> comment_pattern_{};
  std::unique_ptr<RE2> variables_pattern_{};
  std::unique_ptr<RE2> states_pattern_{};
  std::unique_ptr<RE2> transitions_pattern_{};
  std::unique_ptr<RE2> input_pattern_{};
  std::unique_ptr<RE2> output_pattern_{};
};
}  // namespace ParserLib
