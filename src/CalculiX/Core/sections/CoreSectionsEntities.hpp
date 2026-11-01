#ifndef CORESECTIONSENTITIES_HPP
#define CORESECTIONSENTITIES_HPP

#include <string>
#include <vector>

class CoreSections;

class CoreSectionsEntities
{
public:
  static std::vector<std::vector<std::string>> get_material_entities(CoreSections& sections, int material_id);
  static std::vector<std::vector<std::string>> get_section_entities(CoreSections& sections, int section_id);
};

#endif
