#ifndef POSTPROCESSOR_SESSION_HPP
#define POSTPROCESSOR_SESSION_HPP

#include "../sources/DataSource.hpp"
#include "../layouts/LayoutDefinition.hpp"
#include "../representations/Representation.hpp"
#include "../views/ViewDefinition.hpp"

#include <map>
#include <string>

namespace Postprocessor
{
  // Persistent application definitions, independent of Qt and VTK runtime objects.
  // Sources, views and representations have stable, nonempty IDs unique within
  // their respective session registries. Layout node IDs are layout-local.
  // All registry keys equal object IDs; editable display names are not identities.
  // Source names are nonempty, unique within the session and matched exactly,
  // including case, for command addressing. Name collisions must be rejected.
  // Restoring a session must preserve IDs and resolve references before publication.
  // These aggregates describe state, not a public GUI mutation interface:
  // PostprocessorCore validates operations and exposes read-only state to the GUI.
  struct PostprocessorSession
  {
    std::map<std::string, DataSource> sources;
    std::map<std::string, ViewDefinition> views =
    {
      {"view1", ViewDefinition{"view1", "3D View 1", std::nullopt}}
    };
    std::map<std::string, Representation> representations;
    LayoutDefinition layout;

    // Empty IDs mean no active object. All active IDs are session-scoped.
    // Short commands may use active context; command handlers resolve it to
    // explicit IDs before calling Core operations.
    std::string active_source_id;
    std::string active_representation_id;
    std::string active_view_id = "view1";

    // Publish staged definitions by exchanging existing containers directly.
    // Generic std::swap would move-construct a session; MSVC's map move
    // constructor may allocate. Keep every member in the conditional guarantee.
    void swap(PostprocessorSession& other) noexcept(
      noexcept(sources.swap(other.sources)) &&
      noexcept(views.swap(other.views)) &&
      noexcept(representations.swap(other.representations)) &&
      noexcept(layout.root_node_id.swap(other.layout.root_node_id)) &&
      noexcept(layout.nodes.swap(other.layout.nodes)) &&
      noexcept(active_source_id.swap(other.active_source_id)) &&
      noexcept(active_representation_id.swap(other.active_representation_id)) &&
      noexcept(active_view_id.swap(other.active_view_id)))
    {
      sources.swap(other.sources);
      views.swap(other.views);
      representations.swap(other.representations);
      layout.root_node_id.swap(other.layout.root_node_id);
      layout.nodes.swap(other.layout.nodes);
      active_source_id.swap(other.active_source_id);
      active_representation_id.swap(other.active_representation_id);
      active_view_id.swap(other.active_view_id);
    }
  };
}

#endif // POSTPROCESSOR_SESSION_HPP
