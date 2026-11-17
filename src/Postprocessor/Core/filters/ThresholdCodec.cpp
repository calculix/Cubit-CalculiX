#include "ThresholdCodec.hpp"
#include <QJsonArray>
#include <cmath>
#include <utility>
#include <limits>
namespace Postprocessor
{
  QJsonObject threshold_to_json(const ThresholdDefinition& value)
  {
    QJsonValue lower(QJsonValue::Null), upper(QJsonValue::Null), dataset(QJsonValue::Null);
    if (value.lower)
    {
      lower = *value.lower;
    }
    if (value.upper)
    {
      upper = *value.upper;
    }
    if (value.field.dataset_id)
    {
      dataset = QString::number(static_cast<qulonglong>(*value.field.dataset_id));
    }
    QJsonArray dimension;
    QJsonValue unit(QJsonValue::Null);
    if (value.bounds_unit)
    {
      for (const auto exponent : value.bounds_unit->dimension)
      {
        dimension.append(exponent);
      }
      unit = QJsonObject{{"quantity",QString::fromStdString(value.bounds_unit->quantity)},
        {"unit",QString::fromStdString(value.bounds_unit->unit)},
        {"dimension",dimension},{"role",static_cast<int>(value.bounds_unit->role)}};
    }
    return {{"array",QString::fromStdString(value.field.array_name)},
      {"group",QString::fromStdString(value.field.group_key)},
      {"association",static_cast<int>(value.field.association)},
      {"component",QString::fromStdString(value.field.component)},{"magnitude",value.field.magnitude},
      {"dataset",dataset},{"dataset_kind",QString::fromStdString(value.dataset_kind)},
      {"dataset_name",QString::fromStdString(value.dataset_name)},{"dataset_model",value.dataset_model},
      {"lower",lower},{"upper",upper},{"bounds_unit",unit},
      {"nodes",value.node_rule == ThresholdNodeRule::All ? "all" : "any"}};
  }
  bool threshold_from_json(const QJsonValue& json,ThresholdDefinition& output,std::string& error)
  {
    error = "Invalid saved threshold definition.";
    if (!json.isObject())
    {
      return false;
    }
    const auto object = json.toObject();
    ThresholdDefinition value;
    for (const auto* key : {"array","group","component","nodes","dataset_kind","dataset_name"})
    {
      if (!object.value(key).isString() || object.value(key).toString().size() > 4096)
      {
        return false;
      }
    }
    const double association = object.value("association").toDouble(-1);
    const double model = object.value("dataset_model").toDouble(-2);
    if (!std::isfinite(association) || association != std::floor(association) || association < 0 || association > 2 ||
        !std::isfinite(model) || model != std::floor(model) || model < -1 || model > std::numeric_limits<int>::max() ||
        !object.value("magnitude").isBool())
    {
      return false;
    }
    value.field.array_name = object.value("array").toString().toStdString();
    value.field.group_key = object.value("group").toString().toStdString();
    value.field.component = object.value("component").toString().toStdString();
    value.field.magnitude = object.value("magnitude").toBool();
    value.field.association = static_cast<SimResults::Association>(static_cast<int>(association));
    value.dataset_kind = object.value("dataset_kind").toString().toStdString();
    value.dataset_name = object.value("dataset_name").toString().toStdString();
    value.dataset_model = static_cast<int>(model);
    const auto dataset = object.value("dataset");
    if (!dataset.isNull())
    {
      bool valid = false;
      const auto text = dataset.toString();
      const auto id = text.toULongLong(&valid);
      if (!dataset.isString() || !valid || text != QString::number(id) ||
          id > std::numeric_limits<std::size_t>::max() || value.dataset_kind.empty())
      {
        return false;
      }
      value.field.dataset_id = static_cast<std::size_t>(id);
    }
    for (const auto* key : {"lower","upper"})
    {
      const auto limit = object.value(key);
      if (!limit.isNull())
      {
        if (!limit.isDouble() || !std::isfinite(limit.toDouble()))
        {
          return false;
        }
        if (key[0] == 'l')
        {
          value.lower = limit.toDouble();
        }
        else
        {
          value.upper = limit.toDouble();
        }
      }
    }
    const auto nodes = object.value("nodes").toString();
    if (nodes != "all" && nodes != "any")
    {
      return false;
    }
    if (nodes == "any")
    {
      value.node_rule = ThresholdNodeRule::Any;
    }
    const auto unit = object.value("bounds_unit").toObject();
    SimUnits::QuantityUnit descriptor;
    const auto dimension = unit.value("dimension").toArray();
    const double role = unit.value("role").toDouble(-1);
    if (!unit.value("quantity").isString() || !unit.value("unit").isString() ||
        dimension.size() != static_cast<int>(descriptor.dimension.size()) ||
        !std::isfinite(role) || role != std::floor(role) || role < 0 || role > 3)
    {
      return false;
    }
    descriptor.quantity = unit.value("quantity").toString().toStdString();
    descriptor.unit = unit.value("unit").toString().toStdString();
    descriptor.role = static_cast<SimUnits::UnitRole>(static_cast<int>(role));
    for (int index = 0; index < dimension.size(); ++index)
    {
      if (!dimension[index].isDouble() || !std::isfinite(dimension[index].toDouble()))
      {
        return false;
      }
      descriptor.dimension[static_cast<std::size_t>(index)] = dimension[index].toDouble();
    }
    value.bounds_unit = descriptor;
    if (!valid_threshold(value,error))
    {
      return false;
    }
    output = std::move(value);
    return true;
  }
}
