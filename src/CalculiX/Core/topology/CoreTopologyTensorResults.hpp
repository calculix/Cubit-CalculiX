#ifndef CORETOPOLOGYTENSORRESULTS_HPP
#define CORETOPOLOGYTENSORRESULTS_HPP

#include <array>
#include <map>
#include <string>

class CoreResults;
class CoreJobs;
class CoreTopology;
class CoreMaterials;

class CoreTopologyTensorResults
{
public:
  static bool read_final(CoreResults& results, int job_id, const std::string& key, const std::map<int, int>& required_elements, std::map<int, std::map<int, std::array<double, 6>>>& tensors);
  static bool read_job(CoreResults& results, CoreJobs& jobs, const CoreTopology& topology, const CoreMaterials& materials, int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors);
};

#endif
