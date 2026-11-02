#include "ccxDrawStepCommand.hpp"
#include <string>
#include <vector>
#include <array>
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
ccxDrawStepCommand::ccxDrawStepCommand()
{}

ccxDrawStepCommand::~ccxDrawStepCommand()
{}

std::vector<std::string> ccxDrawStepCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("draw step <value:label='step_id',help='<step_id>'>...");
  syntax.append(" [size <value:label='size_value',help='<size_value>'>]");
  syntax.append(" [load");
  syntax.append(" [force <value:label='force_id',help='<force_id>'>...]");
  syntax.append(" [pressure <value:label='pressure_id',help='<pressure_id>'>...]");
  syntax.append(" [heatflux <value:label='heatflux_id',help='<heatflux_id>'>...]");
  syntax.append(" [gravity <value:label='gravity_id',help='<gravity_id>'>...]");
  syntax.append(" [centrifugal <value:label='centrifugal_id',help='<centrifugal_id>'>...]");
  syntax.append(" [trajectory <value:label='trajectory_id',help='<trajectory_id>'>...]");
  syntax.append(" [film <value:label='film_id',help='<film_id>'>...]");
  syntax.append(" [radiation <value:label='radiation_id',help='<radiation_id>'>...]");
  syntax.append(" [surfacetraction <value:label='surfacetraction_id',help='<surfacetraction_id>'>...]");
  syntax.append(" [bodyheatflux <value:label='bodyheatflux_id',help='<bodyheatflux_id>'>...]");
  syntax.append("]");
  syntax.append(" [bc ");
  syntax.append(" [displacement <value:label='displacement_id',help='<displacement_id>'>...]");
  syntax.append(" [temperature <value:label='temperature_id',help='<temperature_id>'>...]");
  syntax.append("]");
  syntax.append(" [load_all]");
  syntax.append(" [load_force_all]");
  syntax.append(" [load_pressure_all]");
  syntax.append(" [load_heatflux_all]");
  syntax.append(" [load_gravity_all]");
  syntax.append(" [load_centrifugal_all]");
  syntax.append(" [load_trajectory_all]");
  syntax.append(" [load_film_all]");
  syntax.append(" [load_radiation_all]");
  syntax.append(" [load_surfacetraction_all]");
  syntax.append(" [load_bodyheatflux_all]");
  syntax.append(" [bc_all]");
  syntax.append(" [bc_displacement_all]");
  syntax.append(" [bc_temperature_all]");
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxDrawStepCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx";
  help[0].append(" draw step <step_id>...");
  help[0].append(" [size <size_value>]");
  help[0].append(" [load [force <force_id>...]");
  help[0].append(" [pressure <pressure_id>...]");
  help[0].append(" [heatflux <heatflux_id>...]");
  help[0].append(" [gravity <gravity_id>...]");
  help[0].append(" [centrifugal <centrifugal_id>...]]");
  help[0].append(" [trajectory <trajectory_id>...]]");
  help[0].append(" [film <film_id>...]]");
  help[0].append(" [radiation <radiation_id>...]]");
  help[0].append(" [surfacetraction <surfacetraction_id>...]]");
  help[0].append(" [bodyheatflux <bodyheatflux_id>...]]");
  help[0].append(" [bc [displacement <displacement_id>...]");
  help[0].append(" [temperature <temperature_id>...]]");
  help[0].append(" [load_all]");
  help[0].append(" [load_force_all]");
  help[0].append(" [load_pressure_all]");
  help[0].append(" [load_heatflux_all]");
  help[0].append(" [load_gravity_all]");
  help[0].append(" [load_centrifugal_all]");
  help[0].append(" [load_trajectory_all]");
  help[0].append(" [load_film_all]");
  help[0].append(" [load_radiation_all]");
  help[0].append(" [load_surfacetraction_all]");
  help[0].append(" [load_bodyheatflux_all]");
  help[0].append(" [bc_all]");
  help[0].append(" [bc_displacement_all]");
  help[0].append(" [bc_temperature_all]");

  return help;
}

std::vector<std::string> ccxDrawStepCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxDrawStepCommand::execute(CubitCommandData &data)
{
  CalculiXDrawInterface drawing;
  int step = -1;
  double size = drawing.default_size();
  std::vector<int> steps;
  data.get_values("step_id", steps);
  if (steps.size() != 1)
  {
    PRINT_ERROR("Draw one step at a time. Select a single step ID.\n");
    return false;
  }
  step = steps.front();
  data.get_value("size_value", size);
  std::vector<DrawFeature> features;
  if (data.find_keyword("LOAD_FORCE_ALL"))
  {
    features.push_back(DrawFeature::Force);
  }
  if (data.find_keyword("LOAD_PRESSURE_ALL"))
  {
    features.push_back(DrawFeature::Pressure);
  }
  if (data.find_keyword("LOAD_HEATFLUX_ALL"))
  {
    features.push_back(DrawFeature::Heatflux);
  }
  if (data.find_keyword("LOAD_GRAVITY_ALL"))
  {
    features.push_back(DrawFeature::Gravity);
  }
  if (data.find_keyword("LOAD_CENTRIFUGAL_ALL"))
  {
    features.push_back(DrawFeature::Centrifugal);
  }
  if (data.find_keyword("LOAD_TRAJECTORY_ALL"))
  {
    features.push_back(DrawFeature::Trajectory);
  }
  if (data.find_keyword("LOAD_FILM_ALL"))
  {
    features.push_back(DrawFeature::Film);
  }
  if (data.find_keyword("LOAD_RADIATION_ALL"))
  {
    features.push_back(DrawFeature::Radiation);
  }
  if (data.find_keyword("LOAD_SURFACETRACTION_ALL"))
  {
    features.push_back(DrawFeature::SurfaceTraction);
  }
  if (data.find_keyword("LOAD_BODYHEATFLUX_ALL"))
  {
    features.push_back(DrawFeature::BodyHeatflux);
  }
  if (data.find_keyword("BC_DISPLACEMENT_ALL"))
  {
    features.push_back(DrawFeature::Displacement);
  }
  if (data.find_keyword("BC_TEMPERATURE_ALL"))
  {
    features.push_back(DrawFeature::Temperature);
  }
  if (data.find_keyword("LOAD_ALL"))
  {
    features.insert(features.end(), {DrawFeature::Force, DrawFeature::Pressure, DrawFeature::Heatflux, DrawFeature::Gravity, DrawFeature::Centrifugal, DrawFeature::Trajectory, DrawFeature::Film, DrawFeature::Radiation, DrawFeature::SurfaceTraction, DrawFeature::BodyHeatflux});
  }
  if (data.find_keyword("BC_ALL"))
  {
    features.insert(features.end(), {DrawFeature::Displacement, DrawFeature::Temperature});
  }
  std::vector<DrawEntityKey> entities;
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_FORCE_ALL"))
  {
    std::vector<int> ids;
    data.get_values("force_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Force, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Force);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_PRESSURE_ALL"))
  {
    std::vector<int> ids;
    data.get_values("pressure_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Pressure, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Pressure);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_HEATFLUX_ALL"))
  {
    std::vector<int> ids;
    data.get_values("heatflux_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Heatflux, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Heatflux);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_GRAVITY_ALL"))
  {
    std::vector<int> ids;
    data.get_values("gravity_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Gravity, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Gravity);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_CENTRIFUGAL_ALL"))
  {
    std::vector<int> ids;
    data.get_values("centrifugal_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Centrifugal, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Centrifugal);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_TRAJECTORY_ALL"))
  {
    std::vector<int> ids;
    data.get_values("trajectory_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Trajectory, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Trajectory);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_FILM_ALL"))
  {
    std::vector<int> ids;
    data.get_values("film_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Film, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Film);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_RADIATION_ALL"))
  {
    std::vector<int> ids;
    data.get_values("radiation_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Radiation, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Radiation);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_SURFACETRACTION_ALL"))
  {
    std::vector<int> ids;
    data.get_values("surfacetraction_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::SurfaceTraction, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::SurfaceTraction);
    }
  }
  if (!data.find_keyword("LOAD_ALL") && !data.find_keyword("LOAD_BODYHEATFLUX_ALL"))
  {
    std::vector<int> ids;
    data.get_values("bodyheatflux_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::BodyHeatflux, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::BodyHeatflux);
    }
  }
  if (!data.find_keyword("BC_ALL") && !data.find_keyword("BC_DISPLACEMENT_ALL"))
  {
    std::vector<int> ids;
    data.get_values("displacement_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Displacement, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Displacement);
    }
  }
  if (!data.find_keyword("BC_ALL") && !data.find_keyword("BC_TEMPERATURE_ALL"))
  {
    std::vector<int> ids;
    data.get_values("temperature_id", ids);
    for (int id : ids)
    {
      entities.push_back({DrawFeature::Temperature, id});
    }
    if (!ids.empty())
    {
      features.push_back(DrawFeature::Temperature);
    }
  }
  std::string error;
  if (!drawing.draw_step(step, features, size, error, entities))
  {
    PRINT_ERROR("Step drawing failed: %s\n", error.c_str());
    return false;
  }
  return true;
}

std::vector<int> ccxDrawStepCommand::get_ids_in_tree_data(std::vector<int> in_ids,std::vector<std::vector<std::string>> tree_data)
{
  std::vector<int> ids;

  for (size_t i = 0; i < tree_data.size(); i++)
  {
    for (size_t ii = 0; ii < in_ids.size(); ii++)
    {
      if (std::stoi(tree_data[i][0])==in_ids[ii])
      {
        ids.push_back(in_ids[ii]);
      }
    }
  }
  return ids;
}

std::vector<int> ccxDrawStepCommand::convert_tree_data(std::vector<std::vector<std::string>> tree_data)
{
  std::vector<int> ids;

  for (size_t i = 0; i < tree_data.size(); i++)
  {
    ids.push_back(std::stoi(tree_data[i][0]));
  }

  return ids;
}
