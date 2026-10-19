#include "ccxTopologyCreateCommand.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"
#include "CoreTopologyStructs.hpp"

std::vector<std::string> ccxTopologyCreateCommand::get_syntax()
{
  return {
      "ccx create topology <value:label='topology_id'> "
      "[name <string:label='name'>] "
      "[method {density|level_set}] "
      "[objective {compliance|mass|displacement}] "
      "[volume_fraction <value:label='volume_fraction'>] "
      "[void_ratio <value:label='void_ratio'>] "
      "[iterations <value:label='maximum_iterations'>] "
      "[filter_radius <value:label='filter_radius'>] "
      "[move_limit <value:label='move_limit'>] "
      "[density_tolerance <value:label='density_tolerance'>] "
      "[compliance_tolerance <value:label='compliance_tolerance'>] "
      "[displacement_nodeset <value:label='displacement_nodeset'>] "
      "[displacement_component {x|y|z|magnitude}]"
  };
}

std::vector<std::string> ccxTopologyCreateCommand::get_syntax_help()
{
  return {
      "ccx create topology <id> [name <name>] "
      "[method {density|level_set}] "
      "[objective {compliance|mass|displacement}] "
      "[volume_fraction <value>] [void_ratio <value>] "
      "[iterations <count>] [filter_radius <length>] "
      "[move_limit <value>] [density_tolerance <value>] "
      "[compliance_tolerance <value>] "
      "[displacement_nodeset <id>] "
      "[displacement_component {x|y|z|magnitude}]"
  };
}

std::vector<std::string> ccxTopologyCreateCommand::get_help()
{
  return {"Create a named topology optimization case.", "displacement_nodeset enables displacement measurement; zero disables it.", "Displacement components use global coordinates. X/Y/Z report the maximum absolute component; magnitude reports the maximum vector norm."};
}

bool ccxTopologyCreateCommand::execute(CubitCommandData& data)
{
  CalculiXCoreInterface ccx_iface;

  TopologyOptions options;
  int topology_id = -1;

  data.get_value("topology_id", topology_id); 

  if (data.find_keyword("name"))
  {
    options.has_name = true;
    data.get_string("name", options.name);
  }

  if (data.find_keyword("method"))
  {
    options.has_method = true;

    if (data.find_keyword("density"))
      options.method = TopologyMethod::Density;
    else if (data.find_keyword("level_set"))
      options.method = TopologyMethod::LevelSet;
  }

  if (data.find_keyword("objective"))
  {
    options.has_objective = true;

    if (data.find_keyword("compliance"))
      options.objective = TopologyObjective::Compliance;
    else if (data.find_keyword("mass"))
      options.objective = TopologyObjective::Mass;
    else if (data.find_keyword("displacement"))
      options.objective = TopologyObjective::Displacement;
  }

  if (data.find_keyword("volume_fraction"))
  {
    options.has_volume_fraction = true;
    data.get_value("volume_fraction", options.volume_fraction);
  }
  
  if (data.find_keyword("void_ratio"))
  {
    options.has_void_ratio = true;
    data.get_value("void_ratio", options.void_ratio);
  }
  
  if (data.find_keyword("iterations"))
  {
    options.has_maximum_iterations = true;
    data.get_value("maximum_iterations", options.maximum_iterations);
  }

  if (data.find_keyword("filter_radius"))
  {
    options.has_filter_radius = true;
    data.get_value("filter_radius", options.filter_radius);
  }

  if (data.find_keyword("move_limit"))
  {
    options.has_move_limit = true;

    if (!data.get_value("move_limit", options.move_limit))
    {
      PRINT_ERROR("Missing topology move limit.\n");
      return false;
    }
  }

  if (data.find_keyword("density_tolerance"))
  {
    options.has_density_tolerance = true;

    if (!data.get_value("density_tolerance", options.density_tolerance))
    {
      PRINT_ERROR("Missing topology density tolerance.\n");
      return false;
    }
  }

  if (data.find_keyword("compliance_tolerance"))
  {
    options.has_compliance_tolerance = true;

    if (!data.get_value("compliance_tolerance", options.compliance_tolerance))
    {
      PRINT_ERROR("Missing topology compliance tolerance.\n");
      return false;
    }
  }

  if (data.find_keyword("displacement_nodeset"))
  {
    options.has_displacement_nodeset = true;

    if (!data.get_value("displacement_nodeset", options.displacement_nodeset))
    {
      PRINT_ERROR("Missing displacement nodeset ID.\n");
      return false;
    }
  }

  if (data.find_keyword("displacement_component"))
  {
    options.has_displacement_component = true;

    if (data.find_keyword("x"))
    {
      options.displacement_component = 1;
    }
    else if (data.find_keyword("y"))
    {
      options.displacement_component = 2;
    }
    else if (data.find_keyword("z"))
    {
      options.displacement_component = 3;
    }
    else if (data.find_keyword("magnitude"))
    {
      options.displacement_component = 0;
    }
    else
    {
      PRINT_ERROR("Missing displacement component.\n");
      return false;
    }
  }

  return ccx_iface.create_topology(topology_id, options);
}