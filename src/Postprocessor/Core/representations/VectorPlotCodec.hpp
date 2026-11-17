#pragma once
#include "VectorPlotDefinition.hpp"
#include <QJsonObject>
#include <QJsonValue>
namespace Postprocessor
{
  QJsonObject vector_plot_to_json(const VectorPlotDefinition& value);
  bool vector_plot_from_json(const QJsonValue& json,VectorPlotDefinition& value,std::string& error);
}
