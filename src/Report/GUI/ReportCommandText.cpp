#include "ReportCommandText.hpp"

bool ReportCommandText::quote(const QString& value, std::string& output, std::string& error)
{
  for (const auto character : value)
  {
    if (character.unicode() < 32 || character.unicode() == 127)
    {
      error = "Report command text cannot contain control characters.";
      return false;
    }
  }
  if (value.contains('"') && value.contains('\''))
  {
    error = "Report command text cannot contain both quote delimiters.";
    return false;
  }
  const char delimiter = value.contains('"') ? '\'' : '"';
  output = std::string(1, delimiter) + value.toStdString() + delimiter;
  error.clear();
  return true;
}
