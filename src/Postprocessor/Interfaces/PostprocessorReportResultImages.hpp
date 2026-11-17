#pragma once
#include "Framework/Reporting/ReportImageRequest.hpp"
#include "Framework/Reporting/ReportDocument.hpp"
class PostprocessorApplicationInterface;

class PostprocessorReportResultImages
{
public:
  PostprocessorReportResultImages();
  bool collect(const PostprocessorApplicationInterface& application,
    const ReportImageRequest& request, ReportDocument& output, std::string& error) const;
};
