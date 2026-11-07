#ifndef RESULTREADNUMBERS_HPP
#define RESULTREADNUMBERS_HPP

#include <array>
#include <charconv>
#include <cmath>
#include <cstddef>
#include <limits>
#include <string_view>
#include <system_error>

namespace ResultReadNumbers
{
constexpr std::size_t max_fields = 128;

inline bool whitespace(char value)
{
  return value == ' ' || value == '\t' || value == '\r'
      || value == '\n' || value == '\f' || value == '\v';
}

inline bool exponent(char value)
{
  return value == 'E' || value == 'e' || value == 'D' || value == 'd';
}

inline std::string_view trim(std::string_view text)
{
  while (!text.empty() && whitespace(text.front()))
  {
    text.remove_prefix(1);
  }
  while (!text.empty() && whitespace(text.back()))
  {
    text.remove_suffix(1);
  }
  return text;
}

inline bool integer(std::string_view text, int& value)
{
  text = trim(text);
  if (!text.empty() && text.front() == '+')
  {
    text.remove_prefix(1);
    if (!text.empty() && (text.front() == '-' || text.front() == '+'))
    {
      return false;
    }
  }
  if (text.empty())
  {
    return false;
  }
  const char* end = text.data() + text.size();
  const auto result = std::from_chars(text.data(), end, value);
  return result.ec == std::errc{} && result.ptr == end;
}

// Reader-only conversion: retain file precision instead of formatting and
// reparsing at the legacy helper's requested scientific precision.
inline bool parse_real(std::string_view text, double& value)
{
  text = trim(text);
  if (!text.empty() && text.front() == '+')
  {
    text.remove_prefix(1);
    if (!text.empty() && (text.front() == '-' || text.front() == '+'))
    {
      return false;
    }
  }
  if (text.empty())
  {
    return false;
  }
  // CCX/MSVC can write signed INF and NAN(IND). Handle these before D-exponent
  // normalization; the D in an NaN payload is not a Fortran exponent.
  auto special = text;
  const bool negative = special.front() == '-';
  if (negative)
  {
    special.remove_prefix(1);
  }
  const auto equal = [&](std::string_view name)
  {
    if (special.size() != name.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < name.size(); ++i)
    {
      const char c = special[i] >= 'a' && special[i] <= 'z' ? special[i] - 'a' + 'A' : special[i];
      if (c != name[i])
      {
        return false;
      }
    }
    return true;
  };
  if (equal("INF") || equal("INFINITY"))
  {
    value = negative ? -std::numeric_limits<double>::infinity() : std::numeric_limits<double>::infinity();
    return true;
  }
  if (equal("NAN") || equal("NAN(IND)"))
  {
    value = std::copysign(std::numeric_limits<double>::quiet_NaN(), negative ? -1.0 : 1.0);
    return true;
  }

  std::array<char, 128> normalized;
  const char* begin = text.data();
  if (text.find_first_of("Dd") != std::string_view::npos)
  {
    if (text.size() > normalized.size())
    {
      return false;
    }
    for (std::size_t i = 0; i < text.size(); ++i)
    {
      normalized[i] = text[i] == 'D' || text[i] == 'd' ? 'E' : text[i];
    }
    begin = normalized.data();
  }
  const char* end = begin + text.size();
  const auto result = std::from_chars(begin, end, value, std::chars_format::general);
  return result.ec == std::errc{} && result.ptr == end;
}

// Result payloads retain non-finite values; consumers mark their validity.
inline bool result_value(std::string_view text, double& value)
{
  return parse_real(text, value);
}

// Numerical metadata (coordinates/times/identities) must remain finite.
inline bool real(std::string_view text, double& value)
{
  return parse_real(text, value) && std::isfinite(value);
}

// Non-owning cursor with no fixed token count. Preserve adjacent signed fields
// without splitting exponent signs. The owner controls record/batch byte limits.
inline bool next_field(std::string_view line, std::size_t& position, std::string_view& token)
{
  while (position < line.size() && whitespace(line[position]))
  {
    ++position;
  }
  if (position == line.size())
  {
    token = {};
    return false;
  }
  const std::size_t begin = position++;
  while (position < line.size() && !whitespace(line[position]))
  {
    const char value = line[position];
    if ((value == '-' || value == '+') && !exponent(line[position - 1]))
    {
      break;
    }
    ++position;
  }
  token = line.substr(begin, position - begin);
  return true;
}

inline std::size_t field_count(std::string_view line, std::string_view* last = nullptr)
{
  std::size_t position = 0;
  std::size_t count = 0;
  std::string_view token;
  if (last)
  {
    *last = {};
  }
  while (next_field(line, position, token))
  {
    ++count;
    if (last)
    {
      *last = token;
    }
  }
  return count;
}

// Non-owning views stay valid only while the batch's text remains unchanged.
// Adjacent signed fields are separated without splitting exponent signs.
inline bool fields(std::string_view line, std::array<std::string_view, max_fields>& output, std::size_t& count)
{
  count = 0;
  std::size_t position = 0;
  std::string_view token;
  while (next_field(line, position, token))
  {
    if (count == output.size())
    {
      return false;
    }

    output[count++] = token;
  }
  return count != 0;
}

inline bool dat_record(std::string_view line)
{
  line = trim(line);
  std::size_t position = 0;
  while (position < line.size() && line[position] >= '0' && line[position] <= '9')
  {
    ++position;
  }
  if (position == 0)
  {
    return false;
  }
  return position == line.size() || whitespace(line[position])
      || line[position] == '-' || line[position] == '+';
}

inline bool frd_record(std::string_view line)
{
  line = trim(line);
  return line.size() > 2 && line[0] == '-' && line[1] == '1' && whitespace(line[2]);
}
}

#endif // RESULTREADNUMBERS_HPP
