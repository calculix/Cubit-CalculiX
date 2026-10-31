#ifndef SCIENTIFICNUMBER_HPP
#define SCIENTIFICNUMBER_HPP

#include <locale>
#include <sstream>
#include <stdexcept>
#include <string>

namespace ScientificNumber
{
inline double parse(const std::string& text)
{
  std::istringstream stream(text);
  stream.imbue(std::locale::classic());
  double value;
  if (!(stream >> value))
  {
    throw std::invalid_argument("Invalid scientific number: " + text);
  }
  stream >> std::ws;
  if (!stream.eof())
  {
    throw std::invalid_argument("Invalid scientific number: " + text);
  }
  return value;
}

inline std::string format(double value, int precision)
{
  std::ostringstream stream;
  stream.imbue(std::locale::classic());
  stream.precision(precision);
  stream << std::scientific << value;
  return stream.str();
}

inline double parse_with_precision(std::string text, int precision = 6)
{
  bool negative = !text.empty() && text[0] == '-';

  if (negative)
  {
    text.erase(0, 1);
  }

  double value = text == "INF" ? 1.255070e+29 : parse(text);

  if (negative)
  {
    value = -value;
  }

  return parse(format(value, precision));
}
}

#endif
