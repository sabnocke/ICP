#pragma once
#include <iomanip>
#include <optional>
#include <ostream>
#include <string>
#include <variant>
#include <vector>

namespace AutomatLib {
class Automat;
struct Transition;
}  // namespace AutomatLib

namespace AutomatRuntime {

template <typename T, typename Variant>
struct is_in_variant;

template <typename T, typename... Alternatives>
struct is_in_variant<T, std::variant<Alternatives...>>
    : std::disjunction<std::is_same<T, Alternatives>...> {};

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
                            std::remove_cvref_t<T>, VariantType>>>
  explicit Numeric(T&& value) : _numeric(std::forward<T>(value)) {}

  template <typename T, typename = std::enable_if_t<
                            is_in_variant_v<std::remove_cvref<T>, VariantType>>>
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

  std::ostream& operator<<(std::ostream& os, const Numeric& numeric) const {
    std::visit(
        [&]<typename T0>(const T0& arg) {
      using T = std::decay_t<T0>;

      if constexpr (std::is_same_v<T, std::monostate>) {
        os << "nothing";
      } else if constexpr (std::is_same_v<T, int> || std::is_same_v<T, long long>) {
        os << arg;
      } else if constexpr (std::is_same_v<T, double>) {
        os << std::fixed << std::setprecision(5) << arg << std::defaultfloat;
      } else {
        os << "unknown type";
      }
    }, numeric._numeric);

    return os;
  }
};



bool WaitFor(int delay);
Numeric StringToNumeric(std::string_view str);

std::optional<long long> AttemptIntegerConversion(const std::string& input);
std::optional<double> AttemptDoubleConversion(const std::string& input);

class Runtime {
 public:
  std::string Name;
  explicit Runtime(AutomatLib::Automat& automat);
  [[deprecated]] void ExtractTransitions();

  template <typename T>
  static void Output(std::string_view name, const T& value);

 private:
  AutomatLib::Automat& Automat;
  std::vector<AutomatLib::Transition> transitions;
};
}  // namespace AutomatRuntime
