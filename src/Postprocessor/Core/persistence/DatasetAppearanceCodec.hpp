#pragma once
#include "Postprocessor/Core/representations/DatasetAppearance.hpp"
#include <QJsonObject>
#include <QJsonValue>
namespace Postprocessor
{
  QJsonObject appearance_to_json(const DatasetAppearance& value);
  bool appearance_from_json(const QJsonValue& input,DatasetAppearance& value,std::string& error);
}
