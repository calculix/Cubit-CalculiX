#pragma once
#include "CalculiXJobInputBundle.hpp"
#include "CalculiXJobSetupSnapshot.hpp"
#include <memory>
#include <map>

class CalculiXJobRunRecord
{
public:
  CalculiXJobRunRecord();
  bool read_input_units(std::string& error);
  std::string materialProjectId;
  std::map<std::string, int> materialIds;
  std::string unitSystem;
  std::string geometryLengthUnit;
  int jobId = -1;
  std::string launchId;
  std::string launchedAt;
  std::string workingDirectory;
  CalculiXJobInputBundle input;
  std::shared_ptr<const CalculiXJobSetupSnapshot> setup;
  std::string setupUnavailableReason;
};
