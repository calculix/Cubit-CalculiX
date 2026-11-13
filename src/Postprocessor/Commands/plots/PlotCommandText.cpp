#include "PlotCommandText.hpp"
#include <utility>
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/selection/PickCommandText.hpp"
#include <algorithm>
#include <iomanip>
#include <locale>
#include <sstream>
namespace Postprocessor
{
  QString plot_quote(const std::string& value)
  {
    const char delimiter = value.find('"') == std::string::npos ? '"' : '\'';
    if (value.empty() || value.find(delimiter) != std::string::npos ||
        std::any_of(value.begin(),value.end(),[](unsigned char c)
        {
          return c < 32 || c == 127;
        }))
    {
      return {};
    }
    return QString::fromStdString(std::string(1,delimiter)+value+delimiter);
  }
  QString plot_axes_command(const std::string& plot,const PlotAxes& axes)
  {
    if (!valid_plot_axes(axes) || plot_quote(plot).isEmpty())
    {
      return {};
    }
    if (!axes.x || !axes.y)
    {
      return QStringLiteral("post plot axes auto plot ")+plot_quote(plot);
    }
    std::ostringstream numbers;
    numbers.imbue(std::locale::classic());
    numbers << std::setprecision(17) << "x " << (*axes.x)[0] << ' ' << (*axes.x)[1] <<
      " y " << (*axes.y)[0] << ' ' << (*axes.y)[1];
    return QStringLiteral("post plot axes range ")+QString::fromStdString(numbers.str())+" plot "+plot_quote(plot);
  }
  std::vector<QString> plot_entity_commands(const PostprocessorApplicationInterface& application,
    const EntityReference& entity,PlotReduction reduction)
  {
    const auto& state = application.session();
    std::string pick, error;
    if (!pick_command_text(application,entity,pick,error))
    {
      return {};
    }
    std::optional<FieldSelection> field;
    for (const auto& item : state.representations)
    {
      if (item.second.view_id == state.active_view_id && item.second.input.owner == OutputOwner::Source &&
          item.second.input.object_id == entity.source_id)
      {
        field = item.second.coloring.field;
        break;
      }
    }
    if (!field || (entity.kind == EntityKind::Element && field->association != SimResults::Association::Element &&
        reduction == PlotReduction::None))
    {
      return {};
    }
    const auto key = plot_quote(field->group_key.empty() ? field->array_name : field->group_key);
    const auto component = field->magnitude ? QStringLiteral("magnitude") : plot_quote(field->component);
    if (key.isEmpty() || component.isEmpty())
    {
      return {};
    }
    std::vector<QString> commands;
    auto active = state.plots.find(state.active_plot_id);
    std::string plotName;
    if (active != state.plots.end())
    {
      plotName = active->second.name;
    }
    else
    {
      std::size_t number = 1;
      while (std::any_of(state.plots.begin(),state.plots.end(),[&](const auto& item)
        {
          return item.second.name == "History "+std::to_string(number);
        }))
      {
        ++number;
      }
      plotName = "History "+std::to_string(number);
      commands.push_back(QStringLiteral("post plot create ")+plot_quote(plotName));
    }
    std::size_t number = 1;
    while (active != state.plots.end() && std::any_of(active->second.curves.begin(),active->second.curves.end(),[&](const auto& item)
      {
        return item.name == "curve_"+std::to_string(number);
      }))
    {
      ++number;
    }
    QString command = QStringLiteral("post plot add ")+QString::fromStdString(pick.substr(10))+
      " field "+key+" component "+component;
    if (reduction != PlotReduction::None)
    {
      command += reduction == PlotReduction::Minimum ? " reduction minimum" : " reduction maximum";
    }
    command += " name "+plot_quote("curve_"+std::to_string(number))+" plot "+plot_quote(plotName);
    commands.push_back(command);
    return commands;
  }
}
