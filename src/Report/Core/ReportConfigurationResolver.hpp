#pragma once
#include "Framework/Reporting/ReportTemplate.hpp"

class ReportConfigurationResolver
{
public:
  ReportConfigurationResolver();
  static void add_common_options(ReportTemplate& definition);
  // Complete configuration is validated against the owning template. Unknown
  // settings are rejected, preventing old profiles silently changing meaning.
  static bool resolve(const ReportTemplate& definition, const ReportConfiguration& overrides,
    ReportConfiguration& output, std::string& error);
};
