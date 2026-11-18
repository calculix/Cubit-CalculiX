#pragma once
#include "ExplosionDefinition.hpp"
#include <QJsonObject>
namespace Postprocessor
{
  QJsonObject explosion_to_json(const ExplosionState& state);
  bool explosion_from_json(const QJsonValue& json,ExplosionState& state,std::string& error);
}
