#include "PlotCodec.hpp"
#include <utility>
#include <QJsonObject>
#include <charconv>
#include <cmath>
#include <limits>
#include <set>
namespace
{
  QString q(const std::string& value)
  {
    return QString::fromUtf8(value.c_str());
  }
  bool text(const QJsonObject& object,const char* key,std::string& output)
  {
    const auto value = object.value(QLatin1String(key));
    if (!value.isString())
    {
      return false;
    }
    output = value.toString().toUtf8().toStdString();
    return output.find('\0') == std::string::npos;
  }
  bool integer(const QJsonObject& object,const char* key,int low,int high,int& output)
  {
    const auto item = object.value(QLatin1String(key));
    const double value = item.toDouble(static_cast<double>(low)-1);
    if (!item.isDouble() || !std::isfinite(value) || value < low || value > high || value != std::floor(value))
    {
      return false;
    }
    output = static_cast<int>(value);
    return true;
  }
  bool id_read(const QJsonValue& value,std::int64_t& output)
  {
    if (!value.isString())
    {
      return false;
    }
    const auto text = value.toString().toStdString();
    const auto parsed = std::from_chars(text.data(),text.data()+text.size(),output);
    return parsed.ec == std::errc{} && parsed.ptr == text.data()+text.size() && output >= 0;
  }
  QJsonValue range_json(const std::optional<std::array<double,2>>& range)
  {
    return range ? QJsonValue(QJsonArray{(*range)[0],(*range)[1]}) : QJsonValue(QJsonValue::Null);
  }
  bool range_read(const QJsonValue& value,std::optional<std::array<double,2>>& output)
  {
    if (value.isNull())
    {
      output.reset();
      return true;
    }
    const auto items = value.toArray();
    if (!value.isArray() || items.size() != 2 || !items[0].isDouble() || !items[1].isDouble())
    {
      return false;
    }
    output = std::array<double,2>{items[0].toDouble(),items[1].toDouble()};
    return true;
  }
}
namespace Postprocessor
{
  QJsonArray plots_to_json(const std::map<std::string,PlotDefinition>& plots)
  {
    QJsonArray output;
    for (const auto& entry : plots)
    {
      const auto& plot = entry.second;
      QJsonArray curves;
      for (const auto& curve : plot.curves)
      {
        QJsonObject item{{"name",q(curve.name)},{"visible",curve.visible},
          {"color",QJsonArray{static_cast<int>(curve.color[0]),static_cast<int>(curve.color[1]),static_cast<int>(curve.color[2])}}};
        if (const auto* entity = std::get_if<PlotEntityReference>(&curve.input))
        {
          item.insert("input",QJsonObject{{"type","entity"},{"source",q(entity->entity.source_id)},
            {"kind",static_cast<int>(entity->entity.kind)},{"id",q(std::to_string(entity->entity.entity_id))},
            {"ip",entity->entity.integration_point ? QJsonValue(q(std::to_string(*entity->entity.integration_point))) : QJsonValue(QJsonValue::Null)},
            {"dataset_kind",q(entity->dataset_kind)},{"dataset_name",q(entity->dataset_name)},{"model",entity->model_id},
            {"array",q(entity->field.array_name)},{"association",static_cast<int>(entity->field.association)},
            {"component",q(entity->field.component)},{"magnitude",entity->field.magnitude},
            {"group",q(entity->field.group_key)},{"reduction",static_cast<int>(entity->reduction)}});
        }
        else
        {
          const auto& table = std::get<PlotTableReference>(curve.input);
          item.insert("input",QJsonObject{{"type","table"},{"source",q(table.source_id)},{"key",q(table.key)},
            {"x",q(table.x)},{"y",q(table.y)},{"filter_column",q(table.filter_column)},{"filter_value",q(table.filter_value)}});
        }
        curves.append(item);
      }
      output.append(QJsonObject{{"id",q(plot.id)},{"name",q(plot.name)},{"x",range_json(plot.axes.x)},
        {"y",range_json(plot.axes.y)},{"curves",curves}});
    }
    return output;
  }
  bool plots_from_json(const QJsonValue& value,std::map<std::string,PlotDefinition>& output)
  {
    if (value.isUndefined())
    {
      output.clear();
      return true;
    }
    if (!value.isArray() || value.toArray().size() > PlotDefaults::maximum_plots)
    {
      return false;
    }
    std::map<std::string,PlotDefinition> plots;
    std::set<std::string> names;
    for (const auto& entry : value.toArray())
    {
      if (!entry.isObject())
      {
        return false;
      }
      const auto object = entry.toObject();
      PlotDefinition plot;
      if (!text(object,"id",plot.id) || plot.id.empty() || !text(object,"name",plot.name) ||
          !valid_plot_name(plot.name) || !names.insert(plot.name).second ||
          !range_read(object.value("x"),plot.axes.x) || !range_read(object.value("y"),plot.axes.y) || !valid_plot_axes(plot.axes) ||
          !object.value("curves").isArray() || object.value("curves").toArray().size() > PlotDefaults::maximum_curves)
      {
        return false;
      }
      std::set<std::string> curveNames;
      for (const auto& entry : object.value("curves").toArray())
      {
        const auto item = entry.toObject();
        PlotCurveDefinition curve;
        if (!entry.isObject() || !text(item,"name",curve.name) || !valid_plot_name(curve.name) ||
            !curveNames.insert(curve.name).second || !item.value("visible").isBool() || !item.value("input").isObject())
        {
          return false;
        }
        curve.visible = item.value("visible").toBool();
        curve.color = PlotDefaults::colors[plot.curves.size()%PlotDefaults::colors.size()];
        if (item.contains("color"))
        {
          const auto color = item.value("color").toArray();
          if (!item.value("color").isArray() || color.size() != 3)
          {
            return false;
          }
          for (int index = 0; index < 3; ++index)
          {
            const auto channel = color[index].toDouble(-1);
            if (!color[index].isDouble() || !std::isfinite(channel) || channel < 0 || channel > 255 || channel != std::floor(channel))
            {
              return false;
            }
            curve.color[index] = static_cast<unsigned char>(channel);
          }
        }
        const auto input = item.value("input").toObject();
        std::string type, source;
        if (!text(input,"type",type) || !text(input,"source",source) || source.empty())
        {
          return false;
        }
        if (type == "entity")
        {
          PlotEntityReference reference;
          reference.entity.source_id = source;
          int kind = -1, association = -1, reduction = -1;
          if (!integer(input,"kind",0,2,kind) || !integer(input,"association",0,2,association) ||
              !integer(input,"reduction",0,2,reduction) || !id_read(input.value("id"),reference.entity.entity_id) ||
              !integer(input,"model",-1,std::numeric_limits<int>::max(),reference.model_id) ||
              !text(input,"dataset_kind",reference.dataset_kind) || !text(input,"dataset_name",reference.dataset_name) ||
              !text(input,"array",reference.field.array_name) || reference.field.array_name.empty() ||
              !text(input,"component",reference.field.component) || !text(input,"group",reference.field.group_key) ||
              !input.value("magnitude").isBool())
          {
            return false;
          }
          reference.entity.kind = static_cast<EntityKind>(kind);
          reference.field.association = static_cast<SimResults::Association>(association);
          reference.reduction = static_cast<PlotReduction>(reduction);
          reference.field.magnitude = input.value("magnitude").toBool();
          if (reference.entity.kind == EntityKind::IntegrationPoint)
          {
            std::int64_t ip = 0;
            if (!id_read(input.value("ip"),ip) || ip < 1 || ip > 2147483647)
            {
              return false;
            }
            reference.entity.integration_point = ip;
          }
          else if (!input.value("ip").isNull())
          {
            return false;
          }
          curve.input = reference;
        }
        else if (type == "table")
        {
          PlotTableReference reference;
          reference.source_id = source;
          if (!text(input,"key",reference.key) || reference.key.empty() || !text(input,"x",reference.x) || reference.x.empty() ||
              !text(input,"y",reference.y) || reference.y.empty() || !text(input,"filter_column",reference.filter_column) ||
              !text(input,"filter_value",reference.filter_value))
          {
            return false;
          }
          curve.input = reference;
        }
        else
        {
          return false;
        }
        plot.curves.push_back(curve);
      }
      if (!plots.emplace(plot.id,plot).second)
      {
        return false;
      }
    }
    output = std::move(plots);
    return true;
  }
}
