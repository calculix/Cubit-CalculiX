#include "CoreSections.hpp"
#include "CubitInterface.hpp"

CoreSections::CoreSections()
{}

CoreSections::~CoreSections()
{}

bool CoreSections::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    is_initialized = true;  
    return true;
  }
}

bool CoreSections::update()
{ 
  return true;
}

bool CoreSections::reset()
{
  sections_data.clear();
  solid_section_data.clear();
  shell_section_data.clear();
  beam_section_data.clear();
  membrane_section_data.clear();
  init();
  return true;
}

bool CoreSections::check_initialized()
{
  return is_initialized;
}

bool CoreSections::add_section(int section_id, int section_type, int section_type_id)
{
  std::vector<int> v = {section_id, section_type, section_type_id};
      
  sections_data.push_back(v);

  return true;
}
bool CoreSections::delete_section(int section_id)
{
  int sections_data_id = get_sections_data_id_from_section_id(section_id);
  if (sections_data_id == -1)
  {
    return false;
  } else {
    sections_data.erase(sections_data.begin() + sections_data_id);
    return true;
  }
}
bool CoreSections::modify_section_id(int section_id, int new_section_id)
{
  int sections_data_id = get_sections_data_id_from_section_id(section_id);
  if (sections_data_id == -1)
  {
    return false;
  } else {
    sections_data[sections_data_id][0]=new_section_id;
    return true;
  }
}

int CoreSections::get_sections_data_id_from_section_id(int section_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < sections_data.size(); i++)
  {
    if (sections_data[i][0]==section_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}


std::string CoreSections::print_data()
{
  std::string str_return;
  str_return = "\n CoreSections sections_data: \n";
  str_return.append("section_id, section_type, section_type_id \n");

  for (size_t i = 0; i < sections_data.size(); i++)
  {
    str_return.append(std::to_string(sections_data[i][0]) + " " + std::to_string(sections_data[i][1]) + " " + std::to_string(sections_data[i][2]) + " \n");
  }
  
  return str_return;
}