#pragma once
#include <concepts>
#include <functional>
#include <locale>
#include <optional>
#include <string>

#include "absl/strings/str_format.h"
#include "absl/synchronization/internal/waiter_base.h"

namespace Utils {
  // TODO concepts are C++20 things, so remake

  namespace detail { // Using a detail namespace for traits

    // Trait to check if T is std::string or const std::string& etc.
    template <typename T>
    struct is_std_string_impl : std::is_same<std::string, std::decay_t<T>> {};
    template <typename T>
    inline constexpr bool is_std_string_v = is_std_string_impl<T>::value;

    // Trait to check if T is std::string_view or const std::string_view& etc.
    template <typename T>
    struct is_std_string_view_impl : std::is_same<std::string_view, std::decay_t<T>> {};
    template <typename T>
    inline constexpr bool is_std_string_view_v = is_std_string_view_impl<T>::value;

    // Trait to check if T is a C-style string (char*, const char*, char[], const char[])
    // Using the revised version from previous discussion
    template <typename T>
    struct is_c_string_condition {
      static constexpr bool value =
          std::is_same_v<char, std::remove_cv_t<std::remove_pointer_t<std::decay_t<T>>>> || // char*, const char*
          (std::is_array_v<std::remove_reference_t<T>> && std::is_same_v<char, std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>> >>); // char[], const char[]
    };
    template <typename T, bool ConditionValue = is_c_string_condition<T>::value>
    struct is_c_string_impl : std::false_type {};
    template <typename T>
    struct is_c_string_impl<T, true> : std::true_type {};
    template <typename T>
    inline constexpr bool is_c_string_v = is_c_string_impl<T>::value;


    // Trait to check if T can be streamed to std::ostream (SFINAE based for C++17)
    template <typename T, typename = void>
    struct is_streamable_impl : std::false_type {};

    template <typename T>
    struct is_streamable_impl<T, std::void_t<decltype(std::declval<std::ostream&>() << std::declval<const T&>())>>
        : std::true_type {};
    template <typename T>
    inline constexpr bool is_streamable_v = is_streamable_impl<T>::value;

  } // namespace detail

  static bool IsNotWhitespace(const unsigned char ch) {return !std::isspace(ch);}
  static bool IsWhiteSpace(const unsigned char ch) {return std::isspace(ch);}

  static std::string Purify(const std::string &str);
  static std::string Remove(const std::string &str, char c);
  static std::string Remove(const std::string &str, const std::string &substr);

  std::string Trim(const std::string &str);

  template<typename T>
    static std::optional<std::string> ToStringOpt(const T& value) {
    using DecayedT = std::decay_t<T>;
    if constexpr (detail::is_std_string_v<DecayedT>) {
      return value;
    }
    if constexpr (detail::is_std_string_view_v<DecayedT>) {
      return std::string(value);
    }
    if constexpr (detail::is_c_string_v<DecayedT>) {
      if constexpr (std::is_pointer_v<DecayedT>) {
        return (value == nullptr) ? std::string("") : std::string(value);
      } else return std::string(value);
    }
    if constexpr (std::is_same_v<DecayedT, bool>) {
      std::ostringstream os;
      os << std::boolalpha << value;
      return os.str();
    }
    if constexpr (detail::is_streamable_v<DecayedT>) {
      std::ostringstream os;
      os << value;
      return os.str();
    }
    return std::nullopt;
  }

  template<typename T>
  static std::optional<std::string> FormatToString(const T& value) {
    using DecayedT = std::decay_t<T>;

    if constexpr (detail::is_std_string_v<DecayedT> || detail::is_std_string_view_v<DecayedT>) {
      return absl::StrFormat("%s", value);
    }
    if constexpr (detail::is_c_string_v<DecayedT>) {
      if constexpr (std::is_pointer_v<DecayedT> && value == nullptr) return std::string("");
      else return std::string(value);
    }
    if constexpr (std::is_arithmetic_v<DecayedT> || std::is_same_v<DecayedT, bool>) {
      return absl::StrFormat("%v", value);
    }
    return std::nullopt;
  }
} // namespace Utils
