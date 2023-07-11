#include "CoreConstraints.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreConstraints::CoreConstraints()
{}

CoreConstraints::~CoreConstraints()
{}

bool CoreConstraints::init()
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

bool CoreConstraints::update()
{ 
  return true;
}

bool CoreConstraints::reset()
{
  constraints_data.clear();
  rigidbody_constraint_data.clear();
  tie_constraint_data.clear();
  
  init();
  return true;
}

bool CoreConstraints::check_initialized()
{
  return is_initialized;
}

bool CoreConstraints::create_constraint(std::string constraint_type, std::vector<std::string> options)
{
  int constraint_id;
  int constraint_type_value;
  int constraint_last;
  int sub_constraint_id;
  int sub_constraint_last;

  if (constraint_type=="RIGIDBODY")
  {
    if (rigidbody_constraint_data.size()==0)
    {
      sub_constraint_id = 1;
    }
    else
    {
      sub_constraint_last = rigidbody_constraint_data.size() - 1;
      sub_constraint_id = std::stoi(rigidbody_constraint_data[sub_constraint_last][0]) + 1;
    }
    constraint_type_value = 1;
    this->add_rigidbody_constraint(std::to_string(sub_constraint_id), options[0], options[1], options[2], options[3]);
  } else if (constraint_type=="TIE")
  {
    if (tie_constraint_data.size()==0)
    {
      sub_constraint_id = 1;
    }
    else
    {
      sub_constraint_last = tie_constraint_data.size() - 1;
      sub_constraint_id = std::stoi(tie_constraint_data[sub_constraint_last][0]) + 1;
    }
    constraint_type_value = 2;
    this->add_tie_constraint(std::to_string(sub_constraint_id), options[0], options[1], options[2], options[3]);
  }

  if (constraints_data.size()==0)
  {
    constraint_id = 1;
  }
  else
  {
    constraint_last = constraints_data.size() - 1;
    constraint_id = constraints_data[constraint_last][0] + 1;
  }

  this->add_constraint(constraint_id,constraint_type_value,sub_constraint_id);
  return true;
}

bool CoreConstraints::modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  int constraint_type_value;
  if (constraint_type=="RIGIDBODY")
  {
    constraint_type_value = 1;
  }else if (constraint_type=="TIE")
  {
    constraint_type_value = 2;
  }

  int sub_constraint_data_id;
  int constraints_data_id = get_constraints_data_id_from_constraint_id(constraint_id);
  
  if (constraints_data_id == -1)
  {
    return false;
  } else {
    if ((constraints_data[constraints_data_id][1]==1) && (constraints_data[constraints_data_id][1]==constraint_type_value))
    {
      sub_constraint_data_id = get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(constraints_data[constraints_data_id][2]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          rigidbody_constraint_data[sub_constraint_data_id][i+1] = options[i];
        }
      }
    }else if ((constraints_data[constraints_data_id][1]==2) && (constraints_data[constraints_data_id][1]==constraint_type_value))
    {
      sub_constraint_data_id = get_tie_constraint_data_id_from_tie_constraint_id(constraints_data[constraints_data_id][2]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          tie_constraint_data[sub_constraint_data_id][i+1] = options[i];
        }
      }
    }
    return true;
  }
}

bool CoreConstraints::add_constraint(int constraint_id, int constraint_type, int constraint_type_id)
{
  std::vector<int> v = {constraint_id, constraint_type, constraint_type_id};
      
  constraints_data.push_back(v);

  return true;
}

bool CoreConstraints::add_rigidbody_constraint(std::string rigid_body_constraint_id, std::string entity_type, std::string type_id,std::string vertex_ref,std::string vertex_rot)
{
  std::vector<std::string> v = {rigid_body_constraint_id,entity_type,type_id,vertex_ref,vertex_rot};
      
  rigidbody_constraint_data.push_back(v);
  
  return true;
}

bool CoreConstraints::add_tie_constraint(std::string tie_constraint_id, std::string name, std::string master,std::string slave,std::string position_tolerance)
{
  std::vector<std::string> v = {tie_constraint_id,name,master,slave,position_tolerance};
      
  tie_constraint_data.push_back(v);

  return true;
}

bool CoreConstraints::delete_constraint(int constraint_id)
{
  int sub_constraint_data_id;
  int constraints_data_id = get_constraints_data_id_from_constraint_id(constraint_id);
  if (constraints_data_id == -1)
  {
    return false;
  } else {
    if (constraints_data[constraints_data_id][1]==1)
    {
      sub_constraint_data_id = get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(constraints_data[constraints_data_id][2]);
      rigidbody_constraint_data.erase(rigidbody_constraint_data.begin() + sub_constraint_data_id);  
    }else if (constraints_data[constraints_data_id][1]==2)
    {
      sub_constraint_data_id = get_tie_constraint_data_id_from_tie_constraint_id(constraints_data[constraints_data_id][2]);
      tie_constraint_data.erase(tie_constraint_data.begin() + sub_constraint_data_id);  
    }
    constraints_data.erase(constraints_data.begin() + constraints_data_id);
    return true;
  }
}

int CoreConstraints::get_constraints_data_id_from_constraint_id(int constraint_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < constraints_data.size(); i++)
  {
    if (constraints_data[i][0]==constraint_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreConstraints::get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(int rigidbody_constraint_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < rigidbody_constraint_data.size(); i++)
  {
    if (rigidbody_constraint_data[i][0]==std::to_string(rigidbody_constraint_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreConstraints::get_tie_constraint_data_id_from_tie_constraint_id(int tie_constraint_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < tie_constraint_data.size(); i++)
  {
    if (tie_constraint_data[i][0]==std::to_string(tie_constraint_id))
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreConstraints::get_node_id_from_vertex_id(int vertex_id)
{
  int node_id = CubitInterface::get_vertex_node(vertex_id);
  return node_id;
}

std::string CoreConstraints::get_constraint_export() // get a list of the CalculiX constraint exports
{
  std::vector<std::string> constraints_export_list;
  constraints_export_list.push_back("********************************** C O N S T R A I N T S ****************************");
  std::string str_temp;
  int sub_constraint_data_id;

  //loop over all constraints
  for (size_t i = 0; i < constraints_data.size(); i++)
  { 
    // RIGID BODY
    if (constraints_data[i][1] == 1)
    {
      sub_constraint_data_id = get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(constraints_data[i][2]);

      str_temp = "*RIGID BODY, ";
      
      if (rigidbody_constraint_data[sub_constraint_data_id][1]=="1")
      {
        str_temp.append("NSET=");
        str_temp.append(ccx_iface->get_nodeset_name(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][2])));
      } else if (rigidbody_constraint_data[sub_constraint_data_id][1]=="2")
      {
        str_temp.append("ELSET=");
        str_temp.append(ccx_iface->get_block_name(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][2])));
      }

      str_temp.append(", REF NODE=");
      str_temp.append(std::to_string(get_node_id_from_vertex_id(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][3]))));
      
      str_temp.append(", ROT NODE=");
      str_temp.append(std::to_string(get_node_id_from_vertex_id(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][4]))));
      constraints_export_list.push_back(str_temp);
    }
    // TIE
    if (constraints_data[i][1] == 2) 
    {
      sub_constraint_data_id = get_tie_constraint_data_id_from_tie_constraint_id(constraints_data[i][2]);
      
      str_temp = "*TIE, NAME=";
      str_temp.append(tie_constraint_data[sub_constraint_data_id][1]);
      
      if (tie_constraint_data[sub_constraint_data_id][4]!="")
      {
        str_temp.append(", POSITION TOLERANCE=");
        str_temp.append(tie_constraint_data[sub_constraint_data_id][4]);
      }

      constraints_export_list.push_back(str_temp);
      // second line
      str_temp = "";
      str_temp.append(ccx_iface->get_sideset_name(std::stoi(tie_constraint_data[sub_constraint_data_id][2])));
      str_temp.append(",");
      str_temp.append(ccx_iface->get_sideset_name(std::stoi(tie_constraint_data[sub_constraint_data_id][3])));
      constraints_export_list.push_back(str_temp);
    }
  }

  std::string constraint_export;

  for (size_t i = 0; i < constraints_export_list.size(); i++)
  {
    constraint_export.append(constraints_export_list[i] + "\n");
  }
  
  return constraint_export;
}

std::vector<int> CoreConstraints::get_rigidbody_vertex_list()
{
  std::vector<int> vertices;
  for (size_t i = 0; i < rigidbody_constraint_data.size(); i++)
  {
    vertices.push_back(std::stoi(rigidbody_constraint_data[i][3]));
    vertices.push_back(std::stoi(rigidbody_constraint_data[i][4]));
  }
  
  return vertices;
}


std::string CoreConstraints::print_data()
{
  std::string str_return;
  str_return = "\n CoreConstraints constraints_data: \n";
  str_return.append("constraint_id, constraint_type, constraint_type_id \n");

  for (size_t i = 0; i < constraints_data.size(); i++)
  {
    str_return.append(std::to_string(constraints_data[i][0]) + " " + std::to_string(constraints_data[i][1]) + " " + std::to_string(constraints_data[i][2]) + " \n");
  }

  str_return.append("\n CoreConstraints rigidbody_constraint_data: \n");
  str_return.append("rigidbody_constraint_id,entity_type,type_id,vertex \n");

  for (size_t i = 0; i < rigidbody_constraint_data.size(); i++)
  {
    str_return.append(rigidbody_constraint_data[i][0] + " " + rigidbody_constraint_data[i][1] + " " + rigidbody_constraint_data[i][2] + " " + rigidbody_constraint_data[i][3] + " \n");
  }

  str_return.append("\n CoreConstraints tie_constraint_data: \n");
  str_return.append("tie_constraint_id,name,master,slave,position tolerance \n");

  for (size_t i = 0; i < tie_constraint_data.size(); i++)
  {
    str_return.append(tie_constraint_data[i][0] + " " + tie_constraint_data[i][1] + " " + tie_constraint_data[i][2] + " " + tie_constraint_data[i][3] + " " + tie_constraint_data[i][4] + " \n");
  }
  
  return str_return;
}