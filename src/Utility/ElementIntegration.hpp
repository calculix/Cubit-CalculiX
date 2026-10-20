#ifndef ELEMENTINTEGRATION_HPP
#define ELEMENTINTEGRATION_HPP

#include <array>
#include <map>
#include <string>
#include <vector>

namespace ElementIntegration
{
  using Coordinates = std::array<double, 3>;

  struct IntegrationPoint
  {
    Coordinates coordinates;
    double weight;
  };

  // Vector index + 1 is the CalculiX integration point ID.
  bool get_rule(const std::string& calculix_type, std::vector<IntegrationPoint>& points);

  /*
   * Node coordinates must be in Cubit/Exodus connectivity order.
   * The returned map uses CalculiX integration point IDs.
   * Each value is the reference weight multiplied by det(J).
   * Output remains empty on failure.
   */
  bool calculate_weights(const std::string& calculix_type, int cubit_type, const std::vector<Coordinates>& node_coordinates, std::map<int, double>& weights);
}

#endif // ELEMENTINTEGRATION_HPP
