#include "CoreSurfaceInteractions.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

CoreSurfaceInteractions::CoreSurfaceInteractions()
{}

CoreSurfaceInteractions::~CoreSurfaceInteractions()
{}

bool CoreSurfaceInteractions::init()
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

bool CoreSurfaceInteractions::update()
{ 
  return true;
}

bool CoreSurfaceInteractions::reset()
{
  constraints_data.clear();
  rigidbody_constraint_data.clear();
  tie_constraint_data.clear();
  
  init();
  return true;
}

bool CoreSurfaceInteractions::check_initialized()
{
  return is_initialized;
}

bool CoreSurfaceInteractions::create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  int surfaceinteraction_id;
  int surfacebehavior_type_value;
  int surfaceinteraction_last;
  int sub_surfacebehavior_id;
  int sub_surfacebehavior_last;

  if (constraint_type=="EXPONENTIAL")
  {
    if (exponential_surfacebehavior_data.size()==0)
    {
      sub_surfacebehavior_id = 1;
    }
    else
    {
      sub_surfacebehavior_last = exponential_surfacebehavior_data.size() - 1;
      sub_surfacebehavior_id = std::stoi(exponential_surfacebehavior_data[sub_constraint_last][0]) + 1;
    }
    surfacebehavior_type_value = 1;
    this->add_exponential_surfacebehavior(std::to_string(sub_surfacebehavior_id), options[1], options[2]);
  } else if (constraint_type=="LINEAR")
  {
    if (linear_surfacebehavior_data.size()==0)
    {
      sub_surfacebehavior_id = 1;
    }
    else
    {
      sub_surfacebehavior_last = linear_surfacebehavior_data.size() - 1;
      sub_surfacebehavior_id = std::stoi(linear_surfacebehavior_data[sub_constraint_last][0]) + 1;
    }
    surfacebehavior_type_value = 2;
    this->add_linear_surfacebehavior(std::to_string(sub_surfacebehavior_id), options[1], options[2], options[3]);
  } else if (constraint_type=="TABULAR")
  {
    if (tabular_surfacebehavior_data.size()==0)
    {
      sub_surfacebehavior_id = 1;
    }
    else
    {
      sub_surfacebehavior_last = tabular_surfacebehavior_data.size() - 1;
      sub_surfacebehavior_id = std::stoi(tabular_surfacebehavior_data[sub_constraint_last][0]) + 1;
    }
    surfacebehavior_type_value = 3;
    for (size_t i = 0; i < options2.size(); i++)
    {
      this->add_tabular_surfacebehavior(std::to_string(sub_surfacebehavior_id), options2[i][0], options2[i][1]);
    }
  } else if (constraint_type=="TIED")
  {
    if (tabular_surfacebehavior_data.size()==0)
    {
      sub_surfacebehavior_id = 1;
    }
    else
    {
      sub_surfacebehavior_last = tied_surfacebehavior_data.size() - 1;
      sub_surfacebehavior_id = std::stoi(tied_surfacebehavior_data[sub_constraint_last][0]) + 1;
    }
    surfacebehavior_type_value = 4;
    this->add_tied_surfacebehavior(std::to_string(sub_surfacebehavior_id), options[1]);
  } else if (constraint_type=="HARD")
  {
    sub_surfacebehavior_id = -1;
    surfacebehavior_type_value = 5;
  }

  if (surfaceinteractions_data.size()==0)
  {
    surfaceinteraction_id = 1;
  }
  else
  {
    surfaceinteraction_last = surfaceinteractions_data.size() - 1;
    surfaceinteraction_id = surfaceinteractions_data[surfaceinteraction_last][0] + 1;
  }

  //this->add_surfaceinteraction(surfaceinteraction_id,surfaceinteraction_name,surfacebehavior_type,surfacebehavior_type_id,gap_conductance_id,gap_heat_generation_id,friction_id);
  this->add_surfaceinteraction(surfaceinteraction_id,options[0],surfacebehavior_type_value,sub_surfacebehavior_id,-1,-1,-1);
  return true;
}

bool CoreSurfaceInteractions::modify_constraint(std::string modify_type,int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  int modify_type_value;
  if (modify_type=="EXPONENTIAL")
  {
    modify_type_value = 1;
  }else if (modify_type=="LINEAR")
  {
    modify_type_value = 2;
  }else if (modify_type=="TABULAR")
  {
    modify_type_value = 3;
  }else if (modify_type=="TIED")
  {
    modify_type_value = 4;
  }else if (modify_type=="HARD")
  {
    modify_type_value = 5;
  }else if (modify_type=="GAPCONDUCTANCE")
  {
    modify_type_value = 6;
  }else if (modify_type=="GAPHEATGENERATION")
  {
    modify_type_value = 7;
  }else if (modify_type=="FRICTION")
  {
    modify_type_value = 8;
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

bool CoreSurfaceInteractions::add_constraint(int constraint_id, int constraint_type, int constraint_type_id)
{
  std::vector<int> v = {constraint_id, constraint_type, constraint_type_id};
      
  constraints_data.push_back(v);

  return true;
}

bool CoreSurfaceInteractions::add_rigidbody_constraint(std::string rigid_body_constraint_id, std::string entity_type, std::string type_id,std::string vertex)
{
  std::vector<std::string> v = {rigid_body_constraint_id,entity_type,type_id,vertex};
      
  rigidbody_constraint_data.push_back(v);
  
  return true;
}

bool CoreSurfaceInteractions::add_tie_constraint(std::string tie_constraint_id, std::string name, std::string master,std::string slave,std::string position_tolerance)
{
  std::vector<std::string> v = {tie_constraint_id,name,master,slave,position_tolerance};
      
  tie_constraint_data.push_back(v);

  return true;
}

bool CoreSurfaceInteractions::delete_constraint(int constraint_id)
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

int CoreSurfaceInteractions::get_constraints_data_id_from_constraint_id(int constraint_id)
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

int CoreSurfaceInteractions::get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(int rigidbody_constraint_id)
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

int CoreSurfaceInteractions::get_tie_constraint_data_id_from_tie_constraint_id(int tie_constraint_id)
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

std::string CoreSurfaceInteractions::get_constraint_export() // get a list of the CalculiX constraint exports
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
      str_temp.append(std::to_string(ccx_iface->referencepoints_get_ref_from_vertex_id(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][3]))));
      
      str_temp.append(", ROT NODE=");
      str_temp.append(std::to_string(ccx_iface->referencepoints_get_rot_from_vertex_id(std::stoi(rigidbody_constraint_data[sub_constraint_data_id][3]))));
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

std::vector<int> CoreSurfaceInteractions::get_rigidbody_vertex_list()
{
  std::vector<int> vertices;
  for (size_t i = 0; i < rigidbody_constraint_data.size(); i++)
  {
    vertices.push_back(std::stoi(rigidbody_constraint_data[i][3]));
  }
  
  return vertices;
}


std::string CoreSurfaceInteractions::print_data()
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