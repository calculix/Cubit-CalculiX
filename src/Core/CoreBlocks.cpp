#include "CoreBlocks.hpp"
#include "CubitInterface.hpp"

CoreBlocks::CoreBlocks()
{
  // define the ccx element types
  // volume
  ccx_element_types.push_back("C3D4");
  //ccx_element_types.push_back("C3D6");
  ccx_element_types.push_back("C3D8");
  ccx_element_types.push_back("C3D8R");
  ccx_element_types.push_back("C3D8I");
  ccx_element_types.push_back("C3D10");
  //ccx_element_types.push_back("C3D10T");
  //ccx_element_types.push_back("C3D15");
  ccx_element_types.push_back("C3D20");
  ccx_element_types.push_back("C3D20R");
  //shell
  ccx_element_types.push_back("S3");
  ccx_element_types.push_back("S4");
  ccx_element_types.push_back("S4R");
  ccx_element_types.push_back("S6");
  ccx_element_types.push_back("S8");
  ccx_element_types.push_back("S8R");
  //membrane
  /*
  ccx_element_types.push_back("M3D3");
  ccx_element_types.push_back("M3D4");
  ccx_element_types.push_back("M3D4R");
  ccx_element_types.push_back("M3D6");
  ccx_element_types.push_back("M3D8");
  ccx_element_types.push_back("M3D8R");
  */
  // plane stress
  ccx_element_types.push_back("CPS3");
  ccx_element_types.push_back("CPS4");
  ccx_element_types.push_back("CPS4R");
  ccx_element_types.push_back("CPS6");
  ccx_element_types.push_back("CPS8");
  ccx_element_types.push_back("CPS8R");
  // plane strain
  ccx_element_types.push_back("CPE3");
  ccx_element_types.push_back("CPE4");
  ccx_element_types.push_back("CPE4R");
  ccx_element_types.push_back("CPE6");
  ccx_element_types.push_back("CPE8");
  ccx_element_types.push_back("CPE8R");
  // axisymetric elements
  ccx_element_types.push_back("CAX3");
  ccx_element_types.push_back("CAX4");
  ccx_element_types.push_back("CAX4R");
  ccx_element_types.push_back("CAX6");
  ccx_element_types.push_back("CAX8");
  ccx_element_types.push_back("CAX8R");
  // beam elements
  ccx_element_types.push_back("B21");
  ccx_element_types.push_back("B31");
  ccx_element_types.push_back("B31R");
  ccx_element_types.push_back("B32");
  ccx_element_types.push_back("B32R");
  // truss elements
  ccx_element_types.push_back("T2D2");
  ccx_element_types.push_back("T3D2");
  ccx_element_types.push_back("T3D3");
  // special elements
  //ccx_element_types.push_back("SPRINGA");
}

CoreBlocks::~CoreBlocks()
{}

bool CoreBlocks::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    is_initialized = true;  
    return true;
  }
}

bool CoreBlocks::update()
{ 
  // Get the list of blocks
  std::vector<int> block_ids;
  int block_id;
  block_ids = CubitInterface::get_block_id_list();
  
  std::string element_type;
  int blocks_data_id;

  // check for new blocks and changes of the cubit element type!

  for (size_t i = 0; i < block_ids.size(); i++)
  {
    block_id = block_ids[i];
    element_type = CubitInterface::get_block_element_type(block_id);
    blocks_data_id = get_blocks_data_id_from_block_id(block_id);
    if (blocks_data_id == -1)
    {
      add_block(int block_id, int element_type_cubit, int element_type_ccx);
    } else {

      add_block(int block_id, int element_type_cubit, int element_type_ccx);      
    }
  }

  // check if a block changes id or is removec, this means we have erase all blocks in our blocks_data that cannot be connected anymore



  return true;
}

bool CoreBlocks::reset()
{
  blocks_data.clear();
  init();
  return true;
}

bool CoreBlocks::check_initialized()
{
  return is_initialized;
}

bool CoreBlocks::add_block(int block_id, int element_type_cubit, int element_type_ccx)
{
  std::vector<int> v = {block_id, element_type_cubit, element_type_ccx};
      
  blocks_data.push_back(v);

  return true;
}
bool CoreBlocks::delete_block(int block_id)
{
  int blocks_data_id = get_blocks_data_id_from_block_id(block_id);
  if (blocks_data_id == -1)
  {
    return false;
  } else {
    blocks_data.erase(blocks_data.begin() + blocks_data_id);
    return true;
  }
}
bool CoreBlocks::modify_block_id(int block_id, int new_block_id)
{
  int blocks_data_id = get_blocks_data_id_from_block_id(block_id);
  if (blocks_data_id == -1)
  {
    return false;
  } else {
    blocks_data[blocks_data_id][0]=new_block_id;
    return true;
  }
}
bool CoreBlocks::modify_block_element_type_cubit(int block_id, int new_element_type_cubit)
{
  int blocks_data_id = get_blocks_data_id_from_block_id(block_id);
  if (blocks_data_id == -1)
  {
    return false;
  } else {
    blocks_data[blocks_data_id][1]=new_element_type_cubit;
    return true;
  }
}
bool CoreBlocks::modify_block_element_type_ccx(int block_id, int new_element_type_ccx)
{
  int blocks_data_id = get_blocks_data_id_from_block_id(block_id);
  if (blocks_data_id == -1)
  {
    return false;
  } else {
    blocks_data[blocks_data_id][2]=new_element_type_ccx;
    return true;
  }
}

std::vector<std::string> CoreBlocks::get_ccx_element_types()
{ 
  return ccx_element_types;
}

std::string CoreBlocks::print_data()
{
  std::string str_return;
  str_return = "\n CoreBlocks blocks_data: \n";
  str_return.append("block_id, element_type_cubit, element_type_ccx \n");

  for (size_t i = 0; i < blocks_data.size(); i++)
  {
    str_return.append(std::to_string(blocks_data[i][0]) + " " + std::to_string(blocks_data[i][1]) + " " + std::to_string(blocks_data[i][2]) + " \n");
  }
  
  return str_return;
}

int CoreBlocks::get_blocks_data_id_from_block_id(int block_id)
{ 
  int return_int = -1;
  for (size_t i = 0; i < blocks_data.size(); i++)
  {
    if (blocks_data[i][0]==block_id)
    {
        return_int = i;
    }  
  }
  return return_int;
}

int CoreBlocks::get_ccx_element_type_id(std::string ccx_element_type)
{ 
  int return_int = -1;
  for (size_t i = 0; i < ccx_element_types.size(); i++)
  {
    if (ccx_element_types[i]==ccx_element_type)
    {
        return i;
    }  
  }
  return return_int;
}