#include "MeshQualityMetric.hpp"
MeshQualityMetric::MeshQualityMetric() = default;
std::string MeshQualityMetric::key() const
{
  return "mesh.quality." + family + "." + id;
}
