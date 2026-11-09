#pragma once
#include "Framework/Reporting/ReportImage.hpp"
#include <string>

// Imports self-contained SVG/PNG/JPEG into a bounded PNG snapshot. Export and
// persistence never reopen the original file. Output changes only on success.
class ReportLogo
{
public:
  static bool load(const std::string& filename, ReportImage& output, std::string& error);
  static bool validate(const ReportImage& image, std::string& error);
};
