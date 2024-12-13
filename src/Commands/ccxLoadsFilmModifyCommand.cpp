#include "ccxLoadsFilmModifyCommand.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"

ccxLoadsFilmModifyCommand::ccxLoadsFilmModifyCommand()
{}

ccxLoadsFilmModifyCommand::~ccxLoadsFilmModifyCommand()
{}

std::vector<std::string> ccxLoadsFilmModifyCommand::get_syntax()
{
  std::vector<std::string> syntax_list;

  std::string syntax = "ccx ";
  syntax.append("modify film <value:label='film id',help='<film id>'>");
  syntax.append("[sideset <value:label='sideset id',help='<sideset id>'>] ");
  syntax.append("[temperature <value:label='temperature_value',help='<temperature_value>'>] ");
  syntax.append("[coefficient <value:label='coefficient_value',help='<coefficient_value>'>] ");
  syntax.append("[op {mod | new}] " );
  syntax.append("[amplitude <value:label='amplitude id',help='<amplitude id>'>] ");
  syntax.append("[timedelay <value:label='timedelay',help='<timedelay>'>] ");
  syntax.append("[filmamplitude <value:label='filmamplitude id',help='<filmamplitude id>'>] ");
  syntax.append("[filmtimedelay <value:label='filmtimedelay',help='<filmtimedelay>'>] ");
  syntax.append("[name <string:type='unquoted', number='1', label='name', help='<name>'>] " );
  
  syntax_list.push_back(syntax);
  
  return syntax_list;
}

std::vector<std::string> ccxLoadsFilmModifyCommand::get_syntax_help()
{
  std::vector<std::string> help(1);
  help[0] = "ccx "; 
  help[0].append("modify film <film id>");
  help[0].append("[sideset <sideset id>] ");
  help[0].append("[temperature <temperature_value>] ");
  help[0].append("[coefficient <coefficient_value>] ");
  help[0].append("[op {mod | new}] " );
  help[0].append("[amplitude <amplitude id>] ");
  help[0].append("[timedelay <timedelay>] ");
  help[0].append("[filmamplitude <filmamplitude id>] ");
  help[0].append("[filmtimedelay <filmtimedelay>] ");
  help[0].append("[name <name>] " );

  return help;
}

std::vector<std::string> ccxLoadsFilmModifyCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxLoadsFilmModifyCommand::execute(CubitCommandData &data)
{
  CalculiXCoreInterface ccx_iface;

  std::string output;
  std::vector<int> options_marker;
  std::vector<std::string> options;
  int film_id;
  std::string sideset_id;
  int sideset_id_value;
  int op_mode = 0;
  double temperature_value;
  std::string temperature;
  double coefficient_value;
  std::string coefficient;
  std::string amplitude_id;
  int amplitude_value;
  std::string timedelay;
  double timedelay_value;
  std::string filmamplitude_id;
  int filmamplitude_value;
  std::string filmtimedelay;
  double filmtimedelay_value;
  std::string name; 
  
  data.get_value("film id", film_id);
  
  if (data.find_keyword("OP")){
    if (data.find_keyword("MOD")){
      options_marker.push_back(1);
      options.push_back("0");
    }else if (data.find_keyword("NEW"))
    {
      options_marker.push_back(1);
      options.push_back("1");
    }
  }else{
      options_marker.push_back(0);
      options.push_back("0");
  }

  if (!data.get_value("amplitude id", amplitude_value))
  {
    amplitude_id = "-1";
    options_marker.push_back(0);
  }
  else
  {
    amplitude_id = std::to_string(amplitude_value);
    options_marker.push_back(1);
  }
  options.push_back(amplitude_id);

  if (!data.get_value("timedelay", timedelay_value))
  {
    timedelay = "";
    options_marker.push_back(0);
  }
  else
  {
    timedelay = std::to_string(timedelay_value);
    options_marker.push_back(1);
  }
  options.push_back(timedelay);
  
  if (!data.get_value("sideset id", sideset_id_value))
  {
    amplitude_id = "-1";
    options_marker.push_back(0);
  }
  else
  {
    sideset_id = std::to_string(sideset_id_value);
    options_marker.push_back(1);
  }
  options.push_back(sideset_id);

  if (!data.get_value("temperature_value", temperature_value))
  {
    temperature = "-1";
    options_marker.push_back(0);
  }
  else
  {
    temperature = ccx_iface.to_string_scientific(temperature_value);
    options_marker.push_back(1);
  }
  options.push_back(temperature);
  
  if (!data.get_value("coefficient_value", coefficient_value))
  {
    coefficient = "-1";
    options_marker.push_back(0);
  }
  else
  {
    coefficient = ccx_iface.to_string_scientific(coefficient_value);
    options_marker.push_back(1);
  }
  options.push_back(coefficient);

  if (!data.get_value("filmamplitude id", filmamplitude_value))
  {
    filmamplitude_id = "-1";
    options_marker.push_back(0);
  }
  else
  {
    filmamplitude_id = std::to_string(filmamplitude_value);
    options_marker.push_back(1);
  }
  options.push_back(filmamplitude_id);

  if (!data.get_value("filmtimedelay", filmtimedelay_value))
  {
    filmtimedelay = "";
    options_marker.push_back(0);
  }
  else
  {
    filmtimedelay = std::to_string(filmtimedelay_value);
    options_marker.push_back(1);
  }
  options.push_back(filmtimedelay);
  
  if (!data.get_string("name", name))
  {
    name = "";
    options_marker.push_back(0);
  }
  else
  {
    options_marker.push_back(1);
  }
  options.push_back(name);
  
  
  if (!ccx_iface.modify_loadsfilm(film_id ,options , options_marker))
  {
    output = "Failed!\n";
    PRINT_ERROR(output.c_str());
  }
  options.clear();
    
  return true;
}