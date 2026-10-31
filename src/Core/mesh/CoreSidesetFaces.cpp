#include "CoreSidesetFaces.hpp"

bool CoreSidesetFaces::add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type)
{
  faces.push_back({sideset_id, sideset_name, face, element_type});
  return true;
}

std::vector<std::vector<std::string>> CoreSidesetFaces::get_sideset_face(int sideset_id) const
{
  std::vector<std::vector<std::string>> result;
  const std::string id = std::to_string(sideset_id);

  for (const auto& face : faces)
  {
    if (face[0] == id)
    {
      result.push_back({face[0], face[1], face[2], face[3]});
    }
  }

  return result;
}

void CoreSidesetFaces::clear()
{
  faces.clear();
}
