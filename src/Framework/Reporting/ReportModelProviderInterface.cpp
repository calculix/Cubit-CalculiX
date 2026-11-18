#include "ReportModelProviderInterface.hpp"

ReportModelProviderInterface::~ReportModelProviderInterface() = default;

bool ReportModelProviderInterface::mesh_quality(const ReportRequest&, const ReportSource&,
  std::shared_ptr<const MeshQualitySnapshot>& output, std::string& error) const
{
  output.reset();
  error.clear();
  return true;
}
