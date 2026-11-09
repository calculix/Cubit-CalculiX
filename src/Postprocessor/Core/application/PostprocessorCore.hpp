#ifndef POSTPROCESSOR_CORE_HPP
#define POSTPROCESSOR_CORE_HPP

#include "../session/PostprocessorSession.hpp"
#include "../sources/SourceRuntime.hpp"
#include "../selection/ResultProbe.hpp"
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
    bool set_pick_mode(const std::string& view_id, EntityKind kind, std::string& error);
    bool pick_entity(const std::string& view_id, const EntityReference& entity, std::string& error);
    bool clear_pick(const std::string& view_id, std::string& error);
    std::optional<ProbeResult> probe(const std::string& view_id, std::string& error) const;
    // Preference for newly created representations only; existing display overrides survive.
    void set_default_palette(ColorPalette palette) noexcept;

    // Restore an empty default session without reusing revision/identity tokens.
    // Reject reset during provider callbacks; failure preserves the old session.
    bool reset(std::string& error);

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
      const ResultColoring& coloring, std::string& error);
    bool set_deformation(const std::string& source_id, const std::string& view_id,
      const ResultDeformation& deformation, std::string& error);
    std::vector<SimResults::Dataset> mesh_datasets(const std::string& source_id) const;
    bool set_dataset_visibility(const std::string& source_id, const std::string& view_id,
      DatasetVisibilityAction action, std::optional<std::size_t> dataset, std::string& error);
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
    bool refresh_result(
      ResultSourceProviderInterface& provider,
      const std::string& source_id,
      std::string& error);

    // Selects a registered source, clearing stale active representation context.
    bool select_source(const std::string& source_id, std::string& error);

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
    ColorPalette default_palette = ColoringDefaults::palette;
    std::map<std::string, SourceRuntime> runtime;
    std::uint64_t next_source_number = 1;
    std::uint64_t scene_revision = 1;
    bool source_operation_active = false;
  };
}

#endif // POSTPROCESSOR_CORE_HPP
