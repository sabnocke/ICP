//
// Created by ReWyn on 06.05.2025.
//

#include "AutomatRuntime.h"

#include <chrono>
#include <thread>
#include "Utils.h"
#include "fast_float/fast_float.h"

#include "AutomatLib.h"
namespace AutomatRuntime {
#define NOTHING std::nullopt

Runtime::Runtime(AutomatLib::Automat &automat) : Automat(automat) {
  Name = automat.Name;
}

void Runtime::ExtractTransitions() {}

bool WaitFor(const int delay) {
  std::this_thread::sleep_for(std::chrono::milliseconds(delay));
  return true;
}

template <typename T>
void Runtime::Output(std::string_view name, const T& value) {}

Numeric StringToNumeric(const std::string_view str) {
  const auto input = std::string(str);
  if (const auto result = AttemptIntegerConversion(input); result.has_value()) {
    return Numeric(result.value());
  }
  if (const auto result = AttemptDoubleConversion(input); result.has_value()) {
    return Numeric(result.value());
  }
  return Numeric::Empty();
}


std::optional<long long> AttemptIntegerConversion(const std::string& input) {
  const auto first = input.data();
  const auto last = input.data() + input.size();
  long long value;

  if (auto [ptr, ec] = fast_float::from_chars(first, last, value);
      ec != std::errc() || ptr != last) {
    return NOTHING;
  }

  return value;
}
std::optional<double> AttemptDoubleConversion(const std::string& input) {
  const auto first = input.data();
  const auto last = input.data() + input.size();
  double value;

  if (auto [ptr, ec] = fast_float::from_chars(first, last, value);
      ec != std::errc{} || ptr != last) {
    return NOTHING;
  }

  return value;
}

}  // namespace AutomatRuntime