#include "FieldCommandText.hpp"
#include <algorithm>
#include <cctype>
namespace Postprocessor
{
  QString field_command_text(const ResultFieldSelector& selector)
  {
    const auto quote = [](const std::string& value) -> QString
    {
      const char delimiter = [&]() -> char {
        if (value.find('"') == std::string::npos) {
          return '"';
        } else {
          return '\'';
        }
      }();
      if (value.empty() || value.find(delimiter) != std::string::npos || std::any_of(value.begin(),value.end(),[](unsigned char c)
        {
          return c < 32 || c == 127;
        }))
      {
        return {};
      }
      return QString::fromUtf8((std::string(1,delimiter)+value+delimiter).c_str());
    };
    const auto name = quote(selector.name);
    if (name.isEmpty())
    {
      return {};
    }
    QString result = "field " + name;
    if (selector.qualifiers.origin)
    {
      const auto origin = quote(*selector.qualifiers.origin);
      if (origin.isEmpty())
      {
        return {};
      }
      result += " origin " + origin;
    }
    if (selector.qualifiers.association)
    {
      result += " association " + QString::fromLatin1(field_association_name(*selector.qualifiers.association));
    }
    if (selector.qualifiers.set_name)
    {
      const auto name = quote(*selector.qualifiers.set_name);
      if (name.isEmpty())
      {
        return {};
      }
      result += " set " + name;
    }
    return result;
  }
}
