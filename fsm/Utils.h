/**
 * @file   Utils.h
 * @brief  Pomocné funkce a šablony pro práci s textem a bezpečnou konverzi.
 * @author xhlochm00 Michal Hloch
 * @details
 * Obsahuje:
 *  - funkce pro trim, lowercase, split, remove, contains, quote
 *  - šablonu FindAll pro testování více podřetězců
 *  - detailní funkci AttemptConversion používající fast_float
 * @date   2025-05-11
 */
#pragma once

#include <absl/strings/str_format.h>

#include <iomanip>
#include <iostream>
#include <locale>
#include <optional>
#include <range/v3/all.hpp>
#include <string>
#include <vector>

#include "external/fast_float.h"

namespace Utils {

namespace detail {

template <typename... Ts>
struct Overloaded : Ts... {
  using Ts::operator()...;
};

template <typename... Ts> Overloaded(Ts...) -> Overloaded<Ts...>;

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

template <bool ExactComparison = true>
std::string RemovePrefix(const std::string &str, const std::string_view view,
                         const bool trimResult = false) {
  if constexpr (ExactComparison) {
    if (!ranges::starts_with(str, view))
      return trimResult ? Trim(str) : str;

    return trimResult ? Trim(str.substr(view.size())) : str.substr(view.size());
  } else {
    const auto a = ToLower(str);
    if (const auto b = ToLower(view); !ranges::starts_with(a, b))
      return trimResult ? Trim(str) : str;
    return trimResult ? Trim(str.substr(view.size())) : str.substr(view.size());
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
  if constexpr (!detail::IsNumeric<T>)
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

class ProgramTermination final : public std::exception {
 public:
  ProgramTermination() = default;
};

}  // namespace Utils
