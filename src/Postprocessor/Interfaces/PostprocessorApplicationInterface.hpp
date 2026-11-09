#ifndef POSTPROCESSORAPPLICATIONINTERFACE_HPP
#define POSTPROCESSORAPPLICATIONINTERFACE_HPP

#include "Framework/Interfaces/UserOption.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/options/PostprocessorUserOptions.hpp"

#include <optional>
#include <string>
#include <variant>

class ResultSourceRegistryInterface;

// Application boundary shared by commands and future GUI actions. Owned by the
// Postprocessor module. The borrowed registry must outlive this application.
// Names are resolved here; Core operations receive internal source IDs.
class PostprocessorApplicationInterface
{
public:
  using DatasetSelector = std::variant<std::size_t,std::string>;
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
  bool set_pick_mode(Postprocessor::EntityKind kind, std::string& error);
  bool clear_pick(std::string& error);
  bool pick_entity(const std::string& source_name, const DatasetSelector& dataset,
    Postprocessor::EntityKind kind, std::int64_t id, std::optional<std::int64_t> integration_point, std::string& error);
  std::optional<Postprocessor::ProbeResult> probe(std::string& error) const;
  bool select_source(const std::string& name, std::string& error);
  bool close_source(const std::string& name, std::string& error);
  bool close_all_sources(std::string& error);
  bool reset(std::string& error);
  std::optional<Postprocessor::AnimationDefinition> animation(const std::string& source_name, std::string& error) const;
  bool set_animation(const std::string& source_name, const Postprocessor::AnimationDefinition& animation, std::string& error);
  std::optional<Postprocessor::ResultColoring> coloring(const std::string& source_name, std::string& error) const;
  std::optional<std::array<double,2>> all_frame_range(const std::string& source_id,
    const Postprocessor::FieldSelection& selection,std::string& error,
    const std::set<std::size_t>& hidden = {}) const;
  bool set_coloring(const std::string& source_name, const Postprocessor::ResultColoring& coloring, std::string& error);
  std::optional<Postprocessor::ResultDeformation> deformation(const std::string& source_name, std::string& error) const;
  bool set_deformation(const std::string& source_name, const Postprocessor::ResultDeformation& deformation, std::string& error);
  bool deform_field(const std::string& source_name, const std::string& field,
    std::optional<std::size_t> dataset, std::string& error);
  bool deform_auto(const std::string& source_name, std::string& error);
  bool color_field(const std::string& source_name, const std::string& field, const std::string& component,
    bool magnitude, std::optional<std::size_t> dataset, std::string& error);
  // User-facing ordinal (1-based). Empty source_name uses the active source.
  bool set_frame(const std::string& source_name, std::size_t frame_number, std::string& error);
  bool frame_has_mesh(const std::string& source_id, std::size_t increment_index) const;
  bool set_source_visible(const std::string& name, bool visible, std::string& error);
  std::vector<SimResults::Dataset> mesh_datasets(const std::string& source_id) const;
  bool set_dataset_visibility(const std::string& source_name, Postprocessor::DatasetVisibilityAction action,
    const std::optional<DatasetSelector>& dataset, std::string& error);
  // A valid empty view succeeds without changing the camera or fit tokens.
  bool request_camera_fit(std::string& error);
  bool camera_navigation_ready(const std::string& view_id) const noexcept;
  bool set_camera(const Postprocessor::CameraDefinition& camera, std::string& error);
  bool rotate_camera(double yaw, double pitch, std::string& error);
  bool pan_camera(double horizontal, double vertical, std::string& error);
  bool zoom_camera(double factor, std::string& error);
  std::shared_ptr<const SimResults::VtkResultsData> result_data(const std::string& source_id) const;
  std::vector<Postprocessor::ResultFieldGroup> field_groups(const std::string& source_id, std::optional<std::size_t> current) const;
  bool source_has_mesh(const std::string& source_id) const noexcept;
  std::uint64_t display_revision() const noexcept;
  // False with an empty error means no visible mesh; bounds remain unchanged.
  bool visible_bounds(const std::string& view_id, std::array<double, 6>& bounds, std::string& error) const;
  bool complete_camera_fit(const std::string& view_id, std::uint64_t request,
    std::uint64_t revision, const Postprocessor::CameraDefinition& camera, std::string& error);

private:
  std::optional<std::size_t> resolve_dataset(const std::string& source_id, const DatasetSelector& dataset, std::string& error) const;
  ResultSourceRegistryInterface& providers;
  Postprocessor::PostprocessorCore core;
  Postprocessor::PostprocessorUserOptions options;
};

#endif // POSTPROCESSORAPPLICATIONINTERFACE_HPP
