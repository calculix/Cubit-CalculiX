#ifndef POSTPROCESSOR_PLOTCODEC_HPP
#define POSTPROCESSOR_PLOTCODEC_HPP
#include "PlotDefinition.hpp"
#include <QJsonArray>
namespace Postprocessor
{
  QJsonArray plots_to_json(const std::map<std::string,PlotDefinition>& plots);
  bool plots_from_json(const QJsonValue& value,std::map<std::string,PlotDefinition>& plots);
}
#endif
