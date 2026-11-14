#pragma once
#include "Postprocessor/Core/representations/ResultFieldSelector.hpp"
#include <QString>
namespace Postprocessor
{
  // Includes the field keyword; quote/validation failure returns an empty string.
  QString field_command_text(const ResultFieldSelector& selector);
}
