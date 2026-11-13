#ifndef POSTPROCESSORAPPLICATIONINTERFACE_HPP
#define POSTPROCESSORAPPLICATIONINTERFACE_HPP

#include "Framework/Interfaces/UserOption.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/options/PostprocessorUserOptions.hpp"

#include <optional>
#include <functional>
#include <string>
#include <variant>
#include "Framework/Results/ResultJobEntry.hpp"

class ResultSourceRegistryInterface;

// Application boundary shared by commands and future GUI actions. Owned by the
// Postprocessor module. The borrowed registry must outlive this application.
// Names are resolved here; Core operations receive internal source IDs.
class PostprocessorApplicationInterface
{
public:
  using CaptureRenderer = std::function<bool(const Postprocessor::CaptureSnapshot&,
    const std::shared_ptr<PostprocessorApplicationInterface>&,QByteArray&,QJsonObject&,std::string&)>;
  using CaptureSizer = std::function<std::array<int,2>(Postprocessor::CaptureTarget,const std::string&)>;
  void set_capture_renderer(CaptureRenderer renderer,CaptureSizer sizer = {});
  std::array<int,2> capture_reference_size(Postprocessor::CaptureTarget target,const std::string& id) const;
  bool render_capture(const Postprocessor::CaptureDefinition& request,QByteArray& png,QJsonObject& provenance,std::string& error) const;
  bool capture_file(const Postprocessor::CaptureDefinition& request,const std::string& path,std::string& error) const;
  using DatasetSelector = std::variant<std::size_t,std::string>;
  explicit PostprocessorApplicationInterface(ResultSourceRegistryInterface& providers);
  bool read_project_definitions(const std::string& filename, std::string& error);
  bool resolve_project_sources(std::string& error);
  bool commit_project_restore(std::string& error);
  bool prepare_project_save(std::string& error);
  bool write_project(const std::string& filename, std::string& error) const;
  void discard_project_restore() noexcept;
  std::string source_unavailable_reason(const std::string& id) const;
  bool available_jobs(std::vector<SimResults::ResultJobEntry>& output, std::string& error) const;
  bool open_file(const std::string& prefix, const std::vector<std::string>& paths,
    const std::optional<std::string>& name, std::string& error);
  bool refresh_source(const std::string& name, std::string& error);
  bool show_only_source(const std::string& name, std::string& error);
  bool open_vtt(const std::string& path, const std::optional<std::string>& name, std::string& error);
  bool open_table(const std::string& source, const std::string& name, bool field, std::string& error);
  bool show_3d(std::string& error);
  Postprocessor::ResultTableData table_data() const;
  std::vector<Postprocessor::ResultTableDescriptor> tables(const std::string& source_id) const;
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
  bool create_plot(const std::string& name,std::string& error);
  bool add_plot_entity(const std::string& plot,const std::string& name,const std::string& source,
    const std::optional<DatasetSelector>& dataset,Postprocessor::EntityKind kind,std::int64_t id,
    std::optional<std::int64_t> ip,const std::string& field,const std::string& component,bool magnitude,
    Postprocessor::PlotReduction reduction,std::string& error);
  bool add_plot_table(const std::string& plot,const std::string& name,const std::string& source,
    const std::string& table,const std::string& x,const std::string& y,const std::string& column,
    const std::string& value,std::string& error);
  bool change_plot(const std::string& name,const std::string& action,const std::string& value,std::string& error);
  bool change_plot_curve(const std::string& plot,const std::string& curve,const std::string& action,std::string& error);
  bool set_plot_axes(const std::string& name,const Postprocessor::PlotAxes& axes,std::string& error);
  bool plot_data(const std::string& id,Postprocessor::PlotData& data,std::string& error) const;
  bool export_plot(const std::string& name,const std::string& path,std::string& error) const;
  bool add_label(const std::string& view, const std::string& name, const std::string& source,
    const std::optional<DatasetSelector>& dataset, Postprocessor::EntityKind kind, std::int64_t id,
    std::optional<std::int64_t> ip, const std::string& field, const std::string& component,
    bool magnitude, std::string& assigned, std::string& error);
  bool change_label(const std::string& view, const std::string& name, const std::string& action,
    const std::string& replacement, const std::array<double,2>& position, std::string& error);
  bool set_labels(const std::string& view, bool extrema, bool visible, std::string& error);
  std::vector<Postprocessor::EvaluatedAnnotation> annotations(const std::string& view) const;
  bool set_pick_mode(Postprocessor::EntityKind kind, std::string& error);
  bool clear_pick(std::string& error);
  bool pick_entity(const std::string& source_name, const DatasetSelector& dataset,
    Postprocessor::EntityKind kind, std::int64_t id, std::optional<std::int64_t> integration_point, std::string& error);
  std::optional<Postprocessor::ProbeResult> probe(std::string& error) const;
  std::optional<Postprocessor::ProbeResult> probe_entity(const Postprocessor::EntityReference& entity, std::string& error) const;
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
  bool frame_available(const std::string& source_id, std::size_t increment_index) const;
  bool set_category_visible(const std::string& name, Postprocessor::DatasetCategory category, bool visible, std::string& error);
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
  CaptureRenderer captureRenderer;
  CaptureSizer captureSizer;
  std::optional<Postprocessor::UserOptions> captureOptions;
  std::optional<std::size_t> resolve_dataset(const std::string& source_id, const DatasetSelector& dataset, std::string& error) const;
  ResultSourceRegistryInterface& providers;
  Postprocessor::PostprocessorCore core;
  std::unique_ptr<Postprocessor::PreparedPostprocessorProject> preparedProject;
  std::unique_ptr<Postprocessor::PostprocessorProjectState> preparedSave;
  Postprocessor::PostprocessorUserOptions options;
};

#endif // POSTPROCESSORAPPLICATIONINTERFACE_HPP
