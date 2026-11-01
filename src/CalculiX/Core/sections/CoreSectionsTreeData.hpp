#ifndef CORESECTIONSTREEDATA_HPP
#define CORESECTIONSTREEDATA_HPP

#include <string>
#include <vector>

class CoreSections;
class CoreOrientations;

class CoreSectionsTreeData
{
public:
  static std::vector<std::vector<std::string>> get_sections_tree_data(CoreSections& sections);
  static std::vector<std::vector<std::string>> get_orientations_tree_data(CoreOrientations& orientations);
};

#endif
