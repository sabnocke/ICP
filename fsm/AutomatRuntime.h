#pragma once


#include "AutomatLib.h"
#include <absl/container/node_hash_map.h>

#include <iomanip>
#include <map>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

#include "types/all_types.h"

namespace AutomatLib {
class Automat;
}  // namespace AutomatLib

namespace types {
struct Transition;
struct TransitionGroup;
}

namespace AutomatRuntime {
using namespace types;
using SignalsType = absl::node_hash_map<std::string, std::string>;


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
  std::vector<Transition> transitions;
  StateGroup states;
  std::unordered_map<std::string, TransitionGroup> groupedTransitions;
};
}  // namespace AutomatRuntime
