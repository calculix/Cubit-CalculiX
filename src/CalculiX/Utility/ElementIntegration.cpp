#include "ElementIntegration.hpp"
#include "ElementNodeOrdering.hpp"
#include "CubitMessage.hpp"
#include "Eigen/Dense"

#include <cmath>
#include <cstddef>

namespace
{
  using Coordinates = ElementIntegration::Coordinates;

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
}

bool ElementIntegration::get_rule(const std::string& calculix_type, std::vector<IntegrationPoint>& points)
{
  points.clear();

  const double g2 = 0.577350269189626;
  const double g3 = 0.774596669241483;

  if (calculix_type == "C3D4")
  {
    points.push_back({{{0.25, 0.25, 0.25}}, 1.0 / 6.0});
  }
  else if (calculix_type == "C3D10")
  {
    const double a = 0.138196601125011;
    const double b = 0.585410196624968;

    points.push_back({{{a, a, a}}, 1.0 / 24.0});
    points.push_back({{{b, a, a}}, 1.0 / 24.0});
    points.push_back({{{a, b, a}}, 1.0 / 24.0});
    points.push_back({{{a, a, b}}, 1.0 / 24.0});
  }
  else if (calculix_type == "C3D6")
  {
    points.push_back({{{1.0 / 3.0, 1.0 / 3.0, -g2}}, 0.5});
    points.push_back({{{1.0 / 3.0, 1.0 / 3.0, g2}}, 0.5});
  }
  else if (calculix_type == "C3D15")
  {
    const double z[3] = {-g3, 0.0, g3};
    const double wz[3] = {5.0 / 9.0, 8.0 / 9.0, 5.0 / 9.0};
    const double triangle[3][2] = {{1.0 / 6.0, 1.0 / 6.0}, {2.0 / 3.0, 1.0 / 6.0}, {1.0 / 6.0, 2.0 / 3.0}};

    for (std::size_t layer = 0; layer < 3; ++layer)
    {
      for (std::size_t point = 0; point < 3; ++point)
      {
        points.push_back({{{triangle[point][0], triangle[point][1], z[layer]}}, wz[layer] / 6.0});
      }
    }
  }
  else if (calculix_type == "C3D8" || calculix_type == "C3D20")
  {
    const bool quadratic = calculix_type == "C3D20";
    const std::size_t count = quadratic ? 3 : 2;
    const double positions[3] = {quadratic ? -g3 : -g2, quadratic ? 0.0 : g2, g3};
    const double weights[3] = {quadratic ? 5.0 / 9.0 : 1.0, quadratic ? 8.0 / 9.0 : 1.0, 5.0 / 9.0};

    // CalculiX numbering: xi changes fastest, then eta, then zeta.
    for (std::size_t z = 0; z < count; ++z)
    {
      for (std::size_t y = 0; y < count; ++y)
      {
        for (std::size_t x = 0; x < count; ++x)
        {
          points.push_back({{{positions[x], positions[y], positions[z]}}, weights[x] * weights[y] * weights[z]});
        }
      }
    }
  }
  else
  {
    PRINT_ERROR("Unsupported CalculiX integration type '%s'.\n", calculix_type.c_str());
    return false;
  }

  return true;
}

bool ElementIntegration::calculate_weights(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, std::map<int, double>& weights)
{
  weights.clear();

  const std::size_t node_count = calculix_node_count(calculix_type);

  if (node_count == 0 || cubit_node_count(cubit_type) != node_count || node_coordinates.size() != node_count)
  {
    PRINT_ERROR("Incompatible connectivity for CalculiX type '%s' and Cubit type %d.\n", calculix_type.c_str(), cubit_type);
    return false;
  }

  for (const Coordinates& coordinates : node_coordinates)
  {
    for (double value : coordinates)
    {
      if (!std::isfinite(value))
      {
        PRINT_ERROR("Non-finite element coordinates in integration weight calculation.\n");
        return false;
      }
    }
  }

  std::vector<IntegrationPoint> points;

  if (!get_rule(calculix_type, points))
  {
    return false;
  }

  std::map<int, double> collected;
  std::vector<Coordinates> gradients;

  for (std::size_t point = 0; point < points.size(); ++point)
  {
    if (node_count == 4 || node_count == 10)
    {
      tetrahedron_gradients(node_count, points[point].coordinates, gradients);
    }
    else if (node_count == 6 || node_count == 15)
    {
      wedge_gradients(node_count, points[point].coordinates, gradients);
    }
    else
    {
      hexahedron_gradients(node_count, points[point].coordinates, gradients);
    }

    Eigen::Matrix3d jacobian = Eigen::Matrix3d::Zero();

    for (std::size_t node = 0; node < node_count; ++node)
    {
      const std::size_t source = ElementNodeOrdering::cubit_source_index(cubit_type, node);

      if (source >= node_coordinates.size())
      {
        PRINT_ERROR("Invalid element node ordering in integration weight calculation.\n");
        return false;
      }

      for (std::size_t row = 0; row < 3; ++row)
      {
        // Translation leaves J unchanged and reduces cancellation.
        const double coordinate = node_coordinates[source][row] - node_coordinates[0][row];

        for (std::size_t column = 0; column < 3; ++column)
        {
          jacobian(row, column) += coordinate * gradients[node][column];
        }
      }
    }

    const double determinant = jacobian.determinant();

    if (!std::isfinite(determinant) || determinant <= 0.0)
    {
      PRINT_ERROR("Invalid Jacobian determinant for '%s' at integration point %d.\n", calculix_type.c_str(), static_cast<int>(point + 1));
      return false;
    }

    const double weight = points[point].weight * determinant;

    if (!std::isfinite(weight) || weight <= 0.0)
    {
      PRINT_ERROR("Invalid physical integration weight for '%s' at integration point %d.\n", calculix_type.c_str(), static_cast<int>(point + 1));
      return false;
    }

    collected.emplace(static_cast<int>(point + 1), weight);
  }

  weights.swap(collected);
  return true;
}
