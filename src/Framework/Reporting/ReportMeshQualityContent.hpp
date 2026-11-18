#pragma once
#include "Framework/MeshQuality/MeshQualitySnapshot.hpp"
#include "Framework/MeshQuality/MeshQualitySettings.hpp"
#include "ReportDocument.hpp"
#include "ReportProgress.hpp"
class ReportMeshQualityContent
{
public:
  ReportMeshQualityContent();
  static bool append(const MeshQualitySnapshot& snapshot, const MeshQualitySettings& settings,
    ReportProgress* progress, ReportDocument& output, std::string& error);
};
