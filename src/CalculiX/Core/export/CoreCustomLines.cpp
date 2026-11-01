#include "Core/export/CoreCustomLines.hpp"
#include <string>
#include <vector>
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
    customline_last = int(customlines_data.size()) - 1;
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
      return_int = int(i);
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

std::vector<std::vector<std::string>> CoreCustomLines::get_tree_data() const
{
  std::vector<std::vector<std::string>> tree_data;

  for (const auto& line : customlines_data)
  {
    std::string name = line[1] + " (" + line[2] + " " + line[3] + " " + line[4] + ")";
    tree_data.push_back({line[0], name});
  }

  return tree_data;
}

std::vector<int> CoreCustomLines::get_customline_ids() const
{
  std::vector<int> ids;
  ids.reserve(customlines_data.size());

  for (const auto& line : customlines_data)
  {
    ids.push_back(std::stoi(line[0]));
  }

  return ids;
}
