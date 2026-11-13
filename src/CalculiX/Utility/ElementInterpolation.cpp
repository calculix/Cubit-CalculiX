#include "ElementInterpolation.hpp"
#include "ElementPhysicalMapping.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <utility>

namespace
{
  bool report_error(const std::string& message, std::string* error)
  {
    if (error)
    {
      *error = message;
    }
    else
    {
      PRINT_ERROR("%s\n", message.c_str());
    }
    return false;
  }

  using Coordinates = ElementInterpolation::Coordinates;

  std::size_t calculix_node_count(const std::string& type)
  {
    if (type == "C3D4")
    {
      return 4;
    }

    if (type == "C3D6")
    {
      return 6;
    }

    if (type == "C3D8")
    {
      return 8;
    }

    if (type == "C3D10")
    {
      return 10;
    }

    if (type == "C3D15")
    {
      return 15;
    }

    if (type == "C3D20")
    {
      return 20;
    }

    return 0;
  }

  std::size_t cubit_node_count(int type)
  {
    switch (type)
    {
      case 30: // TETRA
      case 31: // TETRA4
      {
        return 4;
      }

      case 33: // TETRA10
      {
        return 10;
      }

      case 41: // HEX
      case 42: // HEX8
      {
        return 8;
      }

      case 44: // HEX20
      {
        return 20;
      }

      case 49: // WEDGE
      case 50: // WEDGE6
      {
        return 6;
      }

      case 52: // WEDGE15
      {
        return 15;
      }

      default:
      {
        return 0;
      }
    }
  }

  void tetrahedron_gradients(std::size_t node_count, const Coordinates& point, std::vector<Coordinates>& gradients)
  {
    const double l[4] = {1.0 - point[0] - point[1] - point[2], point[0], point[1], point[2]};
    const Coordinates dl[4] = {{{-1.0, -1.0, -1.0}}, {{1.0, 0.0, 0.0}}, {{0.0, 1.0, 0.0}}, {{0.0, 0.0, 1.0}}};
    const int edges[6][2] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {1, 3}, {2, 3}};

    gradients.assign(node_count, Coordinates{});

    for (std::size_t i = 0; i < 4; ++i)
    {
      for (std::size_t axis = 0; axis < 3; ++axis)
      {
        gradients[i][axis] = node_count == 4 ? dl[i][axis] : (4.0 * l[i] - 1.0) * dl[i][axis];
      }
    }

    if (node_count == 10)
    {
      for (std::size_t edge = 0; edge < 6; ++edge)
      {
        const int a = edges[edge][0];
        const int b = edges[edge][1];

        for (std::size_t axis = 0; axis < 3; ++axis)
        {
          gradients[4 + edge][axis] = 4.0 * (dl[a][axis] * l[b] + l[a] * dl[b][axis]);
        }
      }
    }
  }

  void hexahedron_gradients(std::size_t node_count, const Coordinates& point, std::vector<Coordinates>& gradients)
  {
    const int corners[8][3] = {
      {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
      {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
    };

    // CalculiX order: bottom edges, top edges, vertical edges.
    const int edges[12][3] = {
      {0, -1, -1}, {1, 0, -1}, {0, 1, -1}, {-1, 0, -1},
      {0, -1, 1}, {1, 0, 1}, {0, 1, 1}, {-1, 0, 1},
      {-1, -1, 0}, {1, -1, 0}, {1, 1, 0}, {-1, 1, 0}
    };

    gradients.assign(node_count, Coordinates{});

    for (std::size_t node = 0; node < 8; ++node)
    {
      const double q = corners[node][0] * point[0] + corners[node][1] * point[1] + corners[node][2] * point[2] - 2.0;

      for (std::size_t axis = 0; axis < 3; ++axis)
      {
        double derivative = static_cast<double>(corners[node][axis]) / 8.0;

        for (std::size_t other = 0; other < 3; ++other)
        {
          if (other != axis)
          {
            derivative *= 1.0 + corners[node][other] * point[other];
          }
        }

        if (node_count == 20)
        {
          derivative *= q + 1.0 + corners[node][axis] * point[axis];
        }

        gradients[node][axis] = derivative;
      }
    }

    if (node_count == 20)
    {
      for (std::size_t edge = 0; edge < 12; ++edge)
      {
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
          double derivative = edges[edge][axis] == 0 ? -0.5 * point[axis] : 0.25 * edges[edge][axis];

          for (std::size_t other = 0; other < 3; ++other)
          {
            if (other != axis)
            {
              derivative *= edges[edge][other] == 0 ? 1.0 - point[other] * point[other] : 1.0 + edges[edge][other] * point[other];
            }
          }

          gradients[8 + edge][axis] = derivative;
        }
      }
    }
  }

  void wedge_gradients(std::size_t node_count, const Coordinates& point, std::vector<Coordinates>& gradients)
  {
    const double l[3] = {1.0 - point[0] - point[1], point[0], point[1]};
    const Coordinates dl[3] = {{{-1.0, -1.0, 0.0}}, {{1.0, 0.0, 0.0}}, {{0.0, 1.0, 0.0}}};
    const int edges[3][2] = {{0, 1}, {1, 2}, {2, 0}};
    const double z = point[2];

    gradients.assign(node_count, Coordinates{});

    for (std::size_t side = 0; side < 2; ++side)
    {
      const double sign = side == 0 ? -1.0 : 1.0;
      const double factor = 1.0 + sign * z;

      for (std::size_t node = 0; node < 3; ++node)
      {
        const std::size_t index = side * 3 + node;

        for (std::size_t axis = 0; axis < 2; ++axis)
        {
          gradients[index][axis] = 0.5 * factor * dl[node][axis];

          if (node_count == 15)
          {
            gradients[index][axis] *= 4.0 * l[node] - 2.0 + sign * z;
          }
        }

        gradients[index][2] = 0.5 * sign * l[node];

        if (node_count == 15)
        {
          gradients[index][2] *= 2.0 * l[node] - 1.0 + 2.0 * sign * z;
        }
      }

      if (node_count == 15)
      {
        for (std::size_t edge = 0; edge < 3; ++edge)
        {
          const int a = edges[edge][0];
          const int b = edges[edge][1];
          const std::size_t index = 6 + side * 3 + edge;

          for (std::size_t axis = 0; axis < 2; ++axis)
          {
            gradients[index][axis] = 2.0 * factor * (dl[a][axis] * l[b] + l[a] * dl[b][axis]);
          }

          gradients[index][2] = 2.0 * sign * l[a] * l[b];
        }
      }
    }

    if (node_count == 15)
    {
      for (std::size_t node = 0; node < 3; ++node)
      {
        for (std::size_t axis = 0; axis < 2; ++axis)
        {
          gradients[12 + node][axis] = dl[node][axis] * (1.0 - z * z);
        }

        gradients[12 + node][2] = -2.0 * z * l[node];
      }
    }
  }

  void tetrahedron_values(std::size_t count, const Coordinates& p, std::vector<double>& values)
  {
    const double l[4] = {1.0 - p[0] - p[1] - p[2], p[0], p[1], p[2]};
    const int edges[6][2] = {{0, 1}, {1, 2}, {2, 0}, {0, 3}, {1, 3}, {2, 3}};
    values.resize(count);
    for (std::size_t node = 0; node < 4; ++node)
    {
      values[node] = count == 4 ? l[node] : l[node] * (2.0 * l[node] - 1.0);
    }
    if (count == 10)
    {
      for (std::size_t edge = 0; edge < 6; ++edge)
      {
        values[4 + edge] = 4.0 * l[edges[edge][0]] * l[edges[edge][1]];
      }
    }
  }

  void hexahedron_values(std::size_t count, const Coordinates& p, std::vector<double>& values)
  {
    const int corners[8][3] = {
      {-1, -1, -1}, {1, -1, -1}, {1, 1, -1}, {-1, 1, -1},
      {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}, {-1, 1, 1}
    };
    const int edges[12][3] = {
      {0, -1, -1}, {1, 0, -1}, {0, 1, -1}, {-1, 0, -1},
      {0, -1, 1}, {1, 0, 1}, {0, 1, 1}, {-1, 0, 1},
      {-1, -1, 0}, {1, -1, 0}, {1, 1, 0}, {-1, 1, 0}
    };
    values.resize(count);
    for (std::size_t node = 0; node < 8; ++node)
    {
      values[node] = 0.125;
      double q = -2.0;
      for (std::size_t axis = 0; axis < 3; ++axis)
      {
        values[node] *= 1.0 + corners[node][axis] * p[axis];
        q += corners[node][axis] * p[axis];
      }
      if (count == 20)
      {
        values[node] *= q;
      }
    }
    if (count == 20)
    {
      for (std::size_t edge = 0; edge < 12; ++edge)
      {
        values[8 + edge] = 0.25;
        for (std::size_t axis = 0; axis < 3; ++axis)
        {
          values[8 + edge] *= edges[edge][axis] == 0 ? 1.0 - p[axis] * p[axis] : 1.0 + edges[edge][axis] * p[axis];
        }
      }
    }
  }

  void wedge_values(std::size_t count, const Coordinates& p, std::vector<double>& values)
  {
    const double l[3] = {1.0 - p[0] - p[1], p[0], p[1]};
    const int edges[3][2] = {{0, 1}, {1, 2}, {2, 0}};
    values.resize(count);
    for (std::size_t side = 0; side < 2; ++side)
    {
      const double sign = side == 0 ? -1.0 : 1.0;
      const double factor = 1.0 + sign * p[2];
      for (std::size_t node = 0; node < 3; ++node)
      {
        values[side * 3 + node] = 0.5 * l[node] * factor;
        if (count == 15)
        {
          values[side * 3 + node] *= 2.0 * l[node] - 2.0 + sign * p[2];
        }
      }
      if (count == 15)
      {
        for (std::size_t edge = 0; edge < 3; ++edge)
        {
          values[6 + side * 3 + edge] = 2.0 * factor * l[edges[edge][0]] * l[edges[edge][1]];
        }
      }
    }
    if (count == 15)
    {
      for (std::size_t node = 0; node < 3; ++node)
      {
        values[12 + node] = l[node] * (1.0 - p[2] * p[2]);
      }
    }
  }
}

std::size_t ElementInterpolation::node_count(const std::string& calculix_type)
{
  return calculix_node_count(calculix_type);
}

bool ElementInterpolation::matches_cubit_type(const std::string& calculix_type, int cubit_type)
{
  const auto count = node_count(calculix_type);
  return count != 0 && count == cubit_node_count(cubit_type);
}

bool ElementInterpolation::evaluate_reference(const std::string& calculix_type, const Coordinates& point, std::vector<double>& values, std::vector<Coordinates>& gradients, std::string* error)
{
  values.clear();
  gradients.clear();
  const auto count = node_count(calculix_type);
  bool valid = count != 0;
  for (double value : point)
  {
    valid = valid && std::isfinite(value);
  }
  const bool tet = count == 4 || count == 10;
  const bool wedge = count == 6 || count == 15;
  if (tet || wedge)
  {
    valid = valid && point[0] >= 0.0 && point[1] >= 0.0 && point[0] + point[1] <= 1.0;
    valid = valid && (tet ? point[2] >= 0.0 && point[0] + point[1] + point[2] <= 1.0 : std::abs(point[2]) <= 1.0);
  }
  else
  {
    for (double value : point)
    {
      valid = valid && std::abs(value) <= 1.0;
    }
  }
  if (!valid)
  {
    return report_error("Unsupported element type or invalid reference interpolation point.", error);
  }
  if (tet)
  {
    tetrahedron_values(count, point, values);
    tetrahedron_gradients(count, point, gradients);
  }
  else if (wedge)
  {
    wedge_values(count, point, values);
    wedge_gradients(count, point, gradients);
  }
  else
  {
    hexahedron_values(count, point, values);
    hexahedron_gradients(count, point, gradients);
  }
  return true;
}

bool ElementInterpolation::evaluate_physical(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, const Coordinates& point, std::vector<double>& values, double& determinant, std::string* error)
{
  values.clear();
  determinant = 0.0;
  std::vector<double> shape;
  std::vector<Coordinates> gradients;
  if (!evaluate_reference(calculix_type, point, shape, gradients, error))
  {
    return false;
  }
  return evaluate_physical_from_reference(calculix_type, cubit_type, node_coordinates, shape, gradients, values, determinant, error);
}

bool ElementInterpolation::evaluate_physical_from_reference(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, const std::vector<double>& reference_values, const std::vector<Coordinates>& reference_gradients, std::vector<double>& values, double& determinant, std::string* error)
{
  if (&values == &reference_values)
  {
    return report_error("Reference interpolation inputs must not alias output values.", error);
  }
  values.clear();
  determinant = 0.0;
  ElementPhysicalMapping mapping;
  if (!mapping.prepare(calculix_type, cubit_type, node_coordinates, error))
  {
    return false;
  }
  return mapping.map_reference(reference_values, reference_gradients, values, determinant, error);
}
