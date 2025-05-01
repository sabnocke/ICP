#pragma once
#include <string>
#include <optional>
#include <concepts>

template<typename T>
concept Streamable = requires(std::ostream &os, const T &t)
{
  {os << t} -> std::convertible_to<std::ostream &>;
};

template<typename T>
concept IsStdString = std::same_as<std::remove_cvref_t<T>, std::string>;

template<typename T>
concept IsCString = std::is_same_v<const char *, std::decay_t<T>> || std::is_same_v<char *, std::decay_t<T>>;

template<typename T>
concept IsStringView = std::same_as<std::remove_cvref_t<T>, std::string_view>;

class Utils {
public:
  static std::string Trim(const std::string &str);
  static std::string TrimLeft(const std::string &str);
  static std::string TrimRight(const std::string &str);
  static std::string Purify(const std::string &str);
  static std::string Remove(const std::string &str, char c);
  static std::string Remove(const std::string &str, const std::string &substr);
  template<typename T>
  std::optional<std::string> Utils::ToStringOpt(const T value);
private:
  const std::string whitespacePattern = " \f\n\r\t\v";
};

