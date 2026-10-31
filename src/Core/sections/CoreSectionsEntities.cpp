#include "CoreSectionsEntities.hpp"
#include "CoreSections.hpp"

std::vector<std::vector<std::string>> CoreSectionsEntities::get_material_entities(CoreSections& sections, int material_id)
{
  std::vector<std::vector<std::string>> entities;
  const std::string id = std::to_string(material_id);

  for (const auto& section : sections.sections_data)
  {
    const auto* data = sections.get_section_data(section[0]);

    if (data && (*data)[2] == id)
    {
      entities.push_back({"block", (*data)[1]});
    }
  }

  return entities;
}

std::vector<std::vector<std::string>> CoreSectionsEntities::get_section_entities(CoreSections& sections, int section_id)
{
  const auto* data = sections.get_section_data(section_id);

  if (!data)
  {
    return {};
  }

  return {{"block", (*data)[1]}};
}
