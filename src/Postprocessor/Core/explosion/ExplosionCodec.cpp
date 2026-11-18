#include "ExplosionCodec.hpp"
#include <QJsonArray>
#include <cmath>
#include <algorithm>
#include <utility>
#include <limits>
#include <set>
namespace
{
  QString q(const std::string& value)
  {
    return QString::fromStdString(value);
  }
  bool text(const QJsonObject& object,const char* key,std::string& value)
  {
    if (!object.value(key).isString())
    {
      return false;
    }
    value = object.value(key).toString().toStdString();
    return value.size() <= 65536 && value.find('\0') == std::string::npos;
  }
  QJsonArray point(const std::array<double,3>& value)
  {
    return {value[0],value[1],value[2]};
  }
  bool read_point(const QJsonValue& value,std::array<double,3>& output)
  {
    const auto array = value.toArray();
    if (!value.isArray() || array.size() != 3)
    {
      return false;
    }
    for (int i = 0; i < 3; ++i)
    {
      if (!array[i].isDouble() || !std::isfinite(array[i].toDouble()))
      {
        return false;
      }
      output[i] = array[i].toDouble();
    }
    return true;
  }
  QJsonObject block(const Postprocessor::ExplosionBlockReference& value)
  {
    return {{"kind",q(value.kind)},{"name",q(value.name)},{"model",value.model_id}};
  }
  bool read_block(const QJsonValue& json,Postprocessor::ExplosionBlockReference& value)
  {
    const auto object = json.toObject();
    const double model = object.value("model").toDouble(-2);
    if (!json.isObject() || !text(object,"kind",value.kind) || !text(object,"name",value.name) ||
        !object.value("model").isDouble() || !std::isfinite(model) || model < -1 || model > std::numeric_limits<int>::max() || std::floor(model) != model)
    {
      return false;
    }
    value.model_id = static_cast<int>(model);
    return true;
  }
}
namespace Postprocessor
{
  QJsonObject explosion_to_json(const ExplosionState& state)
  {
    QJsonArray sources,placements,meshes;
    for (const auto& source : state.sources)
    {
      sources.append(q(source));
    }
    for (const auto& piece : state.placements)
    {
      placements.append(QJsonObject{{"source",q(piece.source_id)},{"block",block(piece.block)},
        {"offset",point(piece.offset)},{"manual",point(piece.manual)}});
    }
    for (const auto& mesh : state.meshes)
    {
      QJsonArray blocks;
      for (const auto& value : mesh.second.blocks)
      {
        blocks.append(block(value));
      }
      meshes.append(QJsonObject{{"source",q(mesh.first)},{"enabled",mesh.second.enabled},{"amount",mesh.second.amount},{"blocks",blocks}});
    }
    return {{"enabled",state.enabled},{"amount",state.amount},{"gap",state.gap},{"grouping",static_cast<int>(state.grouping)},
      {"direction",static_cast<int>(state.direction)},{"unit",q(state.coordinate_unit)},
      {"sources",sources},{"placements",placements},{"meshes",meshes},
      {"pulse",QJsonObject{{"period",state.pulse.period_seconds},{"minimum",state.pulse.minimum},
        {"maximum",state.pulse.maximum},{"target",static_cast<int>(state.pulse.target)}}}};
  }
  bool explosion_from_json(const QJsonValue& json,ExplosionState& output,std::string& error)
  {
    error.clear();
    ExplosionState state;
    if (json.isUndefined())
    {
      output = state;
      return true;
    }
    const auto object = json.toObject();
    if (!json.isObject() || !object.value("enabled").isBool() || !object.value("amount").isDouble() || !object.value("gap").isDouble() ||
        !object.value("grouping").isDouble() || !object.value("direction").isDouble() || !text(object,"unit",state.coordinate_unit))
    {
      error = "Invalid saved explosion settings.";
      return false;
    }
    const double grouping = object.value("grouping").toDouble(),direction = object.value("direction").toDouble();
    if (!std::isfinite(grouping) || std::floor(grouping) != grouping || grouping < 0 || grouping > 2 ||
        !std::isfinite(direction) || std::floor(direction) != direction || direction < 0 || direction > 4)
    {
      error = "Invalid saved explosion mode.";
      return false;
    }
    state.enabled = object.value("enabled").toBool();
    state.amount = object.value("amount").toDouble();
    state.gap = object.value("gap").toDouble();
    state.grouping = static_cast<ExplosionGrouping>(static_cast<int>(grouping));
    state.direction = static_cast<ExplosionDirection>(static_cast<int>(direction));
    if (object.contains("pulse"))
    {
      const auto pulse = object.value("pulse").toObject();
      const double target = pulse.value("target").toDouble(-1);
      if (!object.value("pulse").isObject() || !pulse.value("period").isDouble() ||
          !pulse.value("minimum").isDouble() || !pulse.value("maximum").isDouble() ||
          !pulse.value("target").isDouble() || !std::isfinite(target) || std::floor(target) != target || target < 0 || target > 2)
      {
        error = "Invalid saved explosion pulse settings.";
        return false;
      }
      state.pulse.period_seconds = pulse.value("period").toDouble();
      state.pulse.minimum = pulse.value("minimum").toDouble();
      state.pulse.maximum = pulse.value("maximum").toDouble();
      state.pulse.target = static_cast<ExplosionState::Pulse::Target>(static_cast<int>(target));
    }
    for (const auto* key : {"sources","placements","meshes"})
    {
      if (!object.value(key).isArray() || object.value(key).toArray().size() > ExplosionDefaults::maximum_pieces)
      {
        error = "Saved explosion scope exceeds its record limit.";
        return false;
      }
    }
    std::set<std::string> unique;
    for (const auto& item : object.value("sources").toArray())
    {
      const auto source = item.toString().toStdString();
      if (!item.isString() || source.empty() || source.size() > 65536 || !unique.insert(source).second)
      {
        error = "Invalid saved explosion source scope.";
        return false;
      }
      state.sources.push_back(source);
    }
    for (const auto& item : object.value("placements").toArray())
    {
      ExplosionPlacement value;
      const auto entry = item.toObject();
      if (!item.isObject() || !text(entry,"source",value.source_id) || !read_block(entry.value("block"),value.block) ||
          !read_point(entry.value("offset"),value.offset) || !read_point(entry.value("manual"),value.manual))
      {
        error = "Invalid saved explosion placement.";
        return false;
      }
      if (std::any_of(state.placements.begin(),state.placements.end(),[&](const auto& prior)
        {
          return prior.source_id == value.source_id && same_explosion_block(prior.block,value.block);
        }))
      {
        error = "Duplicate saved explosion placement.";
        return false;
      }
      state.placements.push_back(value);
    }
    for (const auto& item : object.value("meshes").toArray())
    {
      const auto entry = item.toObject();
      std::string source;
      MeshExplosion mesh;
      if (!item.isObject() || !text(entry,"source",source) || source.empty() || !entry.value("enabled").isBool() ||
          !entry.value("amount").isDouble() || !entry.value("blocks").isArray() ||
          entry.value("blocks").toArray().size() > ExplosionDefaults::maximum_pieces || state.meshes.count(source))
      {
        error = "Invalid saved mesh explosion.";
        return false;
      }
      mesh.enabled = entry.value("enabled").toBool();
      mesh.amount = entry.value("amount").toDouble();
      for (const auto& item : entry.value("blocks").toArray())
      {
        ExplosionBlockReference value;
        if (!read_block(item,value))
        {
          error = "Invalid saved exploded block.";
          return false;
        }
        mesh.blocks.push_back(value);
      }
      state.meshes.emplace(source,std::move(mesh));
    }
    if (!valid_explosion(state,error))
    {
      return false;
    }
    output = std::move(state);
    return true;
  }
}
