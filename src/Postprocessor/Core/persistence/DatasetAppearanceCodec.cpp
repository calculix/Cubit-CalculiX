#include "DatasetAppearanceCodec.hpp"
#include <QJsonArray>
#include <cmath>
namespace Postprocessor
{
  QJsonObject appearance_to_json(const DatasetAppearance& value)
  {
    QJsonObject object{{"style",display_style_name(value.style)},{"edges",value.edges},
      {"opacity",value.opacity},{"point_size",value.point_size}};
    if (value.solid_color)
    {
      object.insert("color",QJsonArray{(*value.solid_color)[0],(*value.solid_color)[1],(*value.solid_color)[2]});
    }
    return object;
  }
  bool appearance_from_json(const QJsonValue& input,DatasetAppearance& value,std::string& error)
  {
    const auto object = input.toObject();
    DatasetAppearance candidate;
    if (!input.isObject() || !object.value("style").isString() || !object.value("edges").isBool() ||
        !object.value("opacity").isDouble() || !object.value("point_size").isDouble() ||
        !parse_display_style(object.value("style").toString().toStdString(),candidate.style))
    {
      error = "Invalid saved dataset appearance.";
      return false;
    }
    candidate.edges = object.value("edges").toBool();
    candidate.opacity = object.value("opacity").toDouble();
    candidate.point_size = object.value("point_size").toDouble();
    if (object.contains("color"))
    {
      const auto color = object.value("color").toArray();
      if (!object.value("color").isArray() || color.size() != 3)
      {
        error = "Saved solid color requires three RGB channels.";
        return false;
      }
      std::array<double,3> rgb{};
      for (int i = 0; i < 3; ++i)
      {
        if (!color[i].isDouble())
        {
          error = "Saved solid color must be numeric.";
          return false;
        }
        rgb[static_cast<std::size_t>(i)] = color[i].toDouble();
      }
      candidate.solid_color = rgb;
    }
    SimResults::Dataset unknown;
    if (!valid_appearance(unknown,candidate,error))
    {
      return false;
    }
    value = candidate;
    return true;
  }
}
