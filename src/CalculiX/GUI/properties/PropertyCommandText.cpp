#include "PropertyCommandText.hpp"

bool PropertyCommandText::quoteName(const QString& name, QString& quoted, QString& error)
{
  quoted.clear();
  error.clear();
  if (name.isEmpty())
  {
    error = "A name cannot be empty.";
    return false;
  }
  for (QChar character : name)
  {
    if (character.unicode() < 0x20 || character.unicode() == 0x7f)
    {
      error = "A name must be a single line without control characters.";
      return false;
    }
  }
  const QChar delimiter = name.contains('"') ? QChar(0x27) : QChar('"');
  if (name.contains(delimiter))
  {
    error = "A name containing both single and double quotes cannot be quoted for the command.";
    return false;
  }
  quoted = QString(1, delimiter) + name + delimiter;
  return true;
}
