#include "CoreReferencePoints.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"

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

bool CoreReferencePoints::update_on_export()
{
  std::vector<int> vertices;
  std::vector<int> rigidbody_vertices;
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
  ref_node=1;
  rot_node=2;
  
  if (referencepoints_data_id == -1)
  {
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

std::string CoreReferencePoints::get_referencepoints_export()
{
  return "rp nodes";
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
 
  return str_return;
}