#ifndef CALCULIX_MATERIAL_PROPERTIES_HPP
#define CALCULIX_MATERIAL_PROPERTIES_HPP

#include <string>
#include <vector>

struct MaterialPropertyValues
{
  std::string name;
  std::vector<std::vector<double>> values;
};

struct MaterialProperties
{
  int id = -1;
  std::string name;
  std::string group;
  std::vector<MaterialPropertyValues> properties;
};

#endif
