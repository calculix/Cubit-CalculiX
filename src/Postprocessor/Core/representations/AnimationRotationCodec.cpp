#include "AnimationRotationCodec.hpp"
#include <QJsonObject>
namespace Postprocessor
{
  QJsonArray animation_rotation_to_json(const AnimationRotation& value)
  {
    QJsonArray result;
    for (const auto& axis : value)
    {
      result.append(QJsonObject{{"enabled",axis.enabled},{"degrees",axis.degrees}});
    }
    return result;
  }
  bool animation_rotation_from_json(const QJsonValue& json,AnimationRotation& output,std::string& error)
  {
    error.clear();
    AnimationRotation candidate;
    if (json.isUndefined())
    {
      output = candidate;
      return true;
    }
    if (!json.isArray() || json.toArray().size() != 3)
    {
      error = "Saved animation rotation requires three axes.";
      return false;
    }
    const auto axes = json.toArray();
    for (int axis = 0; axis < axes.size(); ++axis)
    {
      const auto entry = axes[axis].toObject();
      if (!entry.value("enabled").isBool() || !entry.value("degrees").isDouble())
      {
        error = "Invalid saved animation rotation settings.";
        return false;
      }
      candidate[static_cast<std::size_t>(axis)] = {entry.value("enabled").toBool(),entry.value("degrees").toDouble()};
    }
    if (!valid_animation_rotation(candidate,error))
    {
      return false;
    }
    output = candidate;
    return true;
  }
}
