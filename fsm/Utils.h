/**
 * @file   Utils.h
 * @brief  Pomocné funkce a šablony pro práci s textem a bezpečnou konverzi.
 * @author xhlochm00 Michal Hloch
 * @details
 * Obsahuje:
 *  - šablony pro detekci typů (string, string_view, C-string, streamovatelné typy)
 *  - funkce pro trim, lowercase, split, remove, contains, quote
 *  - šablonu StringToNumeric pro převod na Numeric
 *  - šablonu FindAll pro testování více podřetězců
 *  - šablony pro konverzi libovolného typu na std::string
 *  - detailní funkci AttemptConversion používající fast_float
 * @date   2025-05-11
 */
#pragma once

#include <absl/strings/str_format.h>

#include <iostream>
#include <locale>
#include <optional>
#include <range/v3/all.hpp>
#include <string>
#include <vector>

#include "external/fast_float.h"
#include "types/numeric.h"

namespace Utils {
using namespace types;

namespace detail {
/**
 * @brief Trait kontrolující, zda je T typu std::string.
 */
template <typename T>
[[deprecated("ToString conversion is unused")]] struct is_std_string_impl
    : std::is_same<std::string, std::decay_t<T>> {};
template <typename T>
[[deprecated("ToString conversion is unused")]]
inline constexpr bool is_std_string_v = is_std_string_impl<T>::value;

/**
 * @brief Trait kontrolující, zda je T typu std::string_view.
 */
template <typename T>
[[deprecated("ToString conversion is unused")]] struct is_std_string_view_impl
    : std::is_same<std::string_view, std::decay_t<T>> {};
template <typename T>
[[deprecated("ToString conversion is unused")]]
inline constexpr bool is_std_string_view_v = is_std_string_view_impl<T>::value;

/**
 * @brief Trait kontrolující, zda je T C-string (char*, char[]).
 */
template <typename T>
[[deprecated("ToString conversion is unused")]] struct is_c_string_condition {
  static constexpr bool value =
      std::is_same_v<
          char, std::remove_cv_t<std::remove_pointer_t<std::decay_t<T>>>> ||
      (std::is_array_v<std::remove_reference_t<T>> &&
       std::is_same_v<
           char,
           std::remove_cv_t<std::remove_extent_t<std::remove_reference_t<T>>>>);
};
template <typename T, bool V = is_c_string_condition<T>::value>
[[deprecated("ToString conversion is unused")]]
struct is_c_string_impl : std::false_type {};
template <typename T>
[[deprecated("ToString conversion is unused")]] struct is_c_string_impl<T, true>
    : std::true_type {};
template <typename T>
[[deprecated("ToString conversion is unused")]]
inline constexpr bool is_c_string_v = is_c_string_impl<T>::value;

/**
 * @brief Trait kontrolující, zda lze typ T streamovat do std::ostream.
 */
template <typename T, typename = void>
[[deprecated("ToString conversion is unused")]] struct is_streamable_impl
    : std::false_type {};
template <typename T>
[[deprecated("ToString conversion is unused")]] struct is_streamable_impl<
    T, std::void_t<decltype(std::declval<std::ostream &>()
                            << std::declval<const T &>())>> : std::true_type {};
template <typename T>
[[deprecated("ToString conversion is unused")]]
inline constexpr bool is_streamable_v = is_streamable_impl<T>::value;

template <typename T>
inline constexpr bool IsNumeric =
    std::disjunction_v<std::is_integral<T>, std::is_floating_point<T>>;

/**
 * @brief Pomocná metafunkce pro odstranění cv-qualifiers a referencí.
 */
template <typename T>
struct remove_cvref : std::remove_cv<std::remove_reference_t<T>> {};

template <typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

}  // namespace detail

enum StringComparison { Lazy, Exact };

namespace internal {
/**
 * @brief Pokusí se převést string_view na hodnotu typu T pomocí fast_float.
 * @tparam T Numerický typ (integral nebo floating).
 * @param str Vstupní řetězec.
 * @return Optional s výsledkem nebo nullopt.
 */
template <typename T>
std::optional<T> AttemptConversion(const std::string_view str) {
  const char *first = str.data();
  const char *last = first + str.size();
  T value{};
  auto [ptr, ec] = fast_float::from_chars(first, last, value);

  if (ec == std::errc{} && ptr == last)
    return value;
  if (ec == std::errc::result_out_of_range) {
    if (*first == '-') {
      std::cerr << "Clamped to MIN" << std::endl;
      return std::numeric_limits<T>::min();
    }
    std::cerr << "Clamped to MAX" << std::endl;
    return std::numeric_limits<T>::max();
  }

  return std::nullopt;
}
}  // namespace internal

/**
 * @brief Test, zda znak není bílý znak.
 * @param ch Vstupní znak.
 * @return true pokud není whitespace.
 */
static bool IsNotWhitespace(const unsigned char ch) {
  return !std::isspace(ch);
}

/**
 * @brief Test, zda znak je bílý znak.
 * @param ch Vstupní znak.
 * @return true pokud je whitespace.
 */
static bool IsWhiteSpace(const unsigned char ch) { return std::isspace(ch); }

/**
 * @brief Odebere všechny výskyty znaku c z řetězce.
 */
std::string Remove(const std::string &str, char c);

/**
 * @brief Odebere všechny výskyty podřetězce substr z řetězce.
 */
std::string Remove(const std::string &str, const std::string &substr);

/**
 * @brief Převede řetězec na malá písmena.
 */
std::string ToLower(const std::string &str);
std::string ToLower(std::string_view str);
char ToLower(char c);

/**
 * @brief Ořízne bílé znaky na okrajích řetězce.
 */
std::string Trim(const std::string &str);
std::string_view Trim(std::string_view str);

/**
 * @brief Test, zda str obsahuje podřetězec view (case-insensitive).
 */
bool Contains(const std::string &str, std::string_view view);
inline bool Contains(const std::string &str, const char c) {
  return ToLower(str).find(c) != std::string::npos;
}

/**
 * @brief Rozdělí řetězec podle char delimiteru.
 */
inline std::vector<std::string> Split(const std::string &str,
                                      const char delim) {
  auto res =
      str | ranges::views::split(delim) | ranges::to<std::vector<std::string>>;
  return res;
}

/**
 * @brief Rozdělí řetězec podle string delimiteru.
 */
inline std::vector<std::string> Split(const std::string &str,
                                      const std::string &delim) {
  return str | ranges::views::split(delim) |
         ranges::to<std::vector<std::string>>;
}

inline std::string RemovePrefix(const std::string &str,
                                const std::string_view view,
                                const StringComparison cmp = Exact) {
  switch (cmp) {
    case Lazy:
      const auto a = ToLower(str);
      if (const auto b = ToLower(view); !ranges::starts_with(a, b))
        return str;
      return str.substr(view.size());
    case Exact:
      if (!ranges::starts_with(str, view))
        return str;

      return str.substr(view.size());
    default:
      return str;
  }
}

inline std::string RemoveSuffix(const std::string &str,
                                const std::string_view view) {
  if (!ranges::ends_with(str, view))
    return str;

  return str.substr(0, str.size() - view.size());
}

/**
 * @brief Ořízne každý prvek vektoru řetězců.
 */
std::vector<std::string> TrimEach(std::vector<std::string> &vec);

/**
 * @brief Přidá uvozovky okolo řetězce.
 */
inline std::string Quote(const std::string &str) {
  std::stringstream ss;
  ss << std::quoted(str);
  return ss.str();
}

/**
 * @brief Převod string_view na Numeric.
 * @tparam T Numerický typ.
 */
template <typename T>
std::optional<T> StringToNumeric(const std::string_view str) {
  if constexpr (!std::disjunction_v<std::is_integral<T>,
                                    std::is_floating_point<T>>)
    return std::nullopt;
  if (auto v = internal::AttemptConversion<T>(str); v.has_value())
    return v.value();
  return std::nullopt;
}

/**
 * @brief Test, zda str obsahuje všechny args (variadic).
 */
template <typename... Args>
bool FindAll(const std::string_view str, Args... args) {
  return ((str.find(args) != std::string::npos) && ...);
}

/**
 * @brief Pokusí se převést arbitrátní streamovatelný typ na std::string.
 * @tparam T Libovolný typ.
 */
template <typename T>
[[deprecated("ToString conversion is unused")]]
std::optional<std::string> ToStringOpt(const T &value) {
  using Decayed = std::decay_t<T>;
  if constexpr (detail::is_std_string_v<Decayed>) {
    return value;
  }
  if constexpr (detail::is_std_string_view_v<Decayed>) {
    return std::string(value);
  }
  if constexpr (detail::is_c_string_v<Decayed>) {
    if constexpr (std::is_pointer_v<Decayed>)
      return value ? std::string(value) : std::string();
    else
      return std::string(value);
  }
  if constexpr (std::is_same_v<Decayed, bool>) {
    std::ostringstream os;
    os << std::boolalpha << value;
    return os.str();
  }
  if constexpr (detail::is_streamable_v<Decayed>) {
    std::ostringstream os;
    os << value;
    return os.str();
  }
  return std::nullopt;
}

/**
 * @brief Formátování hodnoty pomocí absl::StrFormat.
 */
template <typename T>
[[deprecated("ToString conversion is unused")]]
std::optional<std::string> FormatToString(const T &value) {
  using Decayed = std::decay_t<T>;
  if constexpr (detail::is_std_string_v<Decayed> ||
                detail::is_std_string_view_v<Decayed>) {
    return absl::StrFormat("%s", value);
  }
  if constexpr (detail::is_c_string_v<Decayed>) {
    if constexpr (std::is_pointer_v<Decayed> && value == nullptr)
      return std::string();
    else
      return std::string(value);
  }
  if constexpr (std::is_arithmetic_v<Decayed> ||
                std::is_same_v<Decayed, bool>) {
    return absl::StrFormat("%v", value);
  }
  return std::nullopt;
}

class ProgramTermination final : public std::exception {
  std::string message;


 public:
  size_t lineNumber = -1;
  explicit ProgramTermination(std::string &&msg) : message(std::move(msg)) {}
  template <typename... Args>
  explicit ProgramTermination(std::string_view fmt, Args... args) {
    message = absl::StrFormat(fmt, args...);
  }
  ProgramTermination &Line(size_t lineN) {
    lineNumber = lineN;
    return *this;
  }
  const char *what() const noexcept override { return message.c_str(); }
};

}  // namespace Utils
