#pragma once

#include "Framework/ModelExport.hpp"
#include <QString>
#include <string>

// Display only: returned text must never be parsed, persisted as a unit
// expression, or used to generate commands. Numeric values are not converted.
class CUBITSIM_MODEL_EXPORT UnitLabelFormat
{
public:
  // Formats flat products/quotients with integer or half-integer powers.
  // Other expressions retain their original spelling.
  static QString format(const std::string& expression);
  // Explicit symbolic powers for material headings, e.g. -n and -(m+1).
  static QString power(const std::string& expression, const QString& exponent);
};
