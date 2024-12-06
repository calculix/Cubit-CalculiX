#include "ccxDrawStepCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

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
  help[0].append(" [bc [displacement <displacement_id>...]");
  help[0].append(" [temperature <temperature_id>...]]");
  help[0].append(" [load_all]");
  help[0].append(" [load_force_all]");
  help[0].append(" [load_pressure_all]");
  help[0].append(" [load_heatflux_all]");
  help[0].append(" [load_gravity_all]");
  help[0].append(" [load_centrifugal_all]");
  help[0].append(" [load_trajectory_all]");
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
  CalculiXCoreInterface ccx_iface;

  std::string output;
  bool bool_draw_all = false;
  int step_id;
  double size_value;
  std::vector<int> force_id;
  std::vector<int> pressure_id;
  std::vector<int> heatflux_id;
  std::vector<int> gravity_id;
  std::vector<int> centrifugal_id;
  std::vector<int> trajectory_id;
  std::vector<int> displacement_id;
  std::vector<int> temperature_id;
  
  data.get_value("step_id", step_id);

  if (ccx_iface.check_step_exists(step_id))
  {
    if (!data.get_value("size_value", size_value))
    {
      std::array<double,10> vector_list;
      std::vector<int> ids = CubitInterface::parse_cubit_list("vertex", "all");
      if (ids.size()!=0)
      {
        vector_list = CubitInterface::get_total_bounding_box("vertex", ids);
      }else{
        std::vector<int> ids = CubitInterface::parse_cubit_list("node", "all");
        vector_list = CubitInterface::get_total_bounding_box("node", ids);
      }
      size_value = (vector_list[2]+vector_list[5]+vector_list[8])/3;
      if (size_value <= 0)
      {
        size_value = 1;
      }
    }

    if (data.find_keyword("LOAD"))
    {
      data.get_values("force_id", force_id);
      data.get_values("pressure_id", pressure_id);
      data.get_values("heatflux_id", heatflux_id);
      data.get_values("gravity_id", gravity_id);
      data.get_values("centrifugal_id", centrifugal_id);
      data.get_values("trajectory_id", trajectory_id);
    }

    if (data.find_keyword("BC"))
    {
      data.get_values("displacement_id", displacement_id);
      data.get_values("temperature_id", temperature_id);
    }

    // get step data
    std::vector<std::vector<std::string>> step_loadforces_tree_data = ccx_iface.get_steps_loadsforces_tree_data(step_id);
    std::vector<std::vector<std::string>> step_loadpressures_tree_data = ccx_iface.get_steps_loadspressures_tree_data(step_id);
    std::vector<std::vector<std::string>> step_loadheatfluxes_tree_data = ccx_iface.get_steps_loadsheatfluxes_tree_data(step_id);
    std::vector<std::vector<std::string>> step_loadgravities_tree_data = ccx_iface.get_steps_loadsgravity_tree_data(step_id);
    std::vector<std::vector<std::string>> step_loadcentrifugals_tree_data = ccx_iface.get_steps_loadscentrifugal_tree_data(step_id);
    std::vector<std::vector<std::string>> step_loadtrajectory_tree_data = ccx_iface.get_steps_loadstrajectory_tree_data(step_id);
    std::vector<std::vector<std::string>> step_bcdisplacements_tree_data = ccx_iface.get_steps_bcsdisplacements_tree_data(step_id);
    std::vector<std::vector<std::string>> step_bctemperatures_tree_data = ccx_iface.get_steps_bcstemperatures_tree_data(step_id);
    
    if ((force_id.size()==0)&&(pressure_id.size()==0)&&(heatflux_id.size()==0)&&(gravity_id.size()==0)&&(centrifugal_id.size()==0)&&(displacement_id.size()==0)&&(temperature_id.size()==0))
    {
      if((!data.find_keyword("LOAD_ALL"))&&(!data.find_keyword("BC_ALL"))&&(!data.find_keyword("LOAD_FORCE_ALL"))&&(!data.find_keyword("LOAD_PRESSURE_ALL"))&&(!data.find_keyword("LOAD_HEATFLUX_ALL"))&&(!data.find_keyword("LOAD_GRAVITY_ALL"))&&(!data.find_keyword("LOAD_CENTRIFUGAL_ALL"))&&(!data.find_keyword("LOAD_TRAJECTORY_ALL"))&&(!data.find_keyword("BC_DISPLACEMENT_ALL"))&&(!data.find_keyword("BC_TEMPERATURE_ALL")))
      {
        bool_draw_all = true;
      }
    }

    if (bool_draw_all)
    {
      force_id = convert_tree_data(step_loadforces_tree_data);
      pressure_id = convert_tree_data(step_loadpressures_tree_data);
      heatflux_id = convert_tree_data(step_loadheatfluxes_tree_data);
      gravity_id = convert_tree_data(step_loadgravities_tree_data);
      centrifugal_id = convert_tree_data(step_loadcentrifugals_tree_data);
      trajectory_id = convert_tree_data(step_loadtrajectory_tree_data);

      displacement_id = convert_tree_data(step_bcdisplacements_tree_data);
      temperature_id = convert_tree_data(step_bctemperatures_tree_data);
    }
    
    if (data.find_keyword("LOAD_ALL"))
    {
      force_id = convert_tree_data(step_loadforces_tree_data);
      pressure_id = convert_tree_data(step_loadpressures_tree_data);
      heatflux_id = convert_tree_data(step_loadheatfluxes_tree_data);
      gravity_id = convert_tree_data(step_loadgravities_tree_data);
      centrifugal_id = convert_tree_data(step_loadcentrifugals_tree_data);
      trajectory_id = convert_tree_data(step_loadtrajectory_tree_data);
    }
    
    if (data.find_keyword("BC_ALL"))
    {
      displacement_id = convert_tree_data(step_bcdisplacements_tree_data);
      temperature_id = convert_tree_data(step_bctemperatures_tree_data);
    }

    if (data.find_keyword("LOAD_FORCE_ALL"))
    {
      force_id = convert_tree_data(step_loadforces_tree_data);
    }

    if (data.find_keyword("LOAD_PRESSURE_ALL"))
    {
      pressure_id = convert_tree_data(step_loadpressures_tree_data);
    }

    if (data.find_keyword("LOAD_HEATFLUX_ALL"))
    {
      heatflux_id = convert_tree_data(step_loadheatfluxes_tree_data);
    }

    if (data.find_keyword("LOAD_GRAVITY_ALL"))
    {
      gravity_id = convert_tree_data(step_loadgravities_tree_data);
    }

    if (data.find_keyword("LOAD_CENTRIFUGAL_ALL"))
    {
      centrifugal_id = convert_tree_data(step_loadcentrifugals_tree_data);
    }
    
    if (data.find_keyword("LOAD_TRAJECTORY_ALL"))
    {
      trajectory_id = convert_tree_data(step_loadtrajectory_tree_data);
    }
    
    if (data.find_keyword("BC_DISPLACEMENT_ALL"))
    {
      displacement_id = convert_tree_data(step_bcdisplacements_tree_data);
    }

    if (data.find_keyword("BC_TEMPERATURE_ALL"))
    {
      temperature_id = convert_tree_data(step_bctemperatures_tree_data);
    }


    // filter
    force_id = get_ids_in_tree_data(force_id,step_loadforces_tree_data);
    pressure_id = get_ids_in_tree_data(pressure_id,step_loadpressures_tree_data);
    heatflux_id = get_ids_in_tree_data(heatflux_id,step_loadheatfluxes_tree_data);
    gravity_id = get_ids_in_tree_data(gravity_id,step_loadgravities_tree_data);
    centrifugal_id = get_ids_in_tree_data(centrifugal_id,step_loadcentrifugals_tree_data);
    trajectory_id = get_ids_in_tree_data(trajectory_id,step_loadtrajectory_tree_data);
    displacement_id = get_ids_in_tree_data(displacement_id,step_bcdisplacements_tree_data);
    temperature_id = get_ids_in_tree_data(temperature_id,step_bctemperatures_tree_data);

    if (!ccx_iface.draw_load_force(force_id,size_value))
    {
      output = "Failed ccx draw load force!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_pressure(pressure_id,size_value))
    {
      output = "Failed ccx draw load pressure!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_heatflux(heatflux_id,size_value))
    {
      output = "Failed ccx draw load heatflux!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_gravity(gravity_id,size_value))
    {
      output = "Failed ccx draw load gravity!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_centrifugal(centrifugal_id,size_value))
    {
      output = "Failed ccx draw load centrifugal!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_centrifugal(trajectory_id,size_value))
    {
      output = "Failed ccx draw load trajectory!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_bc_displacement(displacement_id,size_value))
    {
      output = "Failed ccx draw bc displacement!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_bc_temperature(temperature_id,size_value))
    {
      output = "Failed ccx draw bc temperature!\n";
      PRINT_ERROR(output.c_str());
    }

  }else{
    output = "Failed ccx draw!\n";
    output.append("Step with ID " + std::to_string(step_id) + " doesn't exist.\n");
    PRINT_ERROR(output.c_str());
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