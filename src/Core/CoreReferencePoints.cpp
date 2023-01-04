#include "CoreReferencePoints.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include <sstream>

CoreReferencePoints::CoreReferencePoints()
{}

CoreReferencePoints::~CoreReferencePoints()
{}

bool CoreReferencePoints::init()
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

bool CoreReferencePoints::update()
{ 
  return true;
}

bool CoreReferencePoints::reset()
{
  referencepoints_data.clear();
  
  init();
  return true;
}

bool CoreReferencePoints::check_initialized()
{
  return is_initialized;
}

bool CoreReferencePoints::update_on_export(int max_node_id)
{
  std::vector<int> vertices;
  std::vector<int> rigidbody_vertices;
  
  rot_max_node_id = max_node_id;

  rigidbody_vertices = ccx_iface->get_rigidbody_vertex_list();
  for (size_t i = 0; i < rigidbody_vertices.size(); i++)
  {
    vertices.push_back(rigidbody_vertices[i]);
  } 
  for (size_t i = 0; i < vertices.size(); i++)
  {
    this->create_referencepoint(vertices[i]);
  }
  return true;
}

bool CoreReferencePoints::create_referencepoint(int vertex_id)
{
  int referencepoints_data_id;
  referencepoints_data_id = get_referencepoint_data_id_from_vertex_id(vertex_id);
  int ref_node;
  int rot_node;

  if (referencepoints_data_id == -1)
  { 
    ref_node=this->get_node_id_from_vertex_id(vertex_id);
    rot_max_node_id = rot_max_node_id + 1;
    rot_node = rot_max_node_id;
    std::vector<int> v = {vertex_id,ref_node,rot_node};
    referencepoints_data.push_back(v);
    return true;
  } else {
    return false;
  }
}

bool CoreReferencePoints::delete_referencepoint(int vertex_id)
{
  int referencepoints_data_id;
  referencepoints_data_id = get_referencepoint_data_id_from_vertex_id(vertex_id);
  if (referencepoints_data_id == -1)
  {
    return false;
  } else {
    referencepoints_data.erase(referencepoints_data.begin() + referencepoints_data_id);
    return true;
  }
}

int CoreReferencePoints::get_node_id_from_vertex_id(int vertex_id)
{
  int node_id = CubitInterface::get_vertex_node(vertex_id);
  return node_id;
}

std::array<double,3> CoreReferencePoints::get_coords_from_node_id(int node_id)
{
  std::array<double,3> coords = CubitInterface::get_nodal_coordinates(node_id);
  return coords;
}

int CoreReferencePoints::get_referencepoint_data_id_from_vertex_id(int vertex_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < referencepoints_data.size(); i++)
  {
    if (referencepoints_data[i][0]==vertex_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreReferencePoints::get_ref_from_vertex_id(int vertex_id)
{
  int referencepoints_data_id;
  referencepoints_data_id = get_referencepoint_data_id_from_vertex_id(vertex_id);
  if (referencepoints_data_id == -1)
  {
    return -1;
  } else {
    int ref_node = referencepoints_data[referencepoints_data_id][1];
    return ref_node;
  }
}

int CoreReferencePoints::get_rot_from_vertex_id(int vertex_id)
{
  int referencepoints_data_id;
  referencepoints_data_id = get_referencepoint_data_id_from_vertex_id(vertex_id);
  if (referencepoints_data_id == -1)
  {
    return -1;
  } else {
    int rot_node = referencepoints_data[referencepoints_data_id][2];
    return rot_node;
  }
}  

std::string CoreReferencePoints::get_referencepoints_export()
{
  std::string output;
  std::array<double,3> coords;
  std::ostringstream streamObj;
  streamObj.precision(6);
  streamObj << std::scientific;  

  output = "";
  for (size_t i = 0; i < referencepoints_data.size(); i++)
  { 
    coords = get_coords_from_node_id(referencepoints_data[i][1]);
    output.append(std::to_string(referencepoints_data[i][2]) + ",  ");
    
    streamObj << coords[0];
    output.append(streamObj.str() + ",  ");
    streamObj.str("");
    streamObj.clear();

    streamObj << coords[1];
    output.append(streamObj.str() + ",  ");
    streamObj.str("");
    streamObj.clear();
    
    streamObj << coords[2];
    output.append(streamObj.str() + "\n");
    streamObj.str("");
    streamObj.clear();
    
  }
  
  return output;
}


std::string CoreReferencePoints::get_referencepoints_export_nodesets()
{
  std::string output;

  output = "";
  for (size_t i = 0; i < referencepoints_data.size(); i++)
  { 
    output.append("*NSET, NSET=REF_NODE_" + std::to_string(i) + "\n");
    output.append(std::to_string(referencepoints_data[i][1]) + "\n");
    output.append("*NSET, NSET=ROT_NODE_" + std::to_string(i) + "\n");
    output.append(std::to_string(referencepoints_data[i][2]) + "\n");
  }
  
  return output;
}


std::string CoreReferencePoints::print_data()
{
  std::string str_return;
  str_return = "\n CoreReferencePoints referencepoints_data: \n";
  str_return.append("vertex_id, ref_node_id, rot_node_id \n");

  for (size_t i = 0; i < referencepoints_data.size(); i++)
  {
    str_return.append(std::to_string(referencepoints_data[i][0]) + " " + std::to_string(referencepoints_data[i][1]) + " " + std::to_string(referencepoints_data[i][2]) + " \n");
  }

  str_return.append("\n CoreReferencePoints max_node_id: \n");
  str_return.append(std::to_string(rot_max_node_id) + "\n");
  
  return str_return;
}