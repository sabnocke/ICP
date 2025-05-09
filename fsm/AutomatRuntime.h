#pragma once


#include "AutomatLib.h"
#include <absl/container/btree_set.h>
#include <absl/container/node_hash_map.h>

#include <iomanip>
#include <map>
#include <optional>
#include <ostream>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>


namespace AutomatLib {
class Automat;
struct Transition;
}  // namespace AutomatLib

namespace AutomatRuntime {
struct TransitionGroup;
using SignalsType = absl::node_hash_map<std::string, std::string>;
using TGT = absl::node_hash_map<std::string, TransitionGroup>;

template <typename T, typename Variant>
struct is_in_variant;

template <typename T, typename... Alternatives>
struct is_in_variant<T, std::variant<Alternatives...>>
    : std::disjunction<std::is_same<T, Alternatives>...> {};

template<typename T>
struct remove_cvref : std::remove_cv<std::remove_reference_t<T>> {};

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template <typename T, typename Variant>
constexpr bool is_in_variant_v = is_in_variant<T, Variant>::value;

struct Numeric {
 private:
  std::variant<std::monostate, int, long long, double> _numeric;

  using VariantType = decltype(_numeric);

 public:
  Numeric() = default;
  static Numeric Empty() { return Numeric{std::monostate{}}; }

  template <typename T, typename = std::enable_if_t<is_in_variant_v<
                            remove_cvref_t<T>, VariantType>>>
  explicit Numeric(T&& value) : _numeric(std::forward<T>(value)) {}
  //? same thing could be achieved by overloading constructor

  template <typename T, typename = std::enable_if_t<
                            is_in_variant_v<remove_cvref<T>, VariantType>>>
  Numeric& operator=(T&& value) {
    _numeric = std::forward<T>(value);
    return *this;
  }

  bool operator==(Numeric const& other) const {
    return _numeric == other._numeric;
  }
  bool operator==(const VariantType& variant) const {
    return _numeric == variant;
  }

  bool operator!=(Numeric const& other) const {
    return _numeric != other._numeric;
  }
  bool operator!=(const VariantType& variant) const {
    return _numeric != variant;
  }

  friend std::ostream& operator<<(std::ostream& os, const Numeric& numeric) {
    std::visit(
        [&](const auto& arg) {
          using T = std::decay_t<decltype(arg)>;

          if constexpr (std::is_same_v<T, std::monostate>) {
            os << "nothing";
          } else if constexpr (std::is_same_v<T, int> ||
                               std::is_same_v<T, long long>) {
            os << arg;
          } else if constexpr (std::is_same_v<T, double>) {
            os << std::fixed << std::setprecision(5) << arg
               << std::defaultfloat;
          } else {
            os << "unknown type";
          }
        },
        numeric._numeric);

    return os;
  }
};

struct TransitionGroup {
private:
  absl::btree_set<AutomatLib::Transition> _transitions;
  TGT _transition_groups;
public:
  TransitionGroup();
  size_t Count = 0;
  size_t Size() const { return _transitions.size(); }
  bool Some() const { return Size() != 0; }
  static TransitionGroup Empty() {
    auto tg = TransitionGroup{};
    tg._transition_groups = {};
    return tg;
  }
  // TransitionGroup() = default;
  void Add(const AutomatLib::Transition& transition);
  void Add(const std::string& from, const std::string& to,
           const std::string& input,
           const std::optional<std::function<bool()>>& condition,
           const std::string& delay);
  absl::btree_set<AutomatLib::Transition> Cost(int cost);
  size_t CostAtMost(int cost);
  static int GetCost(const AutomatLib::Transition& transition);
  TGT GroupTransitions();
  TransitionGroup Retrieve(const std::string& input);
  TransitionGroup Select(std::function<AutomatLib::Transition(AutomatLib::Transition)> pred);
  TransitionGroup Where(
      const std::function<bool(AutomatLib::Transition)>& pred);
  TransitionGroup WhereNoEvent();
  TransitionGroup WhereTimer();
  TransitionGroup WhereCond();

};

bool WaitFor(int delay);

Numeric StringToNumeric(std::string_view str);

class Runtime {
 public:
  std::string Name;
  explicit Runtime(AutomatLib::Automat& automat);
  [[deprecated]] void ExtractTransitions();

  template <typename T>
  static void Store(SignalsType& signals, std::string& name, const T& value);
  static std::string Load(SignalsType signals, const std::string& name);
  static bool RegisterSignal(SignalsType signals, const std::string& name);

  std::map<std::string, std::string> InMap = {};
  std::map<std::string, std::string> OutMap = {};

  void GroupTransitions();


 private:
  AutomatLib::Automat& Automat;
  std::vector<AutomatLib::Transition> transitions;
  std::unordered_map<std::string, AutomatLib::State> states;
  std::unordered_map<std::string, TransitionGroup> groupedTransitions;
};
}  // namespace AutomatRuntime
