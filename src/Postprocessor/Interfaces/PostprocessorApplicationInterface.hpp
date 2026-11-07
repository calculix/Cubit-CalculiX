#ifndef POSTPROCESSORAPPLICATIONINTERFACE_HPP
#define POSTPROCESSORAPPLICATIONINTERFACE_HPP

#include "Framework/Interfaces/UserOption.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/options/PostprocessorUserOptions.hpp"

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
  bool initialize_user_options(UserOptionsInterface& store, std::string& error);
  void shutdown_user_options() noexcept;
  const Postprocessor::UserOptions& user_options() const noexcept;
  const std::string& active_icons_path() const noexcept;
  bool user_options_restart_required() const;
  std::vector<UserOption> user_option_definitions() const;
  bool user_option_values(UserOptionValues& snapshot, std::string& error) const;
  bool apply_user_options(const UserOptionValues& changes, std::string& error);

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
  Postprocessor::PostprocessorUserOptions options;
};

#endif // POSTPROCESSORAPPLICATIONINTERFACE_HPP
