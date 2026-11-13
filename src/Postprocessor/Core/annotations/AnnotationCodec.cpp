#include "AnnotationCodec.hpp"
#include <utility>
#include "../persistence/PostprocessorProjectState.hpp"
#include <QJsonArray>
#include <charconv>
#include <cmath>
#include <limits>
namespace
{
  QString q(const std::string& value)
  {
    return QString::fromUtf8(value.c_str());
  }
  QJsonArray position_json(const std::array<double,2>& value)
  {
    return {value[0],value[1]};
  }
  bool position_read(const QJsonValue& value, std::array<double,2>& output)
  {
    const auto array = value.toArray();
    if (!value.isArray() || array.size() != 2 || !array[0].isDouble() || !array[1].isDouble())
    {
      return false;
    }
    output = {array[0].toDouble(),array[1].toDouble()};
    return Postprocessor::valid_annotation_position(output);
  }
  bool string_read(const QJsonObject& object, const char* key, std::string& output)
  {
    const auto value = object.value(QLatin1String(key));
    if (!value.isString())
    {
      return false;
    }
    output = value.toString().toUtf8().toStdString();
    return output.find('\0') == std::string::npos;
  }
  bool id_read(const QJsonValue& value, std::int64_t& output)
  {
    if (!value.isString())
    {
      return false;
    }
    const auto text = value.toString().toStdString();
    const auto parsed = std::from_chars(text.data(),text.data()+text.size(),output);
    return parsed.ec == std::errc{} && parsed.ptr == text.data()+text.size() && output >= 0;
  }
}
namespace Postprocessor
{
  QJsonObject annotations_to_json(const AnnotationState& state)
  {
    QJsonArray labels;
    for (const auto& entry : state.labels)
    {
      const auto& label = entry.second;
      labels.append(QJsonObject{{"name",q(label.name)},{"source",q(label.entity.source_id)},
        {"kind",static_cast<int>(label.entity.kind)},{"id",q(std::to_string(label.entity.entity_id))},
        {"ip",label.entity.integration_point ? QJsonValue(q(std::to_string(*label.entity.integration_point))) : QJsonValue(QJsonValue::Null)},
        {"dataset_kind",q(label.dataset_kind)},{"dataset_name",q(label.dataset_name)},{"model",label.model_id},
        {"array",q(label.field.array_name)},{"association",static_cast<int>(label.field.association)},
        {"component",q(label.field.component)},{"magnitude",label.field.magnitude},{"group",q(label.field.group_key)},
        {"visible",label.visible},{"position",position_json(label.position)}});
    }
    return {{"visible",state.visible},{"extrema",state.extrema_visible},{"minimum",position_json(state.minimum_position)},
      {"maximum",position_json(state.maximum_position)},{"labels",labels}};
  }
  bool annotations_from_json(const QJsonValue& value, AnnotationState& output)
  {
    if (value.isUndefined())
    {
      output = AnnotationState{};
      return true;
    }
    if (!value.isObject())
    {
      return false;
    }
    AnnotationState state;
    const auto object = value.toObject();
    if (!object.value("visible").isBool() || !object.value("extrema").isBool() ||
        !position_read(object.value("minimum"),state.minimum_position) ||
        !position_read(object.value("maximum"),state.maximum_position) || !object.value("labels").isArray() ||
        object.value("labels").toArray().size() > PersistenceDefaults::maximum_records)
    {
      return false;
    }
    state.visible = object.value("visible").toBool();
    state.extrema_visible = object.value("extrema").toBool();
    for (const auto& item : object.value("labels").toArray())
    {
      if (!item.isObject())
      {
        return false;
      }
      const auto json = item.toObject();
      ResultAnnotation label;
      if (!string_read(json,"name",label.name) || !valid_annotation_name(label.name) ||
          !string_read(json,"source",label.entity.source_id) || label.entity.source_id.empty() ||
          !id_read(json.value("id"),label.entity.entity_id) ||
          !string_read(json,"dataset_kind",label.dataset_kind) || !string_read(json,"dataset_name",label.dataset_name) ||
          !string_read(json,"array",label.field.array_name) || label.field.array_name.empty() ||
          !string_read(json,"component",label.field.component) || !string_read(json,"group",label.field.group_key) ||
          !json.value("magnitude").isBool() || !json.value("visible").isBool() ||
          !position_read(json.value("position"),label.position))
      {
        return false;
      }
      const auto kind = json.value("kind").toDouble(-1);
      const auto association = json.value("association").toDouble(-1);
      const auto model = json.value("model").toDouble(-2);
      if (!std::isfinite(kind) || kind < 0 || kind > 2 || kind != std::floor(kind) ||
          !std::isfinite(association) || association < 0 || association > 2 || association != std::floor(association) ||
          !std::isfinite(model) || model < -1 || model > std::numeric_limits<int>::max() || model != std::floor(model))
      {
        return false;
      }
      label.entity.kind = static_cast<EntityKind>(static_cast<int>(kind));
      label.field.association = static_cast<SimResults::Association>(static_cast<int>(association));
      label.model_id = static_cast<int>(model);
      if (label.entity.kind == EntityKind::IntegrationPoint)
      {
        std::int64_t ip = 0;
        if (!id_read(json.value("ip"),ip) || ip < 1 || ip > 2147483647)
        {
          return false;
        }
        label.entity.integration_point = ip;
      }
      else if (!json.value("ip").isNull())
      {
        return false;
      }
      label.field.magnitude = json.value("magnitude").toBool();
      label.visible = json.value("visible").toBool();
      if (!state.labels.emplace(label.name,label).second)
      {
        return false;
      }
    }
    output = std::move(state);
    return true;
  }
}
