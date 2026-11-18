#include "VectorPlotCodec.hpp"
#include <QJsonArray>
#include <cmath>
#include "ResultColoringUnits.hpp"
namespace
{
  QJsonArray rgb(const std::array<double,3>& value)
  {
    return {value[0],value[1],value[2]};
  }
  QJsonObject limit_json(const Postprocessor::ColorLimit& value)
  {
    return {{"enabled",value.enabled},{"value",value.value},{"color",rgb(value.color)}};
  }
  bool read_limit(const QJsonValue& json,Postprocessor::ColorLimit& value)
  {
    const auto object = json.toObject();
    const auto color = object.value("color").toArray();
    if (!json.isObject() || !object.value("enabled").isBool() || !object.value("value").isDouble() || color.size() != 3)
    {
      return false;
    }
    value.enabled = object.value("enabled").toBool();
    value.value = object.value("value").toDouble();
    for (int i = 0; i < 3; ++i)
    {
      if (!color[i].isDouble())
      {
        return false;
      }
      value.color[i] = color[i].toDouble();
    }
    return true;
  }
  QJsonObject legend_json(const Postprocessor::ResultColoring& value)
  {
    QJsonObject result{{"visible",value.legend_visible},{"palette",static_cast<int>(value.palette)},
      {"reversed",value.reversed},{"range_mode",static_cast<int>(value.range_mode)},
      {"range",QJsonArray{value.fixed_range[0],value.fixed_range[1]}},
      {"minimum",limit_json(value.minimum)},{"maximum",limit_json(value.maximum)}};
    QJsonArray colors;
    for (const auto& color : value.custom_palette)
    {
      colors.append(rgb(color));
    }
    result.insert("colors",colors);
    result.insert("palette_hsv",value.palette_hsv);
    if (value.bands)
    {
      result.insert("bands",static_cast<int>(*value.bands));
    }
    if (value.legend_position)
    {
      result.insert("position",QJsonArray{(*value.legend_position)[0],(*value.legend_position)[1]});
    }
    if (value.bounds_unit)
    {
      QJsonArray dimension;
      for (double exponent : value.bounds_unit->dimension)
      {
        dimension.append(exponent);
      }
      result.insert("unit",QJsonObject{{"quantity",QString::fromStdString(value.bounds_unit->quantity)},
        {"unit",QString::fromStdString(value.bounds_unit->unit)},{"dimension",dimension},{"role",static_cast<int>(value.bounds_unit->role)}});
    }
    return result;
  }
  bool read_legend(const QJsonValue& json,Postprocessor::ResultColoring& value,std::string& error)
  {
    if (json.isUndefined())
    {
      return true;
    }
    const auto object = json.toObject();
    const auto range = object.value("range").toArray();
    const double palette = object.value("palette").toDouble(-1);
    const double mode = object.value("range_mode").toDouble(-1);
    if (!json.isObject() || !object.value("visible").isBool() || !object.value("reversed").isBool() ||
        !object.value("palette").isDouble() || !std::isfinite(palette) || palette < 0 || palette > 4 || palette != std::floor(palette) ||
        !object.value("range_mode").isDouble() || !std::isfinite(mode) || mode < 0 || mode > 3 || mode != std::floor(mode) ||
        range.size() != 2 || !range[0].isDouble() || !range[1].isDouble() ||
        !read_limit(object.value("minimum"),value.minimum) || !read_limit(object.value("maximum"),value.maximum))
    {
      error = "Invalid vector legend appearance.";
      return false;
    }
    if (object.contains("palette_hsv") && !object.value("palette_hsv").isBool())
    {
      error = "Invalid vector palette interpolation.";
      return false;
    }
    value.palette_hsv = object.value("palette_hsv").toBool();
    if (object.contains("colors"))
    {
      const auto colors = object.value("colors").toArray();
      if (!object.value("colors").isArray() || colors.size() > static_cast<int>(Postprocessor::ColoringDefaults::maximum_bands))
      {
        return false;
      }
      for (const auto& entry : colors)
      {
        const auto channels = entry.toArray();
        if (channels.size() != 3 || !channels[0].isDouble() || !channels[1].isDouble() || !channels[2].isDouble())
        {
          return false;
        }
        value.custom_palette.push_back({channels[0].toDouble(),channels[1].toDouble(),channels[2].toDouble()});
      }
    }
    value.legend_visible = object.value("visible").toBool();
    value.reversed = object.value("reversed").toBool();
    value.palette = static_cast<Postprocessor::ColorPalette>(static_cast<int>(palette));
    value.range_mode = static_cast<Postprocessor::ColorRangeMode>(static_cast<int>(mode));
    value.fixed_range = {range[0].toDouble(),range[1].toDouble()};
    value.bands.reset();
    if (object.contains("bands"))
    {
      const auto bands = object.value("bands");
      const double count = bands.toDouble(-1);
      if (!bands.isDouble() || !std::isfinite(count) || count < 2 || count > Postprocessor::ColoringDefaults::maximum_bands || count != std::floor(count))
      {
        error = "Invalid vector legend bands.";
        return false;
      }
      value.bands = static_cast<unsigned int>(count);
    }
    if (object.contains("position"))
    {
      const auto position = object.value("position").toArray();
      if (position.size() != 2 || !position[0].isDouble() || !position[1].isDouble())
      {
        error = "Invalid vector legend position.";
        return false;
      }
      value.legend_position = {position[0].toDouble(),position[1].toDouble()};
    }
    if (object.contains("unit"))
    {
      const auto unit = object.value("unit").toObject();
      SimUnits::QuantityUnit descriptor;
      const auto dimension = unit.value("dimension").toArray();
      const auto role = unit.value("role").toDouble(-1);
      if (!unit.value("quantity").isString() || !unit.value("unit").isString() ||
          dimension.size() != static_cast<int>(descriptor.dimension.size()) || !unit.value("role").isDouble() ||
          !std::isfinite(role) || role < 0 || role > static_cast<int>(SimUnits::UnitRole::Angle) || role != std::floor(role))
      {
        error = "Invalid vector legend unit.";
        return false;
      }
      descriptor.quantity = unit.value("quantity").toString().toStdString();
      descriptor.unit = unit.value("unit").toString().toStdString();
      descriptor.role = static_cast<SimUnits::UnitRole>(static_cast<int>(role));
      for (int i = 0; i < dimension.size(); ++i)
      {
        if (!dimension[i].isDouble())
        {
          return false;
        }
        descriptor.dimension[i] = dimension[i].toDouble();
      }
      value.bounds_unit = descriptor;
    }
    return Postprocessor::valid_coloring(value,error);
  }
}
namespace Postprocessor
{
  QJsonObject vector_plot_to_json(const VectorPlotDefinition& value)
  {
    QJsonObject result{{"enabled",value.enabled},{"proportional",value.proportional},
      {"surface",value.surface_only},{"magnitude_color",value.magnitude_color},{"scale",value.scale},
      {"maximum",static_cast<int>(value.maximum)},{"stride",static_cast<int>(value.stride)},
      {"color",QJsonArray{value.color[0],value.color[1],value.color[2]}}};
    result.insert("legend",legend_json(value.legend));
    if (value.field)
    {
      const auto& field = *value.field;
      QJsonObject selection{{"array",QString::fromStdString(field.array_name)},
        {"group",QString::fromStdString(field.group_key)},{"association",static_cast<int>(field.association)}};
      if (field.dataset_id)
      {
        selection.insert("dataset",static_cast<double>(*field.dataset_id));
      }
      result.insert("field",selection);
    }
    return result;
  }
  bool vector_plot_from_json(const QJsonValue& json,VectorPlotDefinition& value,std::string& error)
  {
    error.clear();
    if (json.isUndefined())
    {
      value = {};
      return true;
    }
    const auto object = json.toObject();
    VectorPlotDefinition candidate;
    for (const auto* key : {"enabled","proportional","surface","magnitude_color"})
    {
      if (!object.value(QLatin1String(key)).isBool())
      {
        error = "Invalid vector display flags.";
        return false;
      }
    }
    candidate.enabled = object.value("enabled").toBool();
    candidate.proportional = object.value("proportional").toBool();
    candidate.surface_only = object.value("surface").toBool();
    candidate.magnitude_color = object.value("magnitude_color").toBool();
    if (!object.value("scale").isDouble())
    {
      error = "Invalid vector scale.";
      return false;
    }
    candidate.scale = object.value("scale").toDouble();
    for (const auto* key : {"maximum","stride"})
    {
      const auto item = object.value(QLatin1String(key));
      const double number = item.toDouble(-1);
      if (!item.isDouble() || !std::isfinite(number) || number < 0 || number > 1000000 || number != std::floor(number))
      {
        error = "Invalid vector sampling settings.";
        return false;
      }
    }
    candidate.maximum = static_cast<unsigned int>(object.value("maximum").toInt());
    candidate.stride = static_cast<unsigned int>(object.value("stride").toInt());
    const auto color = object.value("color").toArray();
    if (color.size() != 3)
    {
      error = "Invalid vector color.";
      return false;
    }
    for (int axis = 0; axis < 3; ++axis)
    {
      if (!color[axis].isDouble())
      {
        error = "Invalid vector color channel.";
        return false;
      }
      candidate.color[static_cast<std::size_t>(axis)] = color[axis].toDouble();
    }
    if (object.contains("field"))
    {
      const auto field = object.value("field").toObject();
      const auto association = field.value("association");
      const double kind = association.toDouble(-1);
      if (!field.value("array").isString() || field.value("array").toString().isEmpty() ||
          !field.value("group").isString() || !association.isDouble() || kind < 0 || kind > 2 || kind != std::floor(kind))
      {
        error = "Invalid vector field identity.";
        return false;
      }
      FieldSelection selection;
      selection.array_name = field.value("array").toString().toStdString();
      selection.group_key = field.value("group").toString().toStdString();
      selection.association = static_cast<SimResults::Association>(static_cast<int>(kind));
      selection.magnitude = true;
      if (field.contains("dataset"))
      {
        const auto id = field.value("dataset");
        const double number = id.toDouble(-1);
        if (!id.isDouble() || !std::isfinite(number) || number < 0 || number > 2147483647 || number != std::floor(number))
        {
          error = "Invalid vector dataset identity.";
          return false;
        }
        selection.dataset_id = static_cast<std::size_t>(number);
      }
      candidate.field = selection;
    }
    if (!read_legend(object.value("legend"),candidate.legend,error))
    {
      return false;
    }
    if (!valid_vector_plot(candidate,error))
    {
      return false;
    }
    value = candidate;
    return true;
  }
}
