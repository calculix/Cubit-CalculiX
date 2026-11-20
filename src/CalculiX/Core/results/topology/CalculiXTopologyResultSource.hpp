#pragma once
#include "Framework/Results/PublishedResultSource.hpp"
#include "Framework/Results/ResultJobEntry.hpp"
class ResultSourceProviderInterface;
class CalculiXTopologyResultSource
{
public:
  static void append_catalogue(std::vector<SimResults::ResultJobEntry>& entries);
  static bool load(ResultSourceProviderInterface& provider, const SimResults::JobSourceReference& request,
    bool restoring, SimResults::PublishedResultSource& output, std::string& error,
    std::optional<double> newThreshold = std::nullopt);
};
