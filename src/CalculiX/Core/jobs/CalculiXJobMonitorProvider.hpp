#ifndef CALCULIXJOBMONITORPROVIDER_HPP
#define CALCULIXJOBMONITORPROVIDER_HPP

#include "Framework/Interfaces/JobMonitorProviderInterface.hpp"

class CalculiXJobMonitorProvider : public JobMonitorProviderInterface
{
public:
  bool snapshot(std::vector<JobMonitorEntry>& output, std::string& error) const override;
};

#endif
