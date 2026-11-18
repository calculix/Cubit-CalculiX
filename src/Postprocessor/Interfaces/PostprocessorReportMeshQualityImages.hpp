#pragma once
#include "Framework/Reporting/ReportImageRequest.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
class PostprocessorApplicationInterface;
class PostprocessorReportMeshQualityImages
{
public:
  PostprocessorReportMeshQualityImages();
  bool collect(const PostprocessorApplicationInterface& application, const ReportImageRequest& request,
    ReportDocument& output, std::string& error) const;
};
