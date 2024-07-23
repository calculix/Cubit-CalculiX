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
  syntax.append("draw step <value:label='step_id',help='<step_id>'>");
  syntax.append(" [size <value:label='size_value',help='<size_value>'>]");
  syntax.append(" [load");
  syntax.append(" [force <value:label='force_id',help='<force_id>'>...]");
  syntax.append(" [pressure <value:label='pressure_id',help='<pressure_id>'>...]");
  syntax.append(" [heatflux <value:label='heatflux_id',help='<heatflux_id>'>...]");
  syntax.append(" [gravity <value:label='gravity_id',help='<gravity_id>'>...]");
  syntax.append(" [centrifugal <value:label='centrifugal_id',help='<centrifugal_id>'>...]");
  syntax.append("]");
  syntax.append(" [bc ");
  syntax.append(" [displacement <value:label='displacement_id',help='<displacement_id>'>...]");
  syntax.append(" [temperature <value:label='temperature_id',help='<temperature_id>'>...]");
  syntax.append("]");
  syntax.append(" [orientation <value:label='orientation_id',help='<orientation_id>'>...]");
  syntax.append(" [load_all]");
  syntax.append(" [load_force_all]");
  syntax.append(" [load_pressure_all]");
  syntax.append(" [load_heatflux_all]");
  syntax.append(" [load_gravity_all]");
  syntax.append(" [load_centrifugal_all]");
  syntax.append(" [bc_all]");
  syntax.append(" [bc_displacement_all]");
  syntax.append(" [bc_temperature_all]");
  syntax.append(" [orientation_all]");
  
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxDrawStepCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx";
  help[0].append(" draw step <step_id>");
  help[0].append(" [size <size_value>]");
  help[0].append(" [load [force <force_id>...]");
  help[0].append(" [pressure <pressure_id>...]");
  help[0].append(" [heatflux <heatflux_id>...]");
  help[0].append(" [gravity <gravity_id>...]");
  help[0].append(" [centrifugal <centrifugal_id>...]]");
  help[0].append(" [bc [displacement <displacement_id>...]");
  help[0].append(" [temperature <temperature_id>...]]");
  help[0].append(" [orientation <orientation_id>...]");
  help[0].append(" [load_all]");
  help[0].append(" [load_force_all]");
  help[0].append(" [load_pressure_all]");
  help[0].append(" [load_heatflux_all]");
  help[0].append(" [load_gravity_all]");
  help[0].append(" [load_centrifugal_all]");
  help[0].append(" [bc_all]");
  help[0].append(" [bc_displacement_all]");
  help[0].append(" [bc_temperature_all]");
  help[0].append(" [orientation_all]");

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
  std::vector<int> displacement_id;
  std::vector<int> temperature_id;
  std::vector<int> orientation_id;
  
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
    }

    if (data.find_keyword("BC"))
    {
      data.get_values("displacement_id", displacement_id);
      data.get_values("temperature_id", temperature_id);
    }

    data.get_values("orientation_id", orientation_id);

    // get step data
    std::vector<std::vector<std::string>> step_loadforces_tree_data = ccx_iface.get_steps_loadsforces_tree_data(step_id);
    
    if ((force_id.size()==0)&&(pressure_id.size()==0)&&(heatflux_id.size()==0)&&(gravity_id.size()==0)&&(centrifugal_id.size()==0)&&(displacement_id.size()==0)&&(temperature_id.size()==0)&&(orientation_id.size()==0))
    {
     bool_draw_all = true;
    }
    
    if (data.find_keyword("LOAD_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_loads(size_value))
      {
        output = "Failed ccx draw loads!\n";
        PRINT_ERROR(output.c_str());
      }
    }
    
    if (data.find_keyword("BC_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_bcs(size_value))
      {
        output = "Failed ccx draw bcs!\n";
        PRINT_ERROR(output.c_str());
      }
    }
    if (data.find_keyword("ORIENTATION_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_orientations(size_value))
      {
        output = "Failed ccx draw orientations!\n";
        PRINT_ERROR(output.c_str());
      }
    }

    //pfusch
    if (data.find_keyword("LOAD_FORCE_ALL"))
    {
      bool_draw_all = false;
      force_id = convert_tree_data(step_loadforces_tree_data);
      if (!ccx_iface.draw_load_forces(size_value))
      {
        output = "Failed ccx draw forces!\n";
        PRINT_ERROR(output.c_str());
      }
    }

    
    if (data.find_keyword("LOAD_PRESSURE_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_load_pressures(size_value))
      {
        output = "Failed ccx draw pressures!\n";
        PRINT_ERROR(output.c_str());
      }
    }
    if (data.find_keyword("LOAD_HEATFLUX_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_load_heatfluxes(size_value))
      {
        output = "Failed ccx draw heatfluxes!\n";
        PRINT_ERROR(output.c_str());
      }
    }
    if (data.find_keyword("LOAD_GRAVITY_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_load_gravities(size_value))
      {
        output = "Failed ccx draw gravities!\n";
        PRINT_ERROR(output.c_str());
      }
    }
    if (data.find_keyword("LOAD_CENTRIFUGAL_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_load_centrifugals(size_value))
      {
        output = "Failed ccx draw centrifugals!\n";
        PRINT_ERROR(output.c_str());
      }
    }
    if (data.find_keyword("BC_DISPLACEMENT_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_bc_displacements(size_value))
      {
        output = "Failed ccx draw displacements!\n";
        PRINT_ERROR(output.c_str());
      }
    }
    if (data.find_keyword("BC_TEMPERATURE_ALL"))
    {
      bool_draw_all = false;
      if (!ccx_iface.draw_bc_temperatures(size_value))
      {
        output = "Failed ccx draw temperatures!\n";
        PRINT_ERROR(output.c_str());
      }
    }


    // filter
    force_id = get_ids_in_tree_data(force_id,step_loadforces_tree_data);



    if (!ccx_iface.draw_load_force(force_id,size_value))
    {
      bool_draw_all = false;
      output = "Failed ccx draw load force!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_pressure(pressure_id,size_value))
    {
      bool_draw_all = false;
      output = "Failed ccx draw load pressure!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_heatflux(heatflux_id,size_value))
    {
      bool_draw_all = false;
      output = "Failed ccx draw load heatflux!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_gravity(gravity_id,size_value))
    {
      bool_draw_all = false;
      output = "Failed ccx draw load gravity!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_load_centrifugal(centrifugal_id,size_value))
    {
      bool_draw_all = false;
      output = "Failed ccx draw load centrifugal!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_bc_displacement(displacement_id,size_value))
    {
      bool_draw_all = false;
      output = "Failed ccx draw bc displacement!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_bc_temperature(temperature_id,size_value))
    {
      bool_draw_all = false;
      output = "Failed ccx draw bc temperature!\n";
      PRINT_ERROR(output.c_str());
    }
    if (!ccx_iface.draw_orientation(orientation_id,size_value))
    {
      bool_draw_all = false;
      output = "Failed ccx draw orientation!\n";
      PRINT_ERROR(output.c_str());
    }
    
    if (bool_draw_all)
    {
      if (!ccx_iface.draw_all(size_value))
      {
        output = "Failed ccx draw!\n";
        PRINT_ERROR(output.c_str());
      } 
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