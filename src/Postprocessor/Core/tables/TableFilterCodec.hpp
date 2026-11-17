#pragma once
#include "TableFilters.hpp"
#include <QJsonArray>
namespace Postprocessor
{
  QJsonArray table_filters_to_json(const std::vector<TableFilter>& filters);
  bool table_filters_from_json(const QJsonValue& json,std::vector<TableFilter>& filters);
}
