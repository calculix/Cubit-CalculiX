#pragma once
#include "CalculiXJobInputBundle.hpp"
#include "CalculiXJobSetupSnapshot.hpp"
#include <memory>

class CalculiXJobRunRecord
{
public:
  CalculiXJobRunRecord();
  int jobId = -1;
  std::string launchId;
  std::string launchedAt;
  std::string workingDirectory;
  CalculiXJobInputBundle input;
  std::shared_ptr<const CalculiXJobSetupSnapshot> setup;
  std::string setupUnavailableReason;
};
