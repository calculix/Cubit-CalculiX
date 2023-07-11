#include "CoreCustomLines.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

CoreCustomLines::CoreCustomLines()
{}

CoreCustomLines::~CoreCustomLines()
{}

bool CoreCustomLines::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    is_initialized = true;  
    return true;
  }
}

bool CoreCustomLines::update()
{ 
  return true;
}

bool CoreCustomLines::reset()
{
  customlines_data.clear();

  init();
  return true;
}

bool CoreCustomLines::check_initialized()
{
  return is_initialized;
}

bool CoreCustomLines::create_customline(std::vector<std::string> options)
{
  int customline_id;
  int customline_last;
  
  if (customlines_data.size()==0)
  {
    customline_id = 1;
  }
  else
  {
    customline_last = customlines_data.size() - 1;
    customline_id = std::stoi(customlines_data[customline_last][0]) + 1;
  }

  this->add_customline(customline_id, options[0], options[1], options[2], options[3], options[4]);
  return true;
}

bool CoreCustomLines::modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  int customlines_data_id = get_customlines_data_id_from_customline_id(customline_id);
  
  if (customlines_data_id == -1)
  {
    return false;
  } else {
    for (size_t i = 0; i < options.size(); i++)
    {
      if (options_marker[i]==1)
      {
        customlines_data[customlines_data_id][i+1] = options[i];
      }
    }
    return true;
  }
}

bool CoreCustomLines::add_customline(int customline_id, std::string name, std::string position, std::string keyword,std::string keyword_id, std::string customline)
{
  std::vector<std::string> v = {std::to_string(customline_id), name, position, keyword, keyword_id, customline};
  customlines_data.push_back(v);
  return true;
}

bool CoreCustomLines::delete_customline(int customline_id)
{
  int customlines_data_id = get_customlines_data_id_from_customline_id(customline_id);
  if (customlines_data_id == -1)
  {
    return false;
  } else {
    customlines_data.erase(customlines_data.begin() + customlines_data_id);
    return true;
  }
}

int CoreCustomLines::get_customlines_data_id_from_customline_id(int customline_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < customlines_data.size(); i++)
  {
    if (customlines_data[i][0]==std::to_string(customline_id))
    {
      return_int = i;
    }  
  }
  return return_int;
}


std::vector<std::string> CoreCustomLines::get_customline_data(std::string position, std::string keyword,int keyword_id)
{
  std::vector<std::string> tmp;
  for (size_t i = 0; i < customlines_data.size(); i++)
  {
    if ((customlines_data[i][2]==position) && (customlines_data[i][3]==keyword) && (customlines_data[i][4]==std::to_string(keyword_id)))
    {
      tmp.push_back(customlines_data[i][5]);
    }
  }
  return tmp;
}

std::string CoreCustomLines::print_data()
{
  std::string str_return;
  str_return = "\n CoreCustomLines customlines_data: \n";
  str_return.append("customline_id, name, position, keyword, keyword_id, customline \n");

  for (size_t i = 0; i < customlines_data.size(); i++)
  {
    str_return.append(customlines_data[i][0] + " " + customlines_data[i][1] + " " + customlines_data[i][2] + " " + customlines_data[i][3] + " " + customlines_data[i][4] + " " + customlines_data[i][5] + " \n");
  }

  return str_return;
}