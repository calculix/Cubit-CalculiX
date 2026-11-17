#pragma once
#include "Postprocessor/Core/filters/ThresholdDefinition.hpp"
#include "Postprocessor/Core/representations/ResultFieldSelector.hpp"
#include <QString>
namespace Postprocessor
{
  QString threshold_command_text(const ResultFieldSelector& selector,const ThresholdDefinition& threshold);
}
