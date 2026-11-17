#include "VectorPlotCodec.hpp"
#include <QJsonArray>
#include <cmath>
namespace Postprocessor
{
  QJsonObject vector_plot_to_json(const VectorPlotDefinition& value)
  {
    QJsonObject result{{"enabled",value.enabled},{"proportional",value.proportional},
      {"surface",value.surface_only},{"magnitude_color",value.magnitude_color},{"scale",value.scale},
      {"maximum",static_cast<int>(value.maximum)},{"stride",static_cast<int>(value.stride)},
      {"color",QJsonArray{value.color[0],value.color[1],value.color[2]}}};
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
    if (!valid_vector_plot(candidate,error))
    {
      return false;
    }
    value = candidate;
    return true;
  }
}
