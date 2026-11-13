#ifndef POSTPROCESSOR_CORE_HPP
#define POSTPROCESSOR_CORE_HPP

#include "../session/PostprocessorSession.hpp"
#include "../persistence/PostprocessorProjectState.hpp"
#include "../sources/SourceRuntime.hpp"
#include "../selection/ResultProbe.hpp"
#include "../plots/PlotData.hpp"
#include "../capture/CaptureDefinition.hpp"
#include "Framework/Results/ResultSourceReference.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>

class ResultSourceProviderInterface;

namespace Postprocessor
{
  /** Host-thread owner of Postprocessor definitions and runtime result handles.
   * Callers resolve provider prefixes through Framework, then pass a borrowed
   * provider for one operation. No provider or CalculiX reader pointer is retained.
   * All mutations reject reentry during a provider operation. Query references
   * and iterators are valid only until the next successful mutation; returned
   * owning result handles can outlive refresh, closure and this Core instance.
   * This class owns display definitions but never creates actors, activates
   * workspaces or submits commands. Presentation objects remain GUI-owned.
   */
  class PostprocessorCore
  {
  public:
    PostprocessorCore() = default;

    PostprocessorCore(const PostprocessorCore&) = delete;
    PostprocessorCore& operator=(const PostprocessorCore&) = delete;

    const PostprocessorSession& session() const noexcept;
    bool capture_snapshot(const CaptureDefinition& request,CaptureSnapshot& output,std::string& error) const;
    // Private rendering copies definitions and shares immutable result handles.
    void install_capture_snapshot(const CaptureSnapshot& snapshot);
    std::string plot_id_from_name(const std::string& name) const;
    bool create_plot(const std::string& name, std::string& error);
    bool add_plot_curve(const std::string& plot, const PlotCurveDefinition& curve, std::string& error);
    bool change_plot(const std::string& plot, const std::string& action, const std::string& value, std::string& error);
    bool change_plot_curve(const std::string& plot, const std::string& curve, const std::string& action, std::string& error);
    bool set_plot_axes(const std::string& plot, const PlotAxes& axes, std::string& error);
    bool plot_data(const std::string& plot, PlotData& output, std::string& error) const;
    bool add_label(const std::string& view_id, const ResultAnnotation& requested, std::string& assigned_name, std::string& error);
    bool change_label(const std::string& view_id, const std::string& name, const std::string& action,
      const std::string& replacement, const std::array<double,2>& position, std::string& error);
    bool set_labels(const std::string& view_id, bool extrema, bool visible, std::string& error);
    std::vector<EvaluatedAnnotation> annotations(const std::string& view_id) const;
    bool set_pick_mode(const std::string& view_id, EntityKind kind, std::string& error);
    bool pick_entity(const std::string& view_id, const EntityReference& entity, std::string& error);
    bool clear_pick(const std::string& view_id, std::string& error);
    std::optional<ProbeResult> probe(const std::string& view_id, std::string& error) const;
    std::optional<ProbeResult> probe_entity(const std::string& view_id, const EntityReference& entity, std::string& error) const;
    // Preference for newly created representations only; existing display overrides survive.
    void set_default_palette(ColorPalette palette) noexcept;

    // Restore an empty default session without reusing revision/identity tokens.
    // Reject reset during provider callbacks; failure preserves the old session.
    bool reset(std::string& error);
    bool capture_project(PostprocessorProjectState& output, std::string& error) const;
    bool bind_project(PreparedPostprocessorProject& prepared, std::string& error) const;
    bool restore_project(PreparedPostprocessorProject&& prepared, std::string& error);
    std::string source_unavailable_reason(const std::string& id) const;

    // Exact, case-sensitive lookup; an empty result means no match.
    std::string source_id_from_name(const std::string& name) const;
    std::shared_ptr<const SimResults::VtkResultsData> result_data(const std::string& source_id) const;
    std::vector<Postprocessor::ResultFieldGroup> field_groups(const std::string& source_id, std::optional<std::size_t> current) const;
    bool source_has_mesh(const std::string& source_id) const noexcept;
    std::uint64_t display_revision() const noexcept;
    // Zero-based result sequence index, independently stored per source/view.
    // Preserve visibility and camera; failure is atomic, repeated selection is a no-op.
    bool set_frame(const std::string& source_id, const std::string& view_id,
      std::size_t increment_index, std::string& error);
    bool frame_has_mesh(const std::string& source_id, std::size_t increment_index) const;
    bool set_animation(const std::string& source_id, const std::string& view_id,
      const AnimationDefinition& animation, std::string& error);
    std::optional<std::array<double,2>> all_frame_range(const std::string& source_id,
      const Postprocessor::FieldSelection& selection,std::string& error,
      const std::set<std::size_t>& hidden = {}) const;
    bool set_coloring(const std::string& source_id, const std::string& view_id,
      const ResultColoring& coloring, std::string& error, bool activateResult = false);
    bool set_deformation(const std::string& source_id, const std::string& view_id,
      const ResultDeformation& deformation, std::string& error);
    std::vector<SimResults::Dataset> mesh_datasets(const std::string& source_id) const;
    bool set_dataset_visibility(const std::string& source_id, const std::string& view_id,
      DatasetVisibilityAction action, std::optional<std::size_t> dataset, std::string& error);
    bool set_category_visible(const std::string& source_id, const std::string& view_id,
      DatasetCategory category, bool visible, std::string& error);
    bool set_source_visible(const std::string& source_id, const std::string& view_id,
      bool visible, std::string& error);
    // False with an empty error means no visible mesh; bounds remain unchanged.
    bool visible_bounds(const std::string& view_id, std::array<double, 6>& bounds, std::string& error) const;
    // A valid empty view succeeds without changing the camera or fit tokens.
    bool request_camera_fit(const std::string& view_id, std::string& error);
    bool camera_navigation_ready(const std::string& view_id) const noexcept;
    bool set_camera(const std::string& view_id, const CameraDefinition& camera, std::string& error);
    bool rotate_camera(const std::string& view_id, double yaw, double pitch, std::string& error);
    bool pan_camera(const std::string& view_id, double horizontal, double vertical, std::string& error);
    bool zoom_camera(const std::string& view_id, double factor, std::string& error);
    // GUI completion of an existing request, not a new user action. Reject stale
    // request/geometry revisions and invalid cameras before publishing anything.
    bool complete_camera_fit(const std::string& view_id, std::uint64_t request,
      std::uint64_t revision, const CameraDefinition& camera, std::string& error);

    // nullopt selects an already retained identical job snapshot, otherwise
    // uses the provider's suggested name. An explicit empty/blank name
    // is invalid. Names are preserved verbatim and must be unique in the session.
    // Failure preserves session, runtime handles and the source ID allocator.
    // Provider-side reader/cache changes are outside this transaction boundary.
    bool open_result(
      ResultSourceProviderInterface& provider,
      const SimResults::ResultSourceReference& request,
      const std::optional<std::string>& name,
      std::string& error);

    // Preserves source ID/name and active context; replaces resolved provenance
    // and runtime data and increments the revision only after full validation.
    bool show_only_source(const std::string& source, const std::string& view, std::string& error);
    bool refresh_result(
      ResultSourceProviderInterface& provider,
      const std::string& source_id,
      std::string& error);

    // Selects a registered source, clearing stale active representation context.
    bool select_source(const std::string& source_id, std::string& error);
    bool open_table(const std::string& source_id, const std::string& view_id, const std::string& name, bool field, std::string& error);
    bool show_3d(const std::string& view_id, std::string& error);
    ResultTableData table_data(const std::string& view_id) const;

    // Removes the source and its direct representations. Future feature
    // dependencies require explicit closure rules when features are implemented.
    // Closing the active source clears active source/representation selection.
    bool close_source(const std::string& source_id, std::string& error);

    // Removes sources, runtime data and representations, retaining views/layout.
    bool close_all_sources(std::string& error);

  private:
    bool ensure_source_representation(PostprocessorSession& staged, const std::string& source_id,
      const std::string& view_id, std::size_t increment, bool visible, std::string& error) const;
    bool prepare_initial_display(PostprocessorSession& staged, const std::string& source_id,
      const std::optional<std::size_t>& increment, std::string& error) const;
    PostprocessorSession state;
    std::map<std::string,SavedRepresentationBinding> saved_bindings;
    std::map<std::string,SavedFrameReference> saved_table_frames;
    ColorPalette default_palette = ColoringDefaults::palette;
    std::map<std::string, SourceRuntime> runtime;
    std::uint64_t next_source_number = 1;
    std::uint64_t scene_revision = 1;
    bool source_operation_active = false;
  };
}

#endif // POSTPROCESSOR_CORE_HPP
