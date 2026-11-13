#ifndef POSTPROCESSOR_PLOTCOMMANDTEXT_HPP
#define POSTPROCESSOR_PLOTCOMMANDTEXT_HPP
#include "Postprocessor/Core/plots/PlotDefinition.hpp"
#include <QString>
class PostprocessorApplicationInterface;
namespace Postprocessor
{
  QString plot_quote(const std::string& value);
  QString plot_axes_command(const std::string& plot,const PlotAxes& axes);
  std::vector<QString> plot_entity_commands(const PostprocessorApplicationInterface& application,
    const EntityReference& entity,PlotReduction reduction = PlotReduction::None);
}
#endif
