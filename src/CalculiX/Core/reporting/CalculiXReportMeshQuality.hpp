#pragma once
#include "Framework/MeshQuality/MeshQualitySnapshot.hpp"
#include "Framework/Reporting/ReportRequest.hpp"
#include "Framework/Reporting/ReportSource.hpp"
class CalculiXJobRunRecord;
class CalculiXReportMeshQuality
{
public:
  CalculiXReportMeshQuality();
  static bool collect(const ReportRequest& request, const ReportSource& source,
    std::shared_ptr<const MeshQualitySnapshot>& output, std::string& error);
  static bool freeze_job(CalculiXJobRunRecord& record, std::string& error);
};
