#pragma once
#include <QString>
#include "Postprocessor/Core/representations/ResultFieldSelector.hpp"
namespace Postprocessor
{
  QString vector_command_text(const QString& operation,const std::string& source);
  QString vector_field_command_text(const ResultFieldSelector& field,const std::string& source);
}
