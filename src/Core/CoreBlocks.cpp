#include "CoreBlocks.hpp"

CoreBlocks::CoreBlocks()
{}

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