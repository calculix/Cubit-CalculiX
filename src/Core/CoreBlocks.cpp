#include "CoreBlocks.hpp"
#include "CubitInterface.hpp"

CoreBlocks::CoreBlocks()
{
  // define the ccx element types
  // volume
  ccx_element_types.push_back("C3D4");
  ccx_element_types.push_back("C3D6"); //wedge 6
  ccx_element_types.push_back("C3D8");
  ccx_element_types.push_back("C3D8R");
  ccx_element_types.push_back("C3D8I");
  ccx_element_types.push_back("C3D10");
  ccx_element_types.push_back("C3D10T");
  ccx_element_types.push_back("C3D15"); //wedge 15
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
  ccx_element_types.push_back("M3D3");
  ccx_element_types.push_back("M3D4");
  ccx_element_types.push_back("M3D4R");
  ccx_element_types.push_back("M3D6");
  ccx_element_types.push_back("M3D8");
  ccx_element_types.push_back("M3D8R");
  
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

  // define the cubit element types
  cubit_element_types.push_back("SPHERE");
  cubit_element_types.push_back("SPRING");
  cubit_element_types.push_back("BAR");
  cubit_element_types.push_back("BAR2");
  cubit_element_types.push_back("BAR3");
  cubit_element_types.push_back("BEAM");
  cubit_element_types.push_back("BEAM2");
  cubit_element_types.push_back("BEAM3");
  cubit_element_types.push_back("TRUSS");
  cubit_element_types.push_back("TRUSS2");
  cubit_element_types.push_back("TRUSS3");
  cubit_element_types.push_back("QUAD");
  cubit_element_types.push_back("QUAD4");
  cubit_element_types.push_back("QUAD5");
  cubit_element_types.push_back("QUAD8");
  cubit_element_types.push_back("QUAD9");
  cubit_element_types.push_back("SHELL");
  cubit_element_types.push_back("SHELL4");
  cubit_element_types.push_back("SHELL8");
  cubit_element_types.push_back("SHELL9");
  cubit_element_types.push_back("TRI");
  cubit_element_types.push_back("TRI3");
  cubit_element_types.push_back("TRI6");
  cubit_element_types.push_back("TRI7");
  cubit_element_types.push_back("TRISHELL");
  cubit_element_types.push_back("TRISHELL3");
  cubit_element_types.push_back("TRISHELL6");
  cubit_element_types.push_back("TRISHELL7");
  cubit_element_types.push_back("HEX");
  cubit_element_types.push_back("HEX8");
  cubit_element_types.push_back("HEX9");
  cubit_element_types.push_back("HEX20");
  cubit_element_types.push_back("HEX27");
  cubit_element_types.push_back("TETRA");
  cubit_element_types.push_back("TETRA4");
  cubit_element_types.push_back("TETRA8");
  cubit_element_types.push_back("TETRA10");
  cubit_element_types.push_back("TETRA14");
  cubit_element_types.push_back("TETRA15");
  cubit_element_types.push_back("WEDGE");
  cubit_element_types.push_back("WEDGE6");
  cubit_element_types.push_back("WEDGE15");
  cubit_element_types.push_back("HEXSHELL");
  cubit_element_types.push_back("PYRAMID");
  cubit_element_types.push_back("PYRAMID5");
  cubit_element_types.push_back("PYRAMID8");
  cubit_element_types.push_back("PYRAMID13");
  cubit_element_types.push_back("PYRAMID18");

  // cubit 2 ccx standard element conversion

  cubit_ccx_element_types_conversion.push_back(std::make_pair("HEX","C3D8R"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("BAR","B21"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("BAR2","B21"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("BEAM","B31"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("BEAM2","B31"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("BEAM3","B32"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRUSS","T3D2"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRUSS2","T3D2"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRUSS3","T3D2"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("SPRING","SPRINGA"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRI","CPS3"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRI3","CPS3"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRI6","CPS6"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRISHELL","S3"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRISHELL3","S3"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("TRISHELL6","S6"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("SHELL","S4R"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("SHELL4","S4R"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("SHELL8","S8R"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("QUAD","CPE4R"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("QUAD4","CPE4"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("QUAD8","CPE8R"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("Tetra","C3D4"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("Tetra4","C3D4"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("Tetra10","C3D10"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("HEX","C3D8R"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("HEX8","C3D8"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("HEX20","C3D20"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("WEDGE","C3D6"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("WEDGE6","C3D6"));
  cubit_ccx_element_types_conversion.push_back(std::make_pair("WEDGE15","C3D15"));
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
  int cubit_element_type_id;
  int ccx_element_type_id;
  bool erase_block;

  // check for new blocks and changes of the cubit element type!

  for (size_t i = 0; i < block_ids.size(); i++)
  {
    block_id = block_ids[i];
    element_type = CubitInterface::get_block_element_type(block_id);
    blocks_data_id = get_blocks_data_id_from_block_id(block_id);
    cubit_element_type_id = get_cubit_element_type_id(element_type);
    ccx_element_type_id = get_standard_ccx_element_type_id(element_type);
    if (blocks_data_id == -1)
    {
      add_block(block_id,cubit_element_type_id, ccx_element_type_id);
    } else {
      if (cubit_element_type_id!=blocks_data[blocks_data_id][1])
      {
        modify_block_element_type_cubit(block_id,cubit_element_type_id);
        modify_block_element_type_ccx(block_id,ccx_element_type_id);
      }   
    }
  }

  // check if a block changes id or is removed, this means we have erase all blocks in our blocks_data that cannot be connected anymore

  for (size_t i = blocks_data.size(); i > 0; i--)
  {
    erase_block = true;
    for (size_t ii = 0; ii < block_ids.size(); ii++)
    {
      block_id = block_ids[ii];
      if (block_id == blocks_data[i-1][0])
      {
        erase_block = false;
        break;
      }
    }
    if (erase_block)
    {
      delete_block(blocks_data[i-1][0]);
    }
  }

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
bool CoreBlocks::modify_block_element_type_cubit(int block_id, int new_cubit_element_type)
{
  int blocks_data_id = get_blocks_data_id_from_block_id(block_id);
  if (blocks_data_id == -1)
  {
    return false;
  } else {
    blocks_data[blocks_data_id][1]=new_cubit_element_type;
    return true;
  }
}
bool CoreBlocks::modify_block_element_type_ccx(int block_id, int new_ccx_element_type)
{
  int blocks_data_id = get_blocks_data_id_from_block_id(block_id);
  if (blocks_data_id == -1)
  {
    return false;
  } else {
    blocks_data[blocks_data_id][2]=new_ccx_element_type;
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

int CoreBlocks::get_cubit_element_type_id(std::string cubit_element_type)
{ 
  int return_int = -1;
  for (size_t i = 0; i < cubit_element_types.size(); i++)
  {
    if (cubit_element_types[i]==cubit_element_type)
    {
        return i;
    }  
  }
  return return_int;
}

int CoreBlocks::get_standard_ccx_element_type_id(std::string cubit_element_type)
{ 
  int return_int = -1;
  for (size_t i = 0; i < cubit_ccx_element_types_conversion.size(); i++)
  {
    if (cubit_ccx_element_types_conversion[i].first==cubit_element_type)
    {
        return get_ccx_element_type_id(cubit_ccx_element_types_conversion[i].second);
    }  
  }
  return return_int;
}

std::string CoreBlocks::get_ccx_element_type_name(int block_id)
{
  std::string str_return;
  str_return = "";
  int standard_id = -1;
  int cubit_element_types_id = -1;
  
  int blocks_data_id = get_blocks_data_id_from_block_id(block_id);
  if (blocks_data_id != -1)
  {
    if (blocks_data[blocks_data_id][2] != -1)
    {
      str_return = ccx_element_types[blocks_data[blocks_data_id][2]];
    }else{
      cubit_element_types_id = blocks_data[blocks_data_id][1];
      if (cubit_element_types_id != -1)
      {
        standard_id = get_standard_ccx_element_type_id(cubit_element_types[cubit_element_types_id]);
        if (standard_id != -1){
          str_return = ccx_element_types[standard_id];
        }
      }
    }
  }
  
  return str_return;
}