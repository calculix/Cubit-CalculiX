#pragma once
#include "ThresholdDefinition.hpp"
#include <QJsonObject>
namespace Postprocessor
{
  QJsonObject threshold_to_json(const ThresholdDefinition& value);
  bool threshold_from_json(const QJsonValue& json,ThresholdDefinition& output,std::string& error);
}
