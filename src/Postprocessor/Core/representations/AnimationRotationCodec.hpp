#pragma once
#include "AnimationRotation.hpp"
#include <QJsonArray>
namespace Postprocessor
{
  QJsonArray animation_rotation_to_json(const AnimationRotation& value);
  bool animation_rotation_from_json(const QJsonValue& json,AnimationRotation& output,std::string& error);
}
