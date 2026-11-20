#pragma once
#include <array>
#include <map>
#include <set>
#include <string>
#include <utility>

class CoreConnections;
struct ConnectionEndpoint;
namespace SimUnits
{
  struct QuantityUnit;
}

class CoreConnectionEndpointResolver
{
public:
  using Position = std::array<double,3>;
  using Resolved = std::pair<int,Position>; // exported solver node and coordinates in solver units
  CoreConnectionEndpointResolver(const CoreConnections& connections, const std::set<int>& structural,
    const std::map<int,std::array<int,2>>& rigid_roles, const std::map<int,Position>& rigid_positions,
    const SimUnits::QuantityUnit& length_quantity);
  bool resolve(const ConnectionEndpoint& endpoint, Resolved& result, std::string& error) const;
private:
  const CoreConnections& model;
  const std::set<int>& mesh_nodes;
  const std::map<int,std::array<int,2>>& roles;
  const std::map<int,Position>& positions;
  const SimUnits::QuantityUnit& length;
  std::set<int> rotations;
};
