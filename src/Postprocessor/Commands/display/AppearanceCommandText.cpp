#include "AppearanceCommandText.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include <QColor>
#include <map>
#include <set>
namespace Postprocessor
{
  std::vector<QString> appearance_commands(const PostprocessorApplicationInterface& app,
    const std::vector<AppearanceTarget>& targets,const AppearancePatch& patch,std::string& error)
  {
    std::map<std::pair<std::string,std::string>,std::set<std::string>> groups;
    for (const auto& target : targets)
    {
      if (target.view_id != app.session().active_view_id)
      {
        error = "Appearance targets must belong to the active view.";
        return {};
      }
      const auto source = app.session().sources.find(target.source_id);
      bool found = false;
      for (const auto& dataset : app.mesh_datasets(target.source_id))
      {
        if (dataset.id != target.dataset_id || source == app.session().sources.end())
        {
          continue;
        }
        for (auto category : {DatasetCategory::Blocks,DatasetCategory::NodeSets,DatasetCategory::SideSets,DatasetCategory::IntegrationPoints})
        {
          if (dataset_in_category(dataset,category))
          {
            groups[{source->second.name,dataset_category_name(category)}].insert(dataset.name);
            found = true;
          }
        }
      }
      if (!found)
      {
        error = "Selected dataset has no journalable category/name.";
        return {};
      }
    }
    std::vector<QString> operations;
    if (patch.reset)
    {
      operations.push_back("reset");
    }
    else
    {
      if (patch.style)
      {
        operations.push_back(QStringLiteral("style ")+display_style_name(*patch.style));
      }
      if (patch.edges)
      {
        QString value = "off";
        if (*patch.edges)
        {
          value = "on";
        }
        operations.push_back("edges "+value);
      }
      if (patch.opacity)
      {
        operations.push_back("opacity "+QString::number(*patch.opacity,'g',17));
      }
      if (patch.point_size)
      {
        operations.push_back("pointsize "+QString::number(*patch.point_size,'g',17));
      }
      if (patch.color_change == ColorChange::Native)
      {
        operations.push_back("color native");
      }
      if (patch.color_change == ColorChange::Override)
      {
        const auto color = QColor::fromRgbF(patch.color[0],patch.color[1],patch.color[2]);
        operations.push_back("color "+plot_quote(color.name().toStdString()));
      }
    }
    std::vector<QString> commands;
    for (const auto& group : groups)
    {
      QString suffix = " source "+plot_quote(group.first.first)+" "+QString::fromStdString(group.first.second);
      if (plot_quote(group.first.first).isEmpty())
      {
        error = "Source name cannot be quoted in a journal.";
        return {};
      }
      for (const auto& name : group.second)
      {
        if (plot_quote(name).isEmpty())
        {
          error = "Dataset name cannot be quoted in a journal.";
          return {};
        }
        suffix += " "+plot_quote(name);
      }
      for (const auto& operation : operations)
      {
        commands.push_back("post display "+operation+suffix);
      }
    }
    error.clear();
    return commands;
  }
}
