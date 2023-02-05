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

bool CoreSurfaceInteractions::modify_surfaceinteraction(std::string modify_type,int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  int modify_type_value;
  if (modify_type=="NAME")
  {
    modify_type_value = 0;
  }else if (modify_type=="EXPONENTIAL")
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

  int sub_data_id;
  std::vector<int> sub_data_ids;
  int surfaceinteractions_data_id = get_surfaceinteractions_data_id_from_surfaceinteraction_id(surfaceinteraction_id);
  
  if (surfaceinteractions_data_id == -1)
  {
    return false;
  } else {
    if (modify_type_value == 0) // modify name
    {
      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          surfaceinteraction_name_data[surfaceinteractions_data[surfaceinteractions_data_id][1]] = options[i];
        }
      }
    }else if ((surfaceinteractions_data[surfaceinteractions_data_id][2]==1) && (surfaceinteractions_data[surfaceinteractions_data_id][2]==modify_type_value))
    {
      sub_data_id = get_exponential_surfacebehavior_data_id_from_exponential_surfacebehavior_id(surfaceinteractions_data[surfaceinteractions_data_id][3]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          exponential_surfacebehavior_data[sub_data_id][i+1] = options[i];
        }
      }
    }else if ((surfaceinteractions_data[surfaceinteractions_data_id][2]==2) && (surfaceinteractions_data[surfaceinteractions_data_id][2]==modify_type_value))
    {
      sub_data_id = get_linear_surfacebehavior_data_id_from_linear_surfacebehavior_id(surfaceinteractions_data[surfaceinteractions_data_id][3]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          linear_surfacebehavior_data[sub_data_id][i+1] = options[i];
        }
      }
    }else if ((surfaceinteractions_data[surfaceinteractions_data_id][2]==3) && (surfaceinteractions_data[surfaceinteractions_data_id][2]==modify_type_value))
    {
      sub_data_ids = get_tabular_surfacebehavior_data_ids_from_tabular_surfacebehavior_id(surfaceinteractions_data[surfaceinteractions_data_id][3]);

      if (options2.size()==sub_data_ids.size())
      {
        for (size_t i = 0; i < options2.size(); i++)
        {
          tabular_surfacebehavior_data[sub_data_ids[i]][0] = options2[i][0];
          tabular_surfacebehavior_data[sub_data_ids[i]][1] = options2[i][1];
          tabular_surfacebehavior_data[sub_data_ids[i]][2] = options2[i][2];
        }
      }else{
        // first delete and then make a push back
        // delete from back to begin so that we don't have to care about mismatching id's
        for (size_t i = sub_data_ids.size()-1; i > -1; i--)
        {
          tabular_surfacebehavior_data.erase(sub_data_ids[i]);
        }
        for (size_t i = 0; i < options2.size(); i++)
        {
          add_tabular_surfacebehavior({options2[i][0],options2[i][1],options2[i][2]});
        }
      }
    }else if ((surfaceinteractions_data[surfaceinteractions_data_id][2]==4) && (surfaceinteractions_data[surfaceinteractions_data_id][2]==modify_type_value))
    {
      sub_data_id = get_tied_surfacebehavior_data_id_from_tied_surfacebehavior_id(surfaceinteractions_data[surfaceinteractions_data_id][3]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          tied_surfacebehavior_data[sub_data_id][i+1] = options[i];
        }
      }
    } // modify type 5 not needed to implement, right now...
    else if (modify_type_value == 6) // gap conductance
    {
      // check if already set, if not create
      sub_data_id = get_gap_conductance_data_id_from_gap_conductance_id(surfaceinteractions_data[surfaceinteractions_data_id][4]);
      if (sub_data_id == -1)
      { 
        if (gap_conductance_data.size()!=0)
        {
          sub_data_id = 1;
          surfaceinteractions_data[surfaceinteractions_data_id][4] = sub_data_id;
        }else{
          sub_data_id = gap_conductance_data[gap_conductance_data.size()-1][0] + 1
        }
        add_gap_conductance(sub_data_id, options[0], options[1], options[2]);
      }else{
        for (size_t i = 0; i < options.size(); i++)
        {
          if (options_marker[i]==1)
          {
            gap_conductance_data[sub_data_id][i+1] = options[i];
          }
        }
      }
    }else if (modify_type_value == 7) // gap_heat_generation
    {
      // check if already set, if not create
      sub_data_id = get_gap_heat_generation_data_id_from_gap_heat_generation_id(surfaceinteractions_data[surfaceinteractions_data_id][5]);
      if (sub_data_id == -1)
      { 
        if (gap_heat_generation_data.size()!=0)
        {
          sub_data_id = 1;
          surfaceinteractions_data[surfaceinteractions_data_id][5] = sub_data_id;
        }else{
          sub_data_id = gap_heat_generation_data[gap_heat_generation_data.size()-1][0] + 1
        }
        add_gap_heat_generation(sub_data_id, options[0], options[1], options[2]);
      }else{
        for (size_t i = 0; i < options.size(); i++)
        {
          if (options_marker[i]==1)
          {
            gap_heat_generation_data[sub_data_id][i+1] = options[i];
          }
        }
      }
    }else if (modify_type_value == 8) // friction
    {
      // check if already set, if not create
      sub_data_id = get_friction_data_id_from_friction_id(surfaceinteractions_data[surfaceinteractions_data_id][6]);
      if (sub_data_id == -1)
      { 
        if (friction_data.size()!=0)
        {
          sub_data_id = 1;
          surfaceinteractions_data[surfaceinteractions_data_id][6] = sub_data_id;
        }else{
          sub_data_id = gap_heat_generation_data[gap_heat_generation_data.size()-1][0] + 1
        }
        add_friction(sub_data_id, options[0], options[1]);
      }else{
        for (size_t i = 0; i < options.size(); i++)
        {
          if (options_marker[i]==1)
          {
            friction_data[sub_data_id][i+1] = options[i];
          }
        }
      }
    }
    return true;
  }
}

bool CoreSurfaceInteractions::delete_surfaceinteraction(int surfaceinteraction_id)
{
  int sub_data_id;
  std::vector<int> sub_data_ids;
  int surfaceinteractions_data_id = get_surfaceinteractions_data_id_from_surfaceinteraction_id(surfaceinteraction_id);
  if (surfaceinteractions_data_id == -1)
  {
    return false;
  } else {
    // surface name
    surfaceinteraction_name_data.erase(surfaceinteractions_data[surfaceinteraction_id][1]);
    // surface behavior
    if (surfaceinteractions_data[surfaceinteractions_data_id][2]==1)
    {
      sub_data_id = get_exponential_surfacebehavior_data_id_from_exponential_surfacebehavior_id(surfaceinteractions_data[surfaceinteractions_data_id][3]);
      exponential_surfacebehavior_data.erase(exponential_surfacebehavior_data.begin() + sub_data_id);  
    }else if (surfaceinteractions_data[surfaceinteractions_data_id][2]==2)
    {
      sub_data_id = get_linear_surfacebehavior_data_id_from_linear_surfacebehavior_id(surfaceinteractions_data[surfaceinteractions_data_id][3]);
      linear_surfacebehavior_data.erase(linear_surfacebehavior_data.begin() + sub_data_id);  
    }else if (surfaceinteractions_data[surfaceinteractions_data_id][2]==3)
    {
      sub_data_ids = get_tabular_surfacebehavior_data_ids_from_tabular_surfacebehavior_id(surfaceinteractions_data[surfaceinteractions_data_id][3]);
      for (size_t i = sub_data_ids.size()-1; i > -1; i--)
      {
        tabular_surfacebehavior_data.erase(sub_data_ids[i]);
      } 
    }else if (surfaceinteractions_data[surfaceinteractions_data_id][2]==4)
    {
      sub_data_id = get_tied_surfacebehavior_data_id_from_tied_surfacebehavior_id(surfaceinteractions_data[surfaceinteractions_data_id][3]);
      tied_surfacebehavior_data.erase(tied_surfacebehavior_data.begin() + sub_data_id);  
    } 
    // skip HARD
    // gap conductance
    if (surfaceinteractions_data[surfaceinteractions_data_id][4]!=-1)
    {
      sub_data_id = get_gap_conductance_data_id_from_gap_conductance_id(surfaceinteractions_data[surfaceinteractions_data_id][4]);
      gap_conductance_data.erase(gap_conductance_data.begin() + sub_data_id);  
    }
    // gap heat generation
    if (surfaceinteractions_data[surfaceinteractions_data_id][5]!=-1)
    {
      sub_data_id = get_gap_heat_generation_data_id_from_gap_heat_generation_id(surfaceinteractions_data[surfaceinteractions_data_id][5]);
      gap_heat_generation_data.erase(gap_heat_generation_data.begin() + sub_data_id);  
    }
    // friction
    if (surfaceinteractions_data[surfaceinteractions_data_id][6]!=-1)
    {
      sub_data_id = get_friction_data_id_from_friction_id(surfaceinteractions_data[surfaceinteractions_data_id][6]);
      friction_data.erase(friction_data.begin() + sub_data_id);  
    }

    surfaceinteractions_data.erase(surfaceinteractions_data.begin() + surfaceinteractions_data_id);
    return true;
  }
}

bool CoreSurfaceInteractions::add_surfaceinteraction(int surfaceinteraction_id, std::string surfaceinteraction_name, int surfacebehavior_type, int surfacebehavior_type_id, int gap_conductance_id, int gap_heat_generation_id, int friction_id)
{
  surfaceinteraction_name_data.push_back(surfaceinteraction_name);

  std::vector<int> v = {surfaceinteraction_id,surfaceinteraction_name_data[surfaceinteraction_name_data.size()-1],surfacebehavior_type,surfacebehavior_type_id,gap_conductance_id,gap_heat_generation_id,friction_id};
      
  surfaceinteractions_data.push_back(v);

  return true;
}

bool CoreSurfaceInteractions::add_exponential_surfacebehavior(std::string exponential_surfacebehavior_id, std::string constant_1, std::string constant_2)
{
  std::vector<std::string> v = {exponential_surfacebehavior_id, constant_1, constant_2};
      
  exponential_surfacebehavior_data.push_back(v);
  
  return true;
}

bool CoreSurfaceInteractions::add_linear_surfacebehavior(std::string linear_surfacebehavior_id, std::string constant_1, std::string constant_2, std::string constant_3)
{
  std::vector<std::string> v = {linear_surfacebehavior_id, constant_1, constant_2, constant_3};
      
  linear_surfacebehavior_data.push_back(v);
  
  return true;
}

bool CoreSurfaceInteractions::add_tabular_surfacebehavior(std::string tabular_surfacebehavior_id, std::string constant_1, std::string constant_2)
{
  std::vector<std::string> v = {tabular_surfacebehavior_id, constant_1, constant_2};
      
  tabular_surfacebehavior_data.push_back(v);
  
  return true;
}

bool CoreSurfaceInteractions::add_tied_surfacebehavior(std::string tied_surfacebehavior_id, std::string constant_1)
{
  std::vector<std::string> v = {tied_surfacebehavior_id, constant_1};
      
  tied_surfacebehavior_data.push_back(v);
  
  return true;
}

bool CoreSurfaceInteractions::add_gap_conductance(std::string gap_conductance_id, std::string constant_1, std::string constant_2, std::string constant_3)
{
  std::vector<std::string> v = {gap_conductance_id, constant_1, constant_2, constant_3};
      
  gap_conductance_data.push_back(v);
  
  return true;
}

bool CoreSurfaceInteractions::add_gap_heat_generation(std::string gap_heat_generation_id, std::string constant_1, std::string constant_2, std::string constant_3)
{
  std::vector<std::string> v = {gap_heat_generation_id, constant_1, constant_2, constant_3};
      
  gap_heat_generation_data.push_back(v);
  
  return true;
}

bool CoreSurfaceInteractions::add_friction(std::string friction_id, std::string constant_1, std::string constant_2)
{
  std::vector<std::string> v = {gap_conductance_id, constant_1, constant_2};
      
  friction_data.push_back(v);
  
  return true;
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