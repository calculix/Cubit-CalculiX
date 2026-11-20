#include "PostTopologyThresholdCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <exception>
#include <utility>

PostTopologyThresholdCommand::PostTopologyThresholdCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}

std::vector<std::string> PostTopologyThresholdCommand::get_syntax()
{
  return {"post topology threshold <value:label='cutoff'> source <string:type='quoted',number='1',label='source_name'>"};
}

std::vector<std::string> PostTopologyThresholdCommand::get_syntax_help()
{
  return {"post topology threshold VALUE source \"<source name>\""};
}

std::vector<std::string> PostTopologyThresholdCommand::get_help()
{
  return {"Repartition the retained and removed sets of an open topology series at a cutoff in [0,1]."};
}

bool PostTopologyThresholdCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string source, error;
    double threshold = 0;
    if (!active || !data.get_string("source_name",source) || source.empty() || !data.get_value("cutoff",threshold) ||
        !std::isfinite(threshold) || threshold < 0 || threshold > 1)
    {
      PRINT_ERROR("Topology cutoff requires an open source name and a value in [0,1].\n");
      return false;
    }
    if (!active->set_topology_threshold(source,threshold,error))
    {
      PRINT_ERROR("Cannot change topology cutoff: %s\n",error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot change topology cutoff: %s\n",exception.what());
    return false;
  }
}
