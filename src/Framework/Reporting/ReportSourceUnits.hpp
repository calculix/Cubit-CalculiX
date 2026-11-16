#pragma once
#include <string>

// Recorded input units. Native geometry and current presentation are separate.
class ReportSourceUnits
{
public:
  ReportSourceUnits();
  std::string system;
  std::string coordinate_unit;
  std::string time_unit;
};
