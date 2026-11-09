#ifndef POSTPROCESSOR_ENTITYREFERENCE_HPP
#define POSTPROCESSOR_ENTITYREFERENCE_HPP
#include <cstdint>
#include <cstddef>
#include <optional>
#include <string>

namespace Postprocessor
{
  enum class EntityKind
  {
    Node,
    Element,
    IntegrationPoint
  };
  struct EntityReference
  {
    std::string source_id;
    std::size_t dataset_id = 0;
    EntityKind kind = EntityKind::Element;
    // Original result identity, never a VTK tuple or tessellation index.
    std::int64_t entity_id = 0;
    std::optional<std::int64_t> integration_point;
  };
  inline bool same_entity(const EntityReference& a, const EntityReference& b)
  {
    return a.source_id == b.source_id && a.dataset_id == b.dataset_id && a.kind == b.kind &&
      a.entity_id == b.entity_id && a.integration_point == b.integration_point;
  }
}
#endif
