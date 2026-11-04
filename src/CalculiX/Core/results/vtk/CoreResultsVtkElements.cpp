#include "CoreResultsVtkElements.hpp"
#include <algorithm>
#include <cctype>
#include <numeric>

namespace
{
  std::string upper(std::string value)
  {
    std::transform(value.begin(), value.end(), value.begin(), [](unsigned char c)
    {
      return static_cast<char>(std::toupper(c));
    });
    return value;
  }

  std::vector<std::size_t> calculix_sources(const CoreResultsVtkElements::Description& element, CoreResultsVtkElements::NodeOrder order)
  {
    using Order = CoreResultsVtkElements::NodeOrder;
    std::vector<std::size_t> indices(element.node_count);
    std::iota(indices.begin(), indices.end(), 0);
    if (element.frd_type == 12 && order == Order::CalculiX)
    {
      // INP quadratic line: endpoint, midpoint, endpoint. Canonical order below
      // is CalculiX; Exodus/FRD/VTK have endpoint, endpoint, midpoint instead.
      return indices;
    }
    if (element.frd_type == 12 && order != Order::CalculiX)
    {
      std::swap(indices[1], indices[2]);
    }
    if (order == Order::Exodus || order == Order::Frd)
    {
      if (element.frd_type == 4)
      {
        for (std::size_t i = 0; i < 4; ++i)
        {
          std::swap(indices[12 + i], indices[16 + i]);
        }
      }
      else if (element.frd_type == 5)
      {
        for (std::size_t i = 0; i < 3; ++i)
        {
          std::swap(indices[9 + i], indices[12 + i]);
        }
      }
    }
    return indices;
  }
}

bool CoreResultsVtkElements::describe_frd(int type, Description& result, std::string& error)
{
  // FRD topology IDs from CCX 2.23 frd.c. Numeric VTK IDs avoid introducing
  // a VTK runtime dependency into the planning/mathematics stage.
  static const int vtk_types[] = {12, 13, 10, 25, 26, 24, 5, 22, 9, 23, 3, 21};
  static const std::size_t counts[] = {8, 6, 4, 20, 15, 10, 3, 6, 4, 8, 2, 3};
  static const char* shapes[] = {"C3D8", "C3D6", "C3D4", "C3D20", "C3D15", "C3D10", "TRI3", "TRI6", "QUAD4", "QUAD8", "LINE2", "LINE3"};
  if (type < 1 || type > 12)
  {
    error = "Unsupported FRD element topology " + std::to_string(type) + ".";
    return false;
  }
  result = {type, vtk_types[type - 1], counts[type - 1], shapes[type - 1], ""};
  error.clear();
  return true;
}

bool CoreResultsVtkElements::describe(const std::string& calculix_type, Description& result, std::string& error)
{
  std::string type = upper(calculix_type);
  if (type.rfind("DC3D", 0) == 0)
  {
    type.erase(0, 1);
  }
  int frd = 0;
  std::string rule;
  if (type == "C3D8" || type == "C3D8R" || type == "C3D8I")
  {
    frd = 1;
    rule = type == "C3D8I" ? "C3D8" : type;
  }
  else if (type == "C3D6")
  {
    frd = 2;
    rule = type;
  }
  else if (type == "C3D4")
  {
    frd = 3;
    rule = type;
  }
  else if (type == "C3D20" || type == "C3D20R")
  {
    frd = 4;
    rule = type;
  }
  else if (type == "C3D15")
  {
    frd = 5;
    rule = type;
  }
  else if (type == "C3D10")
  {
    frd = 6;
    rule = type;
  }
  else if (type == "S3" || type == "CPS3" || type == "CPE3" || type == "CAX3")
  {
    frd = 7;
  }
  else if (type == "S6" || type == "CPS6" || type == "CPE6" || type == "CAX6")
  {
    frd = 8;
  }
  else if (type == "S4" || type == "S4R" || type == "CPS4" || type == "CPS4R" || type == "CPE4" || type == "CPE4R" || type == "CAX4" || type == "CAX4R")
  {
    frd = 9;
  }
  else if (type == "S8" || type == "S8R" || type == "CPS8" || type == "CPS8R" || type == "CPE8" || type == "CPE8R" || type == "CAX8" || type == "CAX8R")
  {
    frd = 10;
  }
  else if (type == "B21" || type == "B31" || type == "T3D2" || type == "T2D2")
  {
    frd = 11;
  }
  else if (type == "B22" || type == "B32" || type == "B32R" || type == "T3D3" || type == "T2D3")
  {
    frd = 12;
  }
  if (frd == 0 || !describe_frd(frd, result, error))
  {
    error = "Unsupported CalculiX element formulation '" + calculix_type + "'.";
    return false;
  }
  // Shell/beam/plane geometry is described, but its expanded/layered IP geometry
  // is not interchangeable with a volume rule. Keep the limitation explicit.
  result.integration_type = rule;
  return true;
}

bool CoreResultsVtkElements::reorder(const Description& element, NodeOrder source, NodeOrder target, const std::vector<int>& input, std::vector<int>& output, std::string& error)
{
  Description checked;
  if (!describe_frd(element.frd_type, checked, error) || element.node_count != checked.node_count || input.size() != checked.node_count)
  {
    error = "Invalid element connectivity size for node-order conversion.";
    return false;
  }
  for (int id : input)
  {
    if (id <= 0)
    {
      error = "Non-positive global node ID in connectivity.";
      return false;
    }
  }
  const auto from = calculix_sources(checked, source);
  const auto to = calculix_sources(checked, target);
  std::vector<int> result(input.size());
  for (std::size_t i = 0; i < result.size(); ++i)
  {
    result[to[i]] = input[from[i]];
  }
  output.swap(result);
  error.clear();
  return true;
}

bool CoreResultsVtkElements::integration_point(const std::string& calculix_type, NodeOrder order, int point_id, const std::vector<ElementIntegration::Coordinates>& values, ElementIntegration::Coordinates& result, std::string& error)
{
  Description description;
  if (!describe(calculix_type, description, error))
  {
    return false;
  }
  if (description.integration_type.empty() || values.size() != description.node_count)
  {
    error = "Integration-point geometry is unsupported or has the wrong node count for '" + calculix_type + "'.";
    return false;
  }
  const auto sources = calculix_sources(description, order);
  std::vector<ElementIntegration::Coordinates> canonical(values.size());
  for (std::size_t i = 0; i < values.size(); ++i)
  {
    canonical[i] = values[sources[i]];
  }
  if (!ElementIntegration::interpolate_point(description.integration_type, point_id, canonical, result))
  {
    error = "Cannot interpolate integration point " + std::to_string(point_id) + " for '" + calculix_type + "'.";
    return false;
  }
  error.clear();
  return true;
}

bool CoreResultsVtkElements::valid_side(const Description& element, const std::string& side)
{
  const bool surface = element.frd_type >= 7 && element.frd_type <= 10;
  if (side == "SPOS" || side == "SNEG")
  {
    return surface;
  }
  if (side.size() != 2 || side[0] != 'S' || side[1] < '1' || side[1] > '6')
  {
    return false;
  }
  static const int sides[] = {6, 5, 4, 6, 5, 4, 5, 5, 6, 6, 0, 0};
  return element.frd_type >= 1 && element.frd_type <= 12 && side[1] - '0' <= sides[element.frd_type - 1];
}

bool CoreResultsVtkElements::exodus_side(const std::string& calculix_type, int side, std::string& result, std::string& error)
{
  Description description;
  if (!describe(calculix_type, description, error))
  {
    return false;
  }
  const std::string type = upper(calculix_type);
  const bool shell = !type.empty() && type[0] == 'S';
  std::vector<int> mapping;
  switch (description.frd_type)
  {
    case 1:
    case 4:
    {
      mapping = {3, 4, 5, 6, 1, 2};
      break;
    }
    case 2:
    case 5:
    {
      mapping = {3, 4, 5, 1, 2};
      break;
    }
    case 3:
    case 6:
    {
      mapping = {2, 3, 4, 1};
      break;
    }
    case 7:
    case 8:
    {
      mapping = shell ? std::vector<int>{2, 1, 3, 4, 5} : std::vector<int>{1, 2, 3};
      break;
    }
    case 9:
    case 10:
    {
      mapping = shell ? std::vector<int>{2, 1, 3, 4, 5, 6} : std::vector<int>{1, 2, 3, 4};
      break;
    }
    default:
    {
      error = "Side mapping is unsupported for '" + calculix_type + "'.";
      return false;
    }
  }
  if (side < 1 || static_cast<std::size_t>(side) > mapping.size())
  {
    error = "Invalid Exodus side for '" + calculix_type + "'.";
    return false;
  }
  result = "S" + std::to_string(mapping[side - 1]);
  error.clear();
  return true;
}

bool CoreResultsVtkElements::surface(const Description& element, const std::string& calculix_type, const std::string& side, const std::vector<int>& connectivity, int& vtk_type, std::vector<int>& nodes, std::string& error)
{
  Description checked;
  if (!describe_frd(element.frd_type, checked, error) || connectivity.size() != checked.node_count || !valid_side(checked, side))
  {
    error = "Invalid topology/connectivity/side for VTK surface extraction.";
    return false;
  }
  std::vector<int> indices;
  const int number = side.size() == 2 ? side[1] - '0' : 0;
  int type = 0;
  if (element.frd_type == 1 || element.frd_type == 4)
  {
    static const int faces[6][8] = {{0, 3, 2, 1, 11, 10, 9, 8}, {4, 5, 6, 7, 12, 13, 14, 15}, {0, 1, 5, 4, 8, 17, 12, 16}, {1, 2, 6, 5, 9, 18, 13, 17}, {2, 3, 7, 6, 10, 19, 14, 18}, {3, 0, 4, 7, 11, 16, 15, 19}};
    const bool quadratic = element.frd_type == 4;
    if (number < 1 || number > 6)
    {
      error = "A solid hex requires a numeric CCX side.";
      return false;
    }
    indices.assign(faces[number - 1], faces[number - 1] + (quadratic ? 8 : 4));
    type = quadratic ? 23 : 9;
  }
  else if (element.frd_type == 3 || element.frd_type == 6)
  {
    static const int faces[4][6] = {{0, 2, 1, 6, 5, 4}, {0, 1, 3, 4, 8, 7}, {1, 2, 3, 5, 9, 8}, {2, 0, 3, 6, 7, 9}};
    const bool quadratic = element.frd_type == 6;
    if (number < 1 || number > 4)
    {
      error = "A solid tetrahedron requires a numeric CCX side.";
      return false;
    }
    indices.assign(faces[number - 1], faces[number - 1] + (quadratic ? 6 : 3));
    type = quadratic ? 22 : 5;
  }
  else if (element.frd_type == 2 || element.frd_type == 5)
  {
    static const int faces[5][8] = {{0, 2, 1, 8, 7, 6, -1, -1}, {3, 4, 5, 9, 10, 11, -1, -1}, {0, 1, 4, 3, 6, 13, 9, 12}, {1, 2, 5, 4, 7, 14, 10, 13}, {2, 0, 3, 5, 8, 12, 11, 14}};
    const bool quadratic = element.frd_type == 5;
    if (number < 1 || number > 5)
    {
      error = "A solid wedge requires a numeric CCX side.";
      return false;
    }
    const bool triangle = number <= 2;
    const int count = triangle ? (quadratic ? 6 : 3) : (quadratic ? 8 : 4);
    indices.assign(faces[number - 1], faces[number - 1] + count);
    type = triangle ? (quadratic ? 22 : 5) : (quadratic ? 23 : 9);
  }
  else if (element.frd_type >= 7 && element.frd_type <= 10)
  {
    const std::string formulation = upper(calculix_type);
    if (formulation.empty())
    {
      error = "A shell/plane formulation is required to interpret its side labels.";
      return false;
    }
    const bool shell = formulation[0] == 'S';
    const bool triangle = element.frd_type == 7 || element.frd_type == 8;
    const bool quadratic = element.frd_type == 8 || element.frd_type == 10;
    const int corners = triangle ? 3 : 4;
    if (side == "SPOS" || side == "SNEG" || (shell && number >= 1 && number <= 2))
    {
      const bool reverse = side == "SNEG" || number == 1;
      if (reverse)
      {
        indices = triangle ? (quadratic ? std::vector<int>{0, 2, 1, 5, 4, 3} : std::vector<int>{0, 2, 1}) : (quadratic ? std::vector<int>{0, 3, 2, 1, 7, 6, 5, 4} : std::vector<int>{0, 3, 2, 1});
      }
      else
      {
        for (std::size_t i = 0; i < connectivity.size(); ++i)
        {
          indices.push_back(static_cast<int>(i));
        }
      }
      type = checked.vtk_type;
    }
    else
    {
      const int edge = shell ? number - 3 : number - 1;
      if (edge < 0 || edge >= corners)
      {
        error = "Invalid shell/plane edge label.";
        return false;
      }
      indices = {edge, (edge + 1) % corners};
      if (quadratic)
      {
        indices.push_back(corners + edge);
      }
      type = quadratic ? 21 : 3;
    }
  }
  else
  {
    error = "Unsupported VTK surface topology.";
    return false;
  }
  std::vector<int> result;
  result.reserve(indices.size());
  for (int index : indices)
  {
    if (index < 0 || static_cast<std::size_t>(index) >= connectivity.size() || connectivity[index] <= 0)
    {
      error = "Invalid local surface connectivity.";
      return false;
    }
    result.push_back(connectivity[index]);
  }
  nodes.swap(result);
  vtk_type = type;
  error.clear();
  return true;
}
