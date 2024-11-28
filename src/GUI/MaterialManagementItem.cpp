#include "MaterialManagementItem.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "MaterialInterface.hpp"

MaterialManagementItem::MaterialManagementItem(QTreeWidget* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

MaterialManagementItem::MaterialManagementItem(QTreeWidgetItem* parent):
  QTreeWidgetItem (parent),
  isInitialized(false)
{}

MaterialManagementItem::~MaterialManagementItem()
{
}

void MaterialManagementItem::initialize_cubit(QString material_id_qstring,QString material_name_qstring)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  mat_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));

  QTreeWidgetItem::setText(0, material_name_qstring);
  QTreeWidgetItem::setText(1, material_id_qstring);
  this->material_id_qstring = material_id_qstring;
  this->material_name_qstring = material_name_qstring;

  MaterialInterface::Material material;
  MaterialInterface::Property prop;
  material_name = material_name_qstring.toStdString();
  material = mat_iface->get_material(material_name);
  double prop_scalar;
  MaterialVector prop_vector;
  MaterialMatrix prop_matrix;

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
    }else if (group_properties[i][1]=="2")
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
    /*else if (group_properties[i][1]=="3")
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
  isInitialized = true;
  isCubit = true;
  isLibraryGroup = false;
  isLibraryMaterial = false;
}

void MaterialManagementItem::initialize_library(std::vector<std::string> tree_data)
{
  if(isInitialized)
    return;
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();

  QTreeWidgetItem::setText(0, QString::fromStdString(tree_data[0]));
  QTreeWidgetItem::setText(1, QString::fromStdString(tree_data[1]));
  //QTreeWidgetItem::setText(2, QString::fromStdString(tree_data[2]));
  //QTreeWidgetItem::setText(3, QString::fromStdString(tree_data[3]));
  this->library_name_qstring = QString::fromStdString(tree_data[0]);
  this->library_description_qstring = QString::fromStdString(tree_data[1]);
  this->library_group_qstring = QString::fromStdString(tree_data[2].substr(0,tree_data[2].length()-tree_data[0].length()-1));
  this->library_name = tree_data[0];
  this->library_description = tree_data[1];
  this->library_group = tree_data[2].substr(0,tree_data[2].length()-tree_data[0].length()-1);
  

  this->hdf5path = tree_data[2];

  isCubit = false;
  isLibraryGroup = false;
  isLibraryMaterial = false;

  if (tree_data[3] == "Group")
  {
    isLibraryGroup = true;
  }

  if (tree_data[3] == "Material")
  {
    isLibraryMaterial = true;
  }


  if (isLibraryMaterial)
  {
    std::vector<std::vector<double>> tmp_values;

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
        tmp_values = ccx_iface->get_materiallibrary_material_values(library_name, this->library_group, group_properties[i][0]);
        if (tmp_values.size()>0)
        {
          tmp_scalar = tmp_values[0][0]; 
        }
        property_scalar.push_back(tmp_scalar);
      }else if (group_properties[i][1]=="2")
      {
        std::vector<double> tmp_vector;
        property[1] = 2;
        property[2] = int(property_vector.size());
        tmp_values = ccx_iface->get_materiallibrary_material_values(library_name, this->library_group, group_properties[i][0]);
        if (tmp_values.size()>0)
        {
          tmp_vector = tmp_values[0]; 
        }
        property_vector.push_back(tmp_vector);
      }
      else if (group_properties[i][1]=="4")
      {
        //std::vector<std::vector<double>> tmp_matrix;
        property[1] = 4;
        property[2] = int(property_matrix.size());
        tmp_values = ccx_iface->get_materiallibrary_material_values(library_name, this->library_group, group_properties[i][0]);
        property_matrix.push_back(tmp_values);
      }
      properties.push_back(property);
    }
  }

  isInitialized = true;
}

void MaterialManagementItem::update()
{
  if (isCubit)
  {
    this->update_cubit();
  }else{
    this->update_library();
  }
}

void MaterialManagementItem::update_cubit()
{
  MaterialInterface::Material material;
  MaterialInterface::Property prop;
  material = mat_iface->get_material(material_name);
  double prop_scalar;
  MaterialVector prop_vector;
  MaterialMatrix prop_matrix;

  for (size_t i = 0; i < properties.size(); i++)
  {
    if (properties[i][1]==1)
    {
      double tmp_scalar=0;
      prop = mat_iface->get_property(group_properties[properties[i][0]][0]);
      if (mat_iface->get_material_property_value(material, prop, prop_scalar))
      {
        tmp_scalar = prop_scalar; 
      }
      property_scalar[properties[i][2]] = tmp_scalar;
    }else if (properties[i][1]==2)
    {
      std::vector<double> tmp_vector;
      prop = mat_iface->get_property(group_properties[properties[i][0]][0]);
      if (mat_iface->get_material_property_value(material, prop, prop_vector))
      {
        tmp_vector = prop_vector; 
      }
      property_vector[properties[i][2]] = tmp_vector;
    }
    else if (properties[i][1]==4)
    {
      std::vector<std::vector<double>> tmp_matrix;
      prop = mat_iface->get_property(group_properties[properties[i][0]][0]);
      if (mat_iface->get_material_property_value(material, prop, prop_matrix))
      {
        tmp_matrix = prop_matrix; 
      }
      property_matrix[properties[i][2]] = tmp_matrix;
    }
  }

  property_scalar_gui = property_scalar;
  //property_vector_gui = property_vector;
  //property_tabular_gui = property_tabular;
  property_matrix_gui = property_matrix;
}

void MaterialManagementItem::update_library()
{
}

int MaterialManagementItem::get_properties_data_id_from_group(std::string group)
{
  int return_int = -1;
  for (size_t i = 0; i < group_properties.size(); i++)
  {
    if (group_properties[i][0]==group)
    {
        return_int = int(i);
    }  
  }
  return return_int;
}


void MaterialManagementItem::setScalarPropertyGUI(std::string group, double prop_scalar)
{
  int properties_data_id = get_properties_data_id_from_group(group);

  if (properties_data_id !=-1 )
  {
    if (properties[properties_data_id][1]==1)
    {
      property_scalar_gui[properties[properties_data_id][2]] = prop_scalar;
    }
  }
}

double MaterialManagementItem::getScalarPropertyGUI(std::string group)
{
  int properties_data_id = get_properties_data_id_from_group(group);
  double double_return;

  if (properties_data_id !=-1 )
  {
    if (properties[properties_data_id][1]==1)
    {
      double_return = property_scalar_gui[properties[properties_data_id][2]];
    }
  }
  return double_return;
}

void MaterialManagementItem::setMatrixPropertyGUI(std::string group, std::vector<std::vector<double>> matrix)
{
  int properties_data_id = get_properties_data_id_from_group(group);

  if (properties_data_id !=-1 )
  {
    if (properties[properties_data_id][1]==4)
    {
      property_matrix_gui[properties[properties_data_id][2]] = matrix;
    }
  }
}

std::vector<std::vector<double>> MaterialManagementItem::getMatrixPropertyGUI(std::string group)
{
  int properties_data_id = get_properties_data_id_from_group(group);
  std::vector<std::vector<double>> matrix_return;

  if (properties_data_id !=-1 )
  {
    if (properties[properties_data_id][1]==4)
    {
      matrix_return = property_matrix_gui[properties[properties_data_id][2]];
    }
  }
  return matrix_return;
}