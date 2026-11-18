#pragma once
#include "MeshQualitySnapshot.hpp"
#include "MeshQualitySettings.hpp"
#include "Framework/Reporting/ReportProgress.hpp"
class CubitMeshQualityProvider
{
public:
  CubitMeshQualityProvider();
  // Host thread only. Reads native mesh and quality without commands, groups,
  // selection changes or writes to Cubit's quality/reference preferences.
  bool collect(const std::map<int, std::string>& blocks, const std::string& coordinateUnit,
    const MeshQualitySettings& settings, ReportProgress* progress,
    std::shared_ptr<const MeshQualitySnapshot>& output, std::string& error,
    const std::string& inputFingerprint = {}) const;
};
