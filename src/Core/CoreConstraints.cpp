#include "CoreConstraints.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"
#include "StopWatch.hpp"

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
  equation_constraint_data.clear();
  equation_data.clear();
  equation_group_constraint_data.clear();
  equation_group_data.clear();

  init();
  return true;
}

bool CoreConstraints::check_initialized()
{
  return is_initialized;
}

bool CoreConstraints::create_constraint(std::string constraint_type, std::vector<std::string> options, std::vector<std::vector<double>> options2)
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
      sub_constraint_last = int(rigidbody_constraint_data.size()) - 1;
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
      sub_constraint_last = int(tie_constraint_data.size()) - 1;
      sub_constraint_id = std::stoi(tie_constraint_data[sub_constraint_last][0]) + 1;
    }
    constraint_type_value = 2;
    this->add_tie_constraint(std::to_string(sub_constraint_id), options[0], options[1], options[2], options[3]);
  } else if (constraint_type=="EQUATION")
  {
    if (equation_constraint_data.size()==0)
    {
      sub_constraint_id = 1;
    }
    else
    {
      sub_constraint_last = int(equation_constraint_data.size()) - 1;
      sub_constraint_id = std::stoi(equation_constraint_data[sub_constraint_last][0]) + 1;
    }
    constraint_type_value = 3;
    this->add_equation_constraint(std::to_string(sub_constraint_id), options[0]);
    for (size_t i = 0; i < options2.size(); i++)
    {
      this->add_equation(double(sub_constraint_id), options2[i][0], options2[i][1], options2[i][2]);
    }
  } else if (constraint_type=="EQUATIONGROUP")
  {
    if (equation_group_constraint_data.size()==0)
    {
      sub_constraint_id = 1;
    }
    else
    {
      sub_constraint_last = int(equation_group_constraint_data.size()) - 1;
      sub_constraint_id = std::stoi(equation_group_constraint_data[sub_constraint_last][0]) + 1;
    }
    constraint_type_value = 4;
    this->add_equation_group_constraint(std::to_string(sub_constraint_id), options[0]);
    for (size_t i = 0; i < options2.size(); i++)
    {
      this->add_equation_group(double(sub_constraint_id), options2[i][0], options2[i][1], options2[i][2]);
    }
  }

  if (constraints_data.size()==0)
  {
    constraint_id = 1;
  }
  else
  {
    constraint_last = int(constraints_data.size()) - 1;
    constraint_id = constraints_data[constraint_last][0] + 1;
  }

  this->add_constraint(constraint_id,constraint_type_value,sub_constraint_id);
  return true;
}

bool CoreConstraints::modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker,std::vector<std::vector<double>> options2)
{
  int constraint_type_value;
  if (constraint_type=="RIGIDBODY")
  {
    constraint_type_value = 1;
  }else if (constraint_type=="TIE")
  {
    constraint_type_value = 2;
  }else if (constraint_type=="EQUATION")
  {
    constraint_type_value = 3;
  }

  int sub_constraint_data_id;
  int constraints_data_id = get_constraints_data_id_from_constraint_id(constraint_id);
  std::vector<int> sub_data_ids;

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
    else if ((constraints_data[constraints_data_id][1]==3) && (constraints_data[constraints_data_id][1]==constraint_type_value))
    {
      sub_constraint_data_id = get_equation_constraint_data_id_from_equation_constraint_id(constraints_data[constraints_data_id][2]);

      for (size_t i = 0; i < options.size(); i++)
      {
        if (options_marker[i]==1)
        {
          equation_constraint_data[sub_constraint_data_id][i+1] = options[i];
        }
      }
      sub_data_ids = get_equation_data_ids_from_equation_constraint_id(std::stoi(equation_constraint_data[sub_constraint_data_id][0]));
      if (options2.size()!=0)
      {
        if (options2.size()==sub_data_ids.size())
        {
          for (size_t i = 0; i < options2.size(); i++)
          {
            equation_data[sub_data_ids[i]][0] = std::stod(equation_constraint_data[sub_constraint_data_id][0]);
            equation_data[sub_data_ids[i]][1] = options2[i][0];
            equation_data[sub_data_ids[i]][2] = options2[i][1];
            equation_data[sub_data_ids[i]][2] = options2[i][2];
          }
        }else{
          // first delete and then make a push back
          // delete from back to begin so that we don't have to care about mismatching id's
          for (size_t i = sub_data_ids.size(); i > 0; i--)
          {
            equation_data.erase(equation_data.begin() + sub_data_ids[i-1]);
          }
          
          for (size_t i = 0; i < options2.size(); i++)
          {
            add_equation(std::stod(equation_constraint_data[sub_constraint_data_id][0]),options2[i][0],options2[i][1],options2[i][2]);
          }
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

bool CoreConstraints::add_equation_constraint(std::string equation_constraint_id, std::string name)
{
  std::vector<std::string> v = {equation_constraint_id,name};
      
  equation_constraint_data.push_back(v);

  return true;
}

bool CoreConstraints::add_equation(double equation_constraint_id, double node_id, double dof, double coefficient)
{
  std::vector<double> v = {equation_constraint_id,node_id,dof,coefficient};
      
  equation_data.push_back(v);

  return true;
}

bool CoreConstraints::add_equation_group_constraint(std::string equation_constraint_id, std::string name)
{
  std::vector<std::string> v = {equation_constraint_id,name};
      
  equation_group_constraint_data.push_back(v);

  return true;
}

bool CoreConstraints::add_equation_group(double equation_constraint_id, double node_id_1, double node_id_2, double dof)
{
  std::vector<double> v = {equation_constraint_id,node_id_1,node_id_2,dof};
      
  equation_group_data.push_back(v);

  return true;
}

bool CoreConstraints::delete_constraint(int constraint_id)
{
  int sub_constraint_data_id;
  std::vector<int> sub_data_ids;
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
    }else if (constraints_data[constraints_data_id][1]==3)
    {
      sub_data_ids = get_equation_data_ids_from_equation_constraint_id(constraints_data[constraints_data_id][2]);
      for (size_t i = sub_data_ids.size(); i > 0; i--)
      {
        equation_data.erase(equation_data.begin() + sub_data_ids[i-1]);
      }
      sub_constraint_data_id = get_equation_constraint_data_id_from_equation_constraint_id(constraints_data[constraints_data_id][2]);
      equation_constraint_data.erase(equation_constraint_data.begin() + sub_constraint_data_id);  
    }else if (constraints_data[constraints_data_id][1]==4)
    {
      sub_data_ids = get_equation_group_data_ids_from_equation_group_constraint_id(constraints_data[constraints_data_id][2]);
      for (size_t i = sub_data_ids.size(); i > 0; i--)
      {
        equation_group_data.erase(equation_group_data.begin() + sub_data_ids[i-1]);
      }
      sub_constraint_data_id = get_equation_group_constraint_data_id_from_equation_group_constraint_id(constraints_data[constraints_data_id][2]);
      equation_group_constraint_data.erase(equation_group_constraint_data.begin() + sub_constraint_data_id);  
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
        return_int = int(i);
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
        return_int = int(i);
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
        return_int = int(i);
    }  
  }
  return return_int;
}

int CoreConstraints::get_equation_constraint_data_id_from_equation_constraint_id(int equation_constraint_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < equation_constraint_data.size(); i++)
  {
    if (equation_constraint_data[i][0]==std::to_string(equation_constraint_id))
    {
      return_int = int(i);
    }  
  }
  return return_int;
}

int CoreConstraints::get_equation_group_constraint_data_id_from_equation_group_constraint_id(int equation_constraint_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < equation_group_constraint_data.size(); i++)
  {
    if (equation_group_constraint_data[i][0]==std::to_string(equation_constraint_id))
    {
      return_int = int(i);
    }  
  }
  return return_int;
}

std::vector<int> CoreConstraints::get_equation_data_ids_from_equation_constraint_id(int equation_constraint_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < equation_data.size(); i++)
  {
    if (equation_data[i][0]==double(equation_constraint_id))
    {
      return_int.push_back(int(i));
    }  
  }
  return return_int;
}

std::vector<int> CoreConstraints::get_equation_group_data_ids_from_equation_group_constraint_id(int equation_constraint_id)
{ 
  std::vector<int> return_int;
  for (size_t i = 0; i < equation_group_data.size(); i++)
  {
    if (equation_group_data[i][0]==double(equation_constraint_id))
    {
      return_int.push_back(int(i));
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
    // CUSTOMLINE START
    std::vector<std::string> customline = ccx_iface->get_customline_data("BEFORE","CONSTRAINT",constraints_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      constraints_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END

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
    // EQUATION
    if (constraints_data[i][1] == 3) 
    {
      sub_constraint_data_id = get_equation_constraint_data_id_from_equation_constraint_id(constraints_data[i][2]);
      
      str_temp = "*EQUATION";
      constraints_export_list.push_back(str_temp);
      
      std::vector<int> sub_data_ids = get_equation_data_ids_from_equation_constraint_id(constraints_data[i][2]);
      
      // number of terms
      str_temp = std::to_string(sub_data_ids.size());
      constraints_export_list.push_back(str_temp);
      
      // equations
      int counter = 0;
      str_temp = "";
      for (size_t i = 0; i < sub_data_ids.size(); i++)
      {
        counter = counter + 1;
      
        str_temp.append(std::to_string(int(equation_data[sub_data_ids[i]][1])));
        str_temp.append(",");
        str_temp.append(std::to_string(int(equation_data[sub_data_ids[i]][2])));
        str_temp.append(",");
        str_temp.append(ccx_iface->to_string_scientific(equation_data[sub_data_ids[i]][3]));
        if (counter == 4)
        {
          str_temp.append("\n");
          counter = 0;
        }else{
          if (i != sub_data_ids.size()-1)
          {
            str_temp.append(",");
          }
        }
      }
      constraints_export_list.push_back(str_temp);
    }
    // EQUATIONGROUP
    if (constraints_data[i][1] == 4) 
    {
      sub_constraint_data_id = get_equation_group_constraint_data_id_from_equation_group_constraint_id(constraints_data[i][2]);
      
      std::vector<int> sub_data_ids = get_equation_group_data_ids_from_equation_group_constraint_id(constraints_data[i][2]);
           
      // equations
      for (size_t i = 0; i < sub_data_ids.size(); i++)
      {
        str_temp = "*EQUATION";
        constraints_export_list.push_back(str_temp);
        // number of terms
        str_temp = "2";
        constraints_export_list.push_back(str_temp);
        str_temp = "";  
        str_temp.append(std::to_string(int(equation_group_data[sub_data_ids[i]][1])));
        str_temp.append(",");
        str_temp.append(std::to_string(int(equation_group_data[sub_data_ids[i]][3])));
        str_temp.append(",");
        str_temp.append(ccx_iface->to_string_scientific(1.));
        str_temp.append(",");
        str_temp.append(std::to_string(int(equation_group_data[sub_data_ids[i]][2])));
        str_temp.append(",");
        str_temp.append(std::to_string(int(equation_group_data[sub_data_ids[i]][3])));
        str_temp.append(",");
        str_temp.append(ccx_iface->to_string_scientific(-1.));
        constraints_export_list.push_back(str_temp);
      }
    }
    // CUSTOMLINE START
    customline = ccx_iface->get_customline_data("AFTER","CONSTRAINT",constraints_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      constraints_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END
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

bool CoreConstraints::create_constraint_equation_from_coincident_nodes(std::string name, int group_id, double tolerance, bool dof_1, bool dof_2, bool dof_3)
{
  StopWatch watch;
  watch.tick("Starting to create Equations from coincident Nodes in Group " + std::to_string(group_id));

  //get nodes from group
  std::vector<int> group_node_ids = CubitInterface::parse_cubit_list("node","all in group " + std::to_string(group_id));
  
  if (group_node_ids.size() == 0)
  {
    std::string log;
    log = "No Nodes found in Group "+ std::to_string(group_id) + "\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  std::vector<std::vector<double>> group_node_coordinates;

  // get coordinates for each node
  for (size_t i = 0; i < group_node_ids.size(); i++)
  {
    std::array<double, 3> coords = CubitInterface::get_nodal_coordinates(int(group_node_ids[i]));
    if (coords.size() > 0)
    {
      std::vector<double> data;            
      data.push_back(coords[0]);
      data.push_back(coords[1]);
      data.push_back(coords[2]);
      group_node_coordinates.push_back(data);
    }else{
      std::string log;
      log = "No Coordinates found for Node "+ std::to_string(int(group_node_ids[i])) + ".\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
  }
  watch.tick("collected coordinates");
  
  /*
  //check each node against each other and create pairs
  std::vector<std::vector<int>> node_pairs;
  int ic = 0;
  for (size_t i = 0; i < group_node_ids.size()-1; i++)
  {
    ++ic;
    if (ic==100)
    {
      watch.tick("loop " + std::to_string(i+1));
      ic=0;
    }

    for (size_t ii = i+1; ii < group_node_ids.size(); ii++)
    {
      std::vector<double> vec(3);
      vec[0] = group_node_coordinates[ii][0] - group_node_coordinates[i][0];
      vec[1] = group_node_coordinates[ii][1] - group_node_coordinates[i][1];
      vec[2] = group_node_coordinates[ii][2] - group_node_coordinates[i][2];
      double distance = std::sqrt(vec[0]*vec[0] + vec[1]*vec[1] + vec[2]*vec[2]);
      if (distance <= tolerance)
      {
        node_pairs.push_back({group_node_ids[i],group_node_ids[ii]});
      }
    }
  }
  */

  //check each node against each other and create pairs
  std::vector<std::vector<int>> node_pairs;
  // create tmp vectors
  std::vector<int> tmp_group_node_ids_x;
  std::vector<int> tmp_group_node_ids_y;
  std::vector<int> tmp_group_node_ids_z;
  std::vector<double> tmp_group_node_coordinates_x;
  std::vector<double> tmp_group_node_coordinates_y;
  std::vector<double> tmp_group_node_coordinates_z;
  for (size_t i = 0; i < group_node_ids.size(); i++)
  {
    tmp_group_node_ids_x.push_back(group_node_ids[i]);
    tmp_group_node_ids_y.push_back(group_node_ids[i]);
    tmp_group_node_ids_z.push_back(group_node_ids[i]);
    tmp_group_node_coordinates_x.push_back(group_node_coordinates[i][0]);
    tmp_group_node_coordinates_y.push_back(group_node_coordinates[i][1]);
    tmp_group_node_coordinates_z.push_back(group_node_coordinates[i][2]);
  }

  // sorting
  auto px = sort_permutation(tmp_group_node_coordinates_x);
  this->apply_permutation(tmp_group_node_coordinates_x, px);
  this->apply_permutation(tmp_group_node_ids_x, px);
  auto py = sort_permutation(tmp_group_node_coordinates_y);
  this->apply_permutation(tmp_group_node_coordinates_y, py);
  this->apply_permutation(tmp_group_node_ids_y, py);
  auto pz = sort_permutation(tmp_group_node_coordinates_z);
  this->apply_permutation(tmp_group_node_coordinates_z, pz);
  this->apply_permutation(tmp_group_node_ids_z, pz);

  watch.tick("created searchable datasets");
  
  int ic = 0;
  std::vector<int> processed_group_node_ids;
  for (size_t i = 0; i < group_node_ids.size()-1; i++)
  {
    int x_min = -1;
    int x_max = -1;
    int y_min = -1;
    int y_max = -1;
    int z_min = -1;
    int z_max = -1;
    
    // check range
    if (std::binary_search(tmp_group_node_coordinates_x.begin(), tmp_group_node_coordinates_x.end(), group_node_coordinates[i][0]))
    {
      auto lower = std::lower_bound(tmp_group_node_coordinates_x.begin(), tmp_group_node_coordinates_x.end(), group_node_coordinates[i][0] - tolerance);
      x_min = int(lower - tmp_group_node_coordinates_x.begin());
    }
    if (std::binary_search(tmp_group_node_coordinates_x.begin(), tmp_group_node_coordinates_x.end(), group_node_coordinates[i][0]))
    {
      auto upper = std::upper_bound(tmp_group_node_coordinates_x.begin(), tmp_group_node_coordinates_x.end(), group_node_coordinates[i][0] + tolerance);
      x_max = int(upper - tmp_group_node_coordinates_x.begin());
    }
    if (std::binary_search(tmp_group_node_coordinates_y.begin(), tmp_group_node_coordinates_y.end(), group_node_coordinates[i][1]))
    {
      auto lower = std::lower_bound(tmp_group_node_coordinates_y.begin(), tmp_group_node_coordinates_y.end(), group_node_coordinates[i][1] - tolerance);
      y_min = int(lower - tmp_group_node_coordinates_y.begin());
    }
    if (std::binary_search(tmp_group_node_coordinates_y.begin(), tmp_group_node_coordinates_y.end(), group_node_coordinates[i][1]))
    {
      auto upper = std::upper_bound(tmp_group_node_coordinates_y.begin(), tmp_group_node_coordinates_y.end(), group_node_coordinates[i][1] + tolerance);
      y_max = int(upper - tmp_group_node_coordinates_y.begin());
    }
    if (std::binary_search(tmp_group_node_coordinates_z.begin(), tmp_group_node_coordinates_z.end(), group_node_coordinates[i][2]))
    {
      auto lower = std::lower_bound(tmp_group_node_coordinates_z.begin(), tmp_group_node_coordinates_z.end(), group_node_coordinates[i][2] - tolerance);
      z_min = int(lower - tmp_group_node_coordinates_z.begin());
    }
    if (std::binary_search(tmp_group_node_coordinates_z.begin(), tmp_group_node_coordinates_z.end(), group_node_coordinates[i][2]))
    {
      auto upper = std::upper_bound(tmp_group_node_coordinates_z.begin(), tmp_group_node_coordinates_z.end(), group_node_coordinates[i][2] + tolerance);
      z_max = int(upper - tmp_group_node_coordinates_z.begin());
    }

    // check if a range can be established
    std::vector<int> x_range;
    std::vector<int> y_range;
    std::vector<int> z_range;
    if ((x_min!=-1) && (x_max!=-1) &&
        (y_min!=-1) && (y_max!=-1) &&
        (z_min!=-1) && (z_max!=-1))
    {
      /*
      for (size_t ii = x_min; ii < x_max; ii++)
      {
        x_range.push_back(ii);
      }
      for (size_t ii = y_min; ii < y_max; ii++)
      {
        y_range.push_back(ii);
      }
      for (size_t ii = z_min; ii < z_max; ii++)
      {
        z_range.push_back(ii);
      }
      */
      // check for node pairs
      for (size_t ii = x_min; ii < x_max; ii++)
      {
        // if node id is the same as the group node id then skip
        if (group_node_ids[i]!=tmp_group_node_ids_x[ii])
        {
          // if node already processed in previous loop then skip
          if (!std::binary_search(processed_group_node_ids.begin(), processed_group_node_ids.end(), tmp_group_node_ids_x[ii]))
          {
            // check id in y
            for (size_t iii = y_min; iii < y_max; iii++) 
            {
              if (tmp_group_node_ids_x[ii]==tmp_group_node_ids_y[iii])
              { 
                // check id in z
                for (size_t iv = z_min; iv < z_max; iv++)
                {
                  if (tmp_group_node_ids_x[ii]==tmp_group_node_ids_z[iv])
                  {
                    //add node pair
                    node_pairs.push_back({group_node_ids[i],tmp_group_node_ids_x[ii]});
                  }
                }
              }
            }
          }
        }   
      }
    }

    processed_group_node_ids.push_back(group_node_ids[i]);
    auto p = sort_permutation(processed_group_node_ids);
    this->apply_permutation(processed_group_node_ids, p);

    ++ic;
    if (ic==1000)
    {
      watch.tick("processed " + std::to_string(i+1) + " from "  + std::to_string(group_node_ids.size()) + " nodes");
      ic=0;
    }
    
  }

  watch.tick("Found " + std::to_string(node_pairs.size()) + " Node Pairs");
  
  /*
  for (size_t i = 0; i < node_pairs.size(); i++)
  {
    std::string log;
    log = "Pair "+ std::to_string(int(i+1)) + " " + std::to_string(int(node_pairs[i][0])) + "-" + std::to_string(int(node_pairs[i][1])) + ".\n";
    PRINT_INFO("%s", log.c_str());
  }
  */

  std::vector<std::string> options;
  options.push_back(name);
  std::vector<std::vector<double>> options2;

  for (size_t i = 0; i < node_pairs.size(); i++)
  {
    if (dof_1)
    {
      options2.push_back({double(node_pairs[i][0]),double(node_pairs[i][1]),1.});
    }
    if (dof_2)
    {
      options2.push_back({double(node_pairs[i][0]),double(node_pairs[i][1]),2.});
    }
    if (dof_3)
    {
      options2.push_back({double(node_pairs[i][0]),double(node_pairs[i][1]),3.});
    } 
  }
  if (node_pairs.size()>0)
  {
    this->create_constraint("EQUATIONGROUP",options,options2);
  }
  watch.tick("Created " + std::to_string(options2.size()) + " Equations");

  return true;
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
  
  str_return.append("\n CoreConstraints equation_constraint_data: \n");
  str_return.append("equation_constraint_id,name\n");

  for (size_t i = 0; i < equation_constraint_data.size(); i++)
  {
    str_return.append(equation_constraint_data[i][0] + " " + equation_constraint_data[i][1] + " \n");
  }

  str_return.append("\n CoreConstraints equation_data: \n");
  str_return.append("equation_constraint_id,node_id,dof,coefficient\n");

  for (size_t i = 0; i < equation_data.size(); i++)
  {
    str_return.append(std::to_string(equation_data[i][0]) + " " + std::to_string(equation_data[i][1]) + " " + std::to_string(equation_data[i][2]) + " " + std::to_string(equation_data[i][3]) + " \n");
  }

  str_return.append("\n CoreConstraints equation_group_constraint_data: \n");
  str_return.append("equation_constraint_id,name\n");

  for (size_t i = 0; i < equation_group_constraint_data.size(); i++)
  {
    str_return.append(equation_group_constraint_data[i][0] + " " + equation_group_constraint_data[i][1] + " \n");
  }

  str_return.append("\n CoreConstraints equation_group_data: \n");
  str_return.append("equation_constraint_id,node_id_1,node_id_2,dof\n");

  for (size_t i = 0; i < equation_group_data.size(); i++)
  {
    str_return.append(std::to_string(equation_group_data[i][0]) + " " + std::to_string(equation_group_data[i][1]) + " " + std::to_string(equation_group_data[i][2]) + " " + std::to_string(equation_group_data[i][3]) + " \n");
  }

  return str_return;
}



//sorting of vectors
template <typename T> 
std::vector<std::size_t> CoreConstraints::sort_permutation(
    const std::vector<T>& vec)
{
    std::vector<std::size_t> p(vec.size());
    std::iota(p.begin(), p.end(), 0);
    std::sort(p.begin(), p.end(),
        [&](std::size_t i, std::size_t j){ return vec[i] < vec[j]; });

    return p;
}

template <typename T> 
void CoreConstraints::apply_permutation(
    std::vector<T>& vec,
    const std::vector<std::size_t>& p)
{
    std::vector<bool> done(vec.size());
    for (std::size_t i = 0; i < vec.size(); ++i)
    {
        if (done[i])
        {
            continue;
        }
        done[i] = true;
        std::size_t prev_j = i;
        std::size_t j = p[i];
        while (i != j)
        {
            std::swap(vec[prev_j], vec[j]);
            done[j] = true;
            prev_j = j;
            j = p[j];
        }
    }
}