#ifndef POSTPROCESSORAPPLICATIONINTERFACE_HPP
#define POSTPROCESSORAPPLICATIONINTERFACE_HPP

#include "Postprocessor/Core/application/PostprocessorCore.hpp"

#include <optional>
#include <string>

class ResultSourceRegistryInterface;

// Application boundary shared by commands and future GUI actions. Owned by the
// Postprocessor module. The borrowed registry must outlive this application.
// Names are resolved here; Core operations receive internal source IDs.
class PostprocessorApplicationInterface
{
public:
  explicit PostprocessorApplicationInterface(ResultSourceRegistryInterface& providers);

  bool open_job(
    const std::string& prefix,
    int job_id,
    const std::optional<std::string>& name,
    std::string& error);

  const Postprocessor::PostprocessorSession& session() const noexcept;
  bool select_source(const std::string& name, std::string& error);
  bool close_source(const std::string& name, std::string& error);
  bool close_all_sources(std::string& error);

private:
  ResultSourceRegistryInterface& providers;
  Postprocessor::PostprocessorCore core;
};

#endif // POSTPROCESSORAPPLICATIONINTERFACE_HPP
