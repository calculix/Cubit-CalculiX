#include "ccxStepRemoveLoadCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxStepRemoveLoadCommand::ccxStepRemoveLoadCommand()
{}

ccxStepRemoveLoadCommand::~ccxStepRemoveLoadCommand()
{}

std::vector<std::string> ccxStepRemoveLoadCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("step <value:label='step id',help='<step id>'> ");
  syntax.append("remove load ");
  syntax.append("[force <value:label='force id',help='<force id>'>...] " );
  syntax.append("[pressure <value:label='pressure id',help='<pressure id>'>...] " );
  syntax.append("[heatflux <value:label='heatflux id',help='<heatflux id>'>...] " );
  syntax.append("[gravity <value:label='gravity id',help='<gravity id>'>...] " );
  syntax.append("[centrifugal <value:label='centrifugal id',help='<centrifugal id>'>...] " );
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxStepRemoveLoadCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx step <step id> remove load [force <force id>...] [pressure <pressure id>...] [heatflux <heatflux id>...] "; 
  help[0].append("[gravity <gravity id>...] ");
  help[0].append("[centrifugal <centrifugal id>...] ");

  return help;
}

std::vector<std::string> ccxStepRemoveLoadCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxStepRemoveLoadCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  
  int step_id;
  std::vector<int> force_ids;
  std::vector<int> pressure_ids;
  std::vector<int> heatflux_ids;
  std::vector<int> gravity_ids;
  std::vector<int> centrifugal_ids;

  data.get_value("step id", step_id);

  data.get_values("force id", force_ids);
  data.get_values("pressure id", pressure_ids);
  data.get_values("heatflux id", heatflux_ids);
  data.get_values("gravity id", gravity_ids);
  data.get_values("centrifugal id", centrifugal_ids);
   
  if (!ccx_iface.step_remove_loads(step_id, 1, force_ids))
  {
    output = "Failed removing Force!\n";
    PRINT_ERROR(output.c_str());
  }
  if (!ccx_iface.step_remove_loads(step_id, 2, pressure_ids))
  {
    output = "Failed removing Pressure!\n";
    PRINT_ERROR(output.c_str());
  }
  if (!ccx_iface.step_remove_loads(step_id, 3, heatflux_ids))
  {
    output = "Failed removing Heatflux!\n";
    PRINT_ERROR(output.c_str());
  }
  if (!ccx_iface.step_remove_loads(step_id, 4, gravity_ids))
  {
    output = "Failed removing Gravity!\n";
    PRINT_ERROR(output.c_str());
  }
  if (!ccx_iface.step_remove_loads(step_id, 5, centrifugal_ids))
  {
    output = "Failed removing Centrifugal!\n";
    PRINT_ERROR(output.c_str());
  }
  return true;
}