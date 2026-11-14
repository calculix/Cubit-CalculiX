#pragma once
#include "FilterDefinition.hpp"
#include <QJsonArray>
#include <QJsonValue>
#include <map>
namespace Postprocessor
{
  QJsonArray filters_to_json(const std::map<std::string,FilterDefinition>& filters);
  bool filters_from_json(const QJsonValue& input,std::map<std::string,FilterDefinition>& output,std::string& error);
}
