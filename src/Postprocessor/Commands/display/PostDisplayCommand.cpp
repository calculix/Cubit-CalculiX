#include "PostDisplayCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <QColor>
#include <utility>
#include <exception>
PostDisplayCommand::PostDisplayCommand(std::weak_ptr<PostprocessorApplicationInterface> value) : application(std::move(value))
{
}
std::vector<std::string> PostDisplayCommand::get_syntax()
{
  const std::string targets = " source <string:type='quoted',number='1',label='source_name'> {blocks|nodesets|sidesets|integrationpoints} <string:type='quoted',number='1',label='dataset_names'>...";
  return {"post tree ids {on|off}","post display style {wireframe|hiddenline|shaded|transparent|points|roundedpoints}"+targets,
    "post display color <string:type='quoted',number='1',label='rgb'>"+targets,
    "post display color native"+targets,"post display edges {on|off}"+targets,
    "post display opacity <value:label='opacity'>"+targets,"post display pointsize <value:label='pointsize'>"+targets,
    "post display reset"+targets};
}
std::vector<std::string> PostDisplayCommand::get_syntax_help()
{
  return get_syntax();
}
std::vector<std::string> PostDisplayCommand::get_help()
{
  return {"Change selected dataset appearance atomically in the active view. Filter outputs inherit source settings.",
    "Quoted dataset names are category-qualified. RGB uses #RRGGBB; scalar results override solid colors."};
}
bool PostDisplayCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto app = application.lock();
    if (app && data.find_keyword("TREE"))
    {
      std::string page = "treeids-off";
      if (data.find_keyword("ON"))
      {
        page = "treeids-on";
      }
      if (app->show_properties(page,"",error))
      {
        return true;
      }
      PRINT_ERROR("Cannot change tree columns: %s\n",error.c_str());
      return false;
    }
    std::string source;
    std::vector<std::string> names;
    bool hasNames = data.get_strings("dataset_names",names);
    if (!hasNames)
    {
      std::string single;
      if (data.get_string("dataset_names",single))
      {
        names.push_back(single);
        hasNames = true;
      }
    }
    if (!app || !data.get_string("source_name",source) || !hasNames)
    {
      PRINT_ERROR("Appearance requires an available source and dataset names.\n");
      return false;
    }
    Postprocessor::DatasetCategory category = Postprocessor::DatasetCategory::Blocks;
    if (data.find_keyword("NODESETS"))
    {
      category = Postprocessor::DatasetCategory::NodeSets;
    }
    if (data.find_keyword("SIDESETS"))
    {
      category = Postprocessor::DatasetCategory::SideSets;
    }
    if (data.find_keyword("INTEGRATIONPOINTS"))
    {
      category = Postprocessor::DatasetCategory::IntegrationPoints;
    }
    Postprocessor::AppearancePatch patch;
    patch.reset = data.find_keyword("RESET");
    if (data.find_keyword("STYLE"))
    {
      for (const auto& option : {std::pair<const char*,Postprocessor::DisplayStyle>{"WIREFRAME",Postprocessor::DisplayStyle::Wireframe},
          {"HIDDENLINE",Postprocessor::DisplayStyle::HiddenLine},{"SHADED",Postprocessor::DisplayStyle::Shaded},
          {"TRANSPARENT",Postprocessor::DisplayStyle::Transparent},{"POINTS",Postprocessor::DisplayStyle::Points},
          {"ROUNDEDPOINTS",Postprocessor::DisplayStyle::RoundedPoints}})
      {
        if (data.find_keyword(option.first))
        {
          patch.style = option.second;
        }
      }
    }
    if (data.find_keyword("COLOR"))
    {
      patch.color_change = Postprocessor::ColorChange::Native;
      if (!data.find_keyword("NATIVE"))
      {
        std::string text;
        data.get_string("rgb",text);
        const QColor color(QString::fromStdString(text));
        if (text.size() != 7 || text.front() != '#' || !color.isValid())
        {
          PRINT_ERROR("Solid color requires #RRGGBB.\n");
          return false;
        }
        patch.color_change = Postprocessor::ColorChange::Override;
        patch.color = {color.redF(),color.greenF(),color.blueF()};
      }
    }
    if (data.find_keyword("EDGES"))
    {
      patch.edges = data.find_keyword("ON");
    }
    double value = 0;
    if (data.get_value("opacity",value))
    {
      patch.opacity = value;
    }
    if (data.get_value("pointsize",value))
    {
      patch.point_size = value;
    }
    std::vector<Postprocessor::AppearanceTarget> targets;
    if (app->resolve_appearance_targets(source,category,names,targets,error) && app->change_dataset_appearance(targets,patch,error))
    {
      return true;
    }
  }
  catch (const std::exception& failure)
  {
    error = failure.what();
  }
  PRINT_ERROR("Cannot change appearance: %s\n",error.c_str());
  return false;
}
