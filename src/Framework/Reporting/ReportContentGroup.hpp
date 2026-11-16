#pragma once
#include <string>

class ReportContentGroup
{
public:
  ReportContentGroup();
  ReportContentGroup(std::string id, std::string label, std::string parent = "", std::string gate = "");
  std::string id;
  std::string label;
  std::string parent_id;
  std::string gate_option;
};
