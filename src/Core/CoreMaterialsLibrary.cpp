#include "CoreMaterialsLibrary.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "MaterialInterface.hpp"
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
    mat_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));

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
  str_data[0] = "";

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

  if (hdf5Tool.nameExists(groupname))
  {
    hdf5Tool.deleteGroup(groupname);
    log = "Delete group " + groupname + " from the Material Library.\n";
    PRINT_INFO("%s", log.c_str());
  }else{
    log = "Cna't delete group " + groupname + " from the Material Library.\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }  
  
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
  str_data[0] = "";

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

  if (hdf5Tool.nameExists(material))
  {
    hdf5Tool.deleteGroup(material);
    log = "Delete material " + name + " from group " + groupname + " from the Material Library.\n";
    PRINT_INFO("%s", log.c_str());
  }else{
    log = "Can't delete material " + name + " from group " + groupname + " from the Material Library.\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  return true;
}  

bool CoreMaterialsLibrary::export_material(std::string name, std::string groupname, std::string cubit_name)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::vector<std::vector<std::string>> group_properties;
  // material group properties from core
  std::vector<std::vector<int>> properties;
  // properties[i][0] group id
  // properties[i][1] property type 1 scalar, 2 vector, 3 tabular, 4 matrix
  // properties[i][2] property id
  std::vector<double> property_scalar;
  std::vector<std::vector<std::vector<double>>> property_matrix;

  std::string log="";

  MaterialInterface::Material material;
  MaterialInterface::Property prop;
  material = mat_iface->get_material(cubit_name);
  double prop_scalar;
  MaterialVector prop_vector;
  MaterialMatrix prop_matrix;

  if (material == NULL)
  {
    log = "Material " + cubit_name + " doesn't exist.\n";
    PRINT_INFO("%s", log.c_str());

    return false;
  }

  // we get the group properties for the Calculix-FEA Type
  group_properties = ccx_iface->get_material_group_properties();

  // we build our storages for the material data
  std::vector<int> property(3);
  for (size_t i = 0; i < group_properties.size(); i++)
  {
    property[0] = int(i);
    if (group_properties[i][1]=="1")
    {
      double tmp_scalar = 0;
      property[1] = 1;
      property[2] = int(property_scalar.size());
      prop = mat_iface->get_property(group_properties[i][0]);
      if (mat_iface->get_material_property_value(material, prop, prop_scalar))
      {
        tmp_scalar = prop_scalar; 
      }
      property_scalar.push_back(tmp_scalar);
    }/*else if (group_properties[i][1]=="2")
    {
      std::vector<double> tmp_vector;
      property[1] = 2;
      property[2] = int(property_vector.size());
      prop = mat_iface->get_property(group_properties[i][0]);
      if (mat_iface->get_material_property_value(material, prop, prop_vector))
      {
        tmp_vector = prop_vector; 
      }
      property_vector.push_back(tmp_vector);
    }
    else if (group_properties[i][1]=="3")
    {
      std::vector<std::vector<double>> tmp_tabular;
      property[1] = 3;
      property[2] = property_tabular.size();
      prop = mat_iface->get_property(group_properties[i][0]);
      if (mat_iface->get_material_property_value(material, prop, prop_tabular))
      {
        tmp_tabular = prop_tabular; 
      }
      property_tabular.push_back(tmp_tabular);
    }*/
    else if (group_properties[i][1]=="4")
    {
      std::vector<std::vector<double>> tmp_matrix;
      property[1] = 4;
      property[2] = int(property_matrix.size());
      prop = mat_iface->get_property(group_properties[i][0]);
      if (mat_iface->get_material_property_value(material, prop, prop_matrix))
      {
        tmp_matrix = prop_matrix; 
      }
      property_matrix.push_back(tmp_matrix);
    }
    properties.push_back(property);
  }

  /*
  log = "Material " + cubit_name + "\n";
  PRINT_INFO("%s", log.c_str());

  for (size_t i = 0; i < group_properties.size(); i++)
  {
    log = "property " + group_properties[i][0] + " type " + std::to_string(properties[i][1]) + " \n";
    PRINT_INFO("%s", log.c_str());

    if (properties[i][1]==1)
    {
      log = std::to_string(property_scalar[properties[i][2]]) + " \n";
      PRINT_INFO("%s", log.c_str());
    }else if (properties[i][1]==4)
    {
      for (size_t ii = 0; ii < property_matrix[properties[i][2]].size(); ii++)
      {
        for (size_t iii = 0; iii < property_matrix[properties[i][2]][ii].size(); iii++)
        {
          log = std::to_string(property_matrix[properties[i][2]][ii][iii]) + " ";
          PRINT_INFO("%s", log.c_str());
        }
        log = "\n ";
        PRINT_INFO("%s", log.c_str());
      }
    }    
  }
  */

  if (!this->create_material(name,groupname))
  {
    return false;
  }
   
  for (size_t i = 0; i < group_properties.size(); i++)
  {
    if (properties[i][1]==1)
    {
      this->modify_material_scalar(name, groupname, group_properties[i][0], {property_scalar[properties[i][2]]});
    }else if (properties[i][1]==4)
    {
      std::vector<double> value_data;
      for (size_t ii = 0; ii < property_matrix[properties[i][2]].size(); ii++)
      {
        for (size_t iii = 0; iii < property_matrix[properties[i][2]][ii].size(); iii++)
        {
          value_data.push_back(property_matrix[properties[i][2]][ii][iii]);
        }
      }
      this->modify_material_matrix(name, groupname, group_properties[i][0], value_data);
    }    
  }

  return true;
}

bool CoreMaterialsLibrary::import_material(std::string name, std::string groupname, std::string cubit_name)
{
  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::vector<std::vector<std::string>> group_properties;
  // material group properties from core
  std::string log="";
  std::string material = groupname + "/" + name;
  MaterialInterface::Material cubit_material;
  cubit_material = mat_iface->get_material(cubit_name);

  if (cubit_material != NULL)
  {
    log = "Material " + cubit_name + " does already exist.\n";
    PRINT_INFO("%s", log.c_str());

    return false;
  }

  if (!hdf5Tool.nameExists(material))
  {
    log = "Can't create new material.\n" + name + " doesn't exist in group " + groupname + "!\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  if (!hdf5Tool.nameExists(material +"/material"))
  {
    log = "Can't create new material.\n" + name + " doesn't exist in group " + groupname + "!\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  std::vector<std::string> str_data;
  hdf5Tool.read_dataset_string_rank_1(std::string("PropertyGroup"), material, str_data);

  ccx_iface->silent_cmd("create material \"" + cubit_name + "\" property_group \"" + str_data[0] + "\"");

  // we get the group properties for the Calculix-FEA Type
  group_properties = ccx_iface->get_material_group_properties();

  for (size_t i = 0; i < group_properties.size(); i++)
  {
    if (group_properties[i][1]=="1")
    {
      if (hdf5Tool.nameExists(material +"/" + group_properties[i][0]))
      {
        std::vector<double> data;
        hdf5Tool.read_dataset_double_rank_1(group_properties[i][0], material, data);      
        ccx_iface->silent_cmd("modify material \"" + cubit_name + "\" scalar_properties \"" + group_properties[i][0] + "\" " + std::to_string(data[0]));
      }
    }else if (group_properties[i][1]=="4")
    {
      if (hdf5Tool.nameExists(material +"/" + group_properties[i][0]))
      {
        std::string cmd = "modify material \"" + cubit_name + "\" matrix_property \"" + group_properties[i][0] + "\"";
        std::vector<std::vector<double>> data;
        hdf5Tool.read_dataset_double_rank_2(group_properties[i][0], material, data);      

        if (data.size() > 0)
        {
          for (size_t ii = 0; ii < data.size(); ii++)
          {
            for (size_t iii = 0; iii < data[ii].size(); iii++)
            {
              cmd.append(" " + std::to_string(data[ii][iii]));
            }
          } 
          ccx_iface->silent_cmd(cmd);
        }
      }
    }    
  }

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

  std::string material = groupname + "/" + name + "/material";

  if (hdf5Tool.nameExists(material))
  {
    return true;
  }

  return false;
}


std::vector<std::vector<std::string>> CoreMaterialsLibrary::get_materiallibrary_tree_data()
{
  std::vector<std::vector<std::string>> tree_data;
  std::vector<std::string> all_groups;
  //[0][0] name
  //[0][1] description
  //[0][2] hdf5 path
  //[0][3] type: Group, Material

  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  all_groups = hdf5Tool.getGroupsFromFile();

  // for each group returned, check if its a material or not and extract the name by separator "/"
  for (size_t i = 0; i < all_groups.size(); i++)
  {
    std::vector<std::string> tree_item;
    //[0][0] name
    //tree_item.push_back(all_groups[i]);
    tree_item.push_back(all_groups[i].substr(all_groups[i].find_last_of("/") + 1));
    
    //[0][1] description
    std::vector<std::string> str_data;
    if (hdf5Tool.nameExists(all_groups[i] +"/description"))
    {
      hdf5Tool.read_dataset_string_rank_1(std::string("description"), all_groups[i], str_data);
      tree_item.push_back(str_data[0]);
    }else{
      tree_item.push_back("");
    }

    //[0][2] hdf5 path
    tree_item.push_back(all_groups[i]);

    //[0][3] type: Group, Material
    if (hdf5Tool.nameExists(all_groups[i] +"/material"))
    {
      tree_item.push_back("Material");
    }else{
      tree_item.push_back("Group");
    }

    tree_data.push_back(tree_item);
  }


  return tree_data;
}

std::vector<std::vector<std::string>> CoreMaterialsLibrary::get_materiallibrary_material_properties(std::string name, std::string groupname)
{
  std::vector<std::vector<std::string>> properties;

  return properties;
}

std::vector<std::vector<double>> CoreMaterialsLibrary::get_materiallibrary_material_values(std::string name, std::string groupname, std::string property)
{
  std::vector<std::vector<double>> values;
  std::vector<double> value_data;

  HDF5Tool hdf5Tool(ccx_uo.mPathMaterialLibrary.toStdString());
  std::string log = "";

  int property_size = ccx_iface->get_group_property_size(property);

  if (property_size == -1)
  {
    log = "Material property "+ property + " not found!\n";
    PRINT_INFO("%s", log.c_str());
    return values;
  }

  std::string material = groupname + "/" + name;
  if (property_size==1) //scalar property
  {
    hdf5Tool.read_dataset_double_rank_1(property, material, value_data);   
    if (value_data.size()>0)
    {
      values.push_back(value_data);
    }
  }else{
    hdf5Tool.read_dataset_double_rank_2(property, material, values);   
  }

  return values;
}  