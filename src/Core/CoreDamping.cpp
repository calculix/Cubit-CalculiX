#include "CoreDamping.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

CoreDamping::CoreDamping()
{}

CoreDamping::~CoreDamping()
{}

bool CoreDamping::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    damping_data.push_back("");
    damping_data.push_back("");
    
    is_initialized = true;  
    return true;
  }
}

bool CoreDamping::update()
{ 
  return true;
}

bool CoreDamping::reset()
{
  damping_data.clear();
  damping_data.push_back("");
  damping_data.push_back("");
    
  init();
  return true;
}

bool CoreDamping::check_initialized()
{
  return is_initialized;
}

bool CoreDamping::modify_damping(std::vector<std::string> options, std::vector<int> options_marker)
{
  for (size_t i = 0; i < options.size(); i++)
  {
    if (options_marker[i]==1)
    {
      damping_data[i] = options[i];
    }
  }
  return true; 
}

bool CoreDamping::delete_damping(bool delete_alpha, bool delete_beta)
{
  if (delete_alpha)
  {
    damping_data[0] = "";
  } 
  if (delete_beta)
  {
    damping_data[1] = "";
  }

  return true;
}

std::string CoreDamping::get_damping_export_data()
{
  std::string str_temp = "";
  
  if ((damping_data[0]!="")||(damping_data[1]!=""))
  {
    str_temp = "*DAMPING";
  }
  if (damping_data[0]!="")
  {
    str_temp.append(",ALPHA=" + damping_data[0]);
  }
  if (damping_data[1]!="")
  {
    str_temp.append(",BETA=" + damping_data[1]);
  }
  str_temp.append("\n");
  
  return str_temp;
}

std::string CoreDamping::print_data()
{
  std::string str_return;
  str_return = "\n CoreDamping damping_data: \n";
  str_return.append("alpha, beta \n");

  str_return.append(damping_data[0] + " " + damping_data[1] + " \n");

  return str_return;
}