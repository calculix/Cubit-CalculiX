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
  bool reset(std::string& error);
  bool set_source_visible(const std::string& name, bool visible, std::string& error);
  bool request_camera_fit(std::string& error);
  bool camera_navigation_ready(const std::string& view_id) const noexcept;
  bool set_camera(const Postprocessor::CameraDefinition& camera, std::string& error);
  bool rotate_camera(double yaw, double pitch, std::string& error);
  bool pan_camera(double horizontal, double vertical, std::string& error);
  bool zoom_camera(double factor, std::string& error);
  std::shared_ptr<const SimResults::VtkResultsData> result_data(const std::string& source_id) const;
  bool source_has_mesh(const std::string& source_id) const noexcept;
  std::uint64_t display_revision() const noexcept;
  bool visible_bounds(const std::string& view_id, std::array<double, 6>& bounds, std::string& error) const;
  bool complete_camera_fit(const std::string& view_id, std::uint64_t request,
    std::uint64_t revision, const Postprocessor::CameraDefinition& camera, std::string& error);

private:
  ResultSourceRegistryInterface& providers;
  Postprocessor::PostprocessorCore core;
  Postprocessor::PostprocessorUserOptions options;
};

#endif // POSTPROCESSORAPPLICATIONINTERFACE_HPP
