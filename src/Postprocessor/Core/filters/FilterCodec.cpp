#include "FilterCodec.hpp"
#include <QJsonObject>
#include <cmath>
#include <set>
namespace Postprocessor
{
  QJsonArray filters_to_json(const std::map<std::string,FilterDefinition>& filters)
  {
    QJsonArray result;
    for (const auto& item : filters)
    {
      const auto& value = item.second;
      result.append(QJsonObject{{"id",QString::fromStdString(value.id)},{"name",QString::fromStdString(value.name)},
        {"source",QString::fromStdString(value.input.object_id)},{"kind",[&]() -> const char* {
          if (value.kind == FilterKind::Clip) {
            return "clip";
          } else {
            return "section";
          }
        }()},
        {"keep",[&]() -> const char* {
          if (value.keep == ClipSide::Positive) {
            return "positive";
          } else {
            return "negative";
          }
        }()},
        {"origin",QJsonArray{value.plane.origin[0],value.plane.origin[1],value.plane.origin[2]}},
        {"normal",QJsonArray{value.plane.normal[0],value.plane.normal[1],value.plane.normal[2]}}});
    }
    return result;
  }
  bool filters_from_json(const QJsonValue& input,std::map<std::string,FilterDefinition>& output,std::string& error)
  {
    std::map<std::string,FilterDefinition> result;
    std::set<std::string> names;
    if (input.isUndefined())
    {
      output.clear();
      error.clear();
      return true;
    }
    if (!input.isArray() || input.toArray().size() > static_cast<int>(FilterDefaults::maximum_filters))
    {
      error = "Invalid saved filter registry.";
      return false;
    }
    for (const auto& entry : input.toArray())
    {
      const auto object = entry.toObject();
      FilterDefinition filter;
      for (const auto* key : {"id","name","source","kind","keep"})
      {
        if (!object.value(QLatin1String(key)).isString())
        {
          error = "Invalid saved filter text.";
          return false;
        }
      }
      filter.id = object.value("id").toString().toStdString();
      filter.name = object.value("name").toString().toStdString();
      filter.input = {OutputOwner::Source,object.value("source").toString().toStdString(),0};
      const auto kind = object.value("kind").toString();
      const auto keep = object.value("keep").toString();
      if ((kind != "clip" && kind != "section") || (keep != "positive" && keep != "negative"))
      {
        error = "Invalid saved filter kind or side.";
        return false;
      }
      if (kind == "clip") {
        filter.kind = FilterKind::Clip;
      } else {
        filter.kind = FilterKind::Section;
      }
      if (keep == "positive") {
        filter.keep = ClipSide::Positive;
      } else {
        filter.keep = ClipSide::Negative;
      }
      for (const auto* key : {"origin","normal"})
      {
        const auto values = object.value(QLatin1String(key)).toArray();
        auto& vector = [&]() -> decltype((filter.plane.origin)) {
          if (key[0] == 'o') {
            return filter.plane.origin;
          } else {
            return filter.plane.normal;
          }
        }();
        if (values.size() != 3)
        {
          error = "A saved plane requires three finite coordinates.";
          return false;
        }
        for (int i = 0; i < 3; ++i)
        {
          if (!values[i].isDouble() || !std::isfinite(values[i].toDouble()))
          {
            error = "Invalid saved plane coordinate.";
            return false;
          }
          vector[i] = values[i].toDouble();
        }
      }
      if (!normalize_plane(filter.plane,error) || !valid_filter(filter,error))
      {
        return false;
      }
      if (!names.insert(filter.name).second || !result.emplace(filter.id,filter).second)
      {
        error = "Saved filters require unique identifiers and names.";
        return false;
      }
    }
    output = std::move(result);
    error.clear();
    return true;
  }
}
