#include "CoreMaterialsLibrary.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"
#include "loadUserOptions.hpp"
#include "HDF5Tool.hpp"


CoreMaterialsLibrary::CoreMaterialsLibrary()
{}

CoreMaterialsLibrary::~CoreMaterialsLibrary()
{}

bool CoreMaterialsLibrary::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

    is_initialized = true;
    this->load_library();

    return true;
  }
}

bool CoreMaterialsLibrary::update()
{ 
  return true;
}

bool CoreMaterialsLibrary::reset()
{
  //init();
  return true;
}

bool CoreMaterialsLibrary::check_initialized()
{
  return is_initialized;
}

bool CoreMaterialsLibrary::load_library()
{
  std::string log = "Loading Material Library File " + ccx_uo.mPathMaterialLibrary.toStdString() + "\n";
  PRINT_INFO("%s", log.c_str());

  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  
  return true;
}


bool CoreMaterialsLibrary::create_group(std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  if (hdf5Tool.nameExists(groupname))
  {
    log = "Can't create new group in the Material Library.\n" + groupname + " already exists!\n";
    PRINT_INFO("%s", log.c_str());

    return false;
  }

  hdf5Tool.createGroup(groupname);
  std::vector<std::string> str_data(1);
  str_data[0] = "add Description";

  hdf5Tool.write_dataset_string_rank_1(std::string("description"), groupname, str_data); // create description dataset


  log = "Create new group " + groupname + " in the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}


bool CoreMaterialsLibrary::modify_group(std::string groupname, std::string data, int mode)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  if (!hdf5Tool.nameExists(groupname))
  {
    log = "Can't modify group in the Material Library.\n" + groupname + " doesn't exist!\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  if (mode == 0)
  {
    return this->rename_group(groupname,data);
  }else if (mode == 1)
  {
    return this->description_group(groupname,data);
  }
  
  return false;
}

bool CoreMaterialsLibrary::rename_group(std::string groupname, std::string new_groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";
  
  hdf5Tool.renameGroup(groupname,new_groupname);
  
  log = "Rename group " + groupname + " to " + new_groupname + " in the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}

bool CoreMaterialsLibrary::description_group(std::string groupname, std::string description)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";
  
  std::vector<std::string> str_data(1);
  str_data[0] = description;

  if (hdf5Tool.nameExists(groupname+"/description"))
  {
    hdf5Tool.deleteDataset("description",groupname);
  }
  
  hdf5Tool.write_dataset_string_rank_1(std::string("description"), groupname, str_data);
  
  log = "Changed description of group " + groupname + " in the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}

bool CoreMaterialsLibrary::delete_group(std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  hdf5Tool.deleteGroup(groupname);
  
  log = "Delete group " + groupname + " from the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}  

bool CoreMaterialsLibrary::create_material(std::string name, std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  std::string material = groupname + "/" + name;

  if (hdf5Tool.nameExists(material))
  {
    log = "Can't create new material in the Material Library.\n" + name + " already exists in group " + groupname + "!\n";
    PRINT_INFO("%s", log.c_str());

    return false;
  }

  hdf5Tool.createGroup(material);
  std::vector<int> data(1);
  data[0] = 1;

  std::vector<std::string> str_data(1);
  str_data[0] = "add Description";

  hdf5Tool.write_dataset_int_rank_1(std::string("material"), material, data); // create check variable
  hdf5Tool.write_dataset_string_rank_1(std::string("description"), material, str_data); // create description dataset

  str_data[0] = "CalculiX-FEA";
  hdf5Tool.write_dataset_string_rank_1(std::string("PropertyGroup"), material, str_data); // create property group


  log = "Create new material "+ name +" in group " + groupname + " in the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}

bool CoreMaterialsLibrary::modify_material(std::string name, std::string groupname, std::string data, int mode, std::vector<double> value_data)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  if (!hdf5Tool.nameExists(groupname + "/" + name + "/material"))
  {
    log = "Can't modify material in the Material Library.\n Material " + name + " in group " + groupname + " doesn't exist!\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  if (mode == 0)
  {
    return this->rename_material(name, data, groupname);
  }else if (mode == 1)
  {
    return this->description_material(name, data, groupname);
  }else if (mode == 2)
  {
    return this->modify_material_scalar(name,groupname,data,value_data);
  }else if (mode == 3)
  {
    return this->modify_material_matrix(name,groupname,data,value_data);
  }

  return false;
}


bool CoreMaterialsLibrary::rename_material(std::string name,std::string new_name, std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  std::string material = groupname + "/" + name;
  std::string new_material = groupname + "/" + new_name;

  hdf5Tool.renameGroup(material,new_material);
  
  log = "Rename material " + name + " to " + new_name + " in group " + groupname + " in the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}  

bool CoreMaterialsLibrary::description_material(std::string name, std::string description, std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";
  
  std::vector<std::string> str_data(1);
  str_data[0] = description;

  std::string material = groupname + "/" + name;

  if (hdf5Tool.nameExists(material +"/description"))
  {
    hdf5Tool.deleteDataset("description",material);
  }
  
  hdf5Tool.write_dataset_string_rank_1(std::string("description"), material, str_data);
  
  log = "Changed description of material "+ name + " in group " + groupname + " in the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}

bool CoreMaterialsLibrary::modify_material_scalar(std::string name, std::string groupname, std::string data, std::vector<double> value_data)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  int property_size = ccx_iface->get_group_property_size(data);
  if (property_size == -1)
  {
    log = "Material property "+ data + " not found!\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  if (property_size!=1)
  {
    log = "WARNING: The number of values given does not match the number \n";
    log.append("of columns defined for this property\n");
    log.append("(\'" + data + "\': # cols = 1)\n");
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  std::string material = groupname + "/" + name;

  if (hdf5Tool.nameExists(material +"/" + data))
  {
    hdf5Tool.deleteDataset(data,material);
  }

  hdf5Tool.write_dataset_double_rank_1(data, material, value_data);

  return true;
}

bool CoreMaterialsLibrary::modify_material_matrix(std::string name, std::string groupname, std::string data, std::vector<double> value_data)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  int property_size = ccx_iface->get_group_property_size(data);
  if (property_size == -1)
  {
    log = "Material property "+ data + " not found!\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  if (value_data.size() % property_size != 0)
  {
    log = "WARNING: The number of values given does not match the number \n";
    log.append("of columns defined for this property\n");
    log.append("(\'" + data + "\': # cols = " + std::to_string(property_size) + ")\n");
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  std::string material = groupname + "/" + name;

  if (hdf5Tool.nameExists(material +"/" + data))
  {
    hdf5Tool.deleteDataset(data,material);
  }

  int rows = value_data.size()/property_size;
  
  std::vector<std::vector<double>> values;
  for (size_t i = 0; i < rows; i++)
  {
    std::vector<double> row;
    for (size_t ii = 0; ii < property_size; ii++)
    {
      row.push_back(value_data[i*property_size+ii]);
    }
    values.push_back(row);
  }
  
  hdf5Tool.write_dataset_double_rank_2(data, material, values);

  return true;
}

bool CoreMaterialsLibrary::delete_material(std::string name, std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  std::string material = groupname + "/" + name;

  hdf5Tool.deleteGroup(material);
  
  log = "Delete material " + name + " from group " + groupname + " from the Material Library.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}  

bool CoreMaterialsLibrary::export_material(std::string name, std::string groupname, std::string cubit_name)
{
  return true;
}

bool CoreMaterialsLibrary::import_material(std::string name, std::string groupname, std::string cubit_name)
{
  return true;
}

bool CoreMaterialsLibrary::check_group(std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());

  if (hdf5Tool.nameExists(groupname))
  {
    return true;
  }

  return false;
}

bool CoreMaterialsLibrary::check_material(std::string name, std::string groupname)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());

  std::string material = groupname + "/" + name + "/is_material";

  if (hdf5Tool.nameExists(material))
  {
    return true;
  }

  return false;
}