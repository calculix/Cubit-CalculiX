#ifndef POSTPROCESSOR_ANNOTATIONDEFINITION_HPP
#define POSTPROCESSOR_ANNOTATIONDEFINITION_HPP
#include "../selection/ResultProbe.hpp"
#include <map>
namespace Postprocessor
{
  namespace AnnotationDefaults
  {
    inline constexpr bool visible = true;
    inline constexpr bool extrema_visible = false;
    inline constexpr std::array<double,2> position = {0.75,0.75};
    inline constexpr std::array<double,2> minimum_position = {0.05,0.15};
    inline constexpr std::array<double,2> maximum_position = {0.65,0.85};
    inline constexpr int font_size = 12;
  }
  struct ResultAnnotation
  {
    std::string name;
    EntityReference entity;
    FieldSelection field;
    // Durable dataset identity; dataset_id is only the resolved runtime ordinal.
    std::string dataset_kind, dataset_name;
    int model_id = -1;
    bool visible = AnnotationDefaults::visible;
    // Normalized drawable viewport coordinates, origin at bottom-left, [0,1].
    std::array<double,2> position = AnnotationDefaults::position;
  };
  struct AnnotationState
  {
    bool visible = AnnotationDefaults::visible;
    bool extrema_visible = AnnotationDefaults::extrema_visible;
    std::map<std::string,ResultAnnotation> labels;
    std::array<double,2> minimum_position = AnnotationDefaults::minimum_position;
    std::array<double,2> maximum_position = AnnotationDefaults::maximum_position;
    std::uint64_t revision = 1;
  };
  struct EvaluatedAnnotation
  {
    std::string name, text;
    std::optional<std::array<double,3>> anchor;
    std::array<double,2> position;
  };
  bool valid_annotation_position(const std::array<double,2>& position);
  bool valid_annotation_name(const std::string& name);
  // Element leaders use the arithmetic mean of original cell-node positions
  // after deformation. This is a geometric anchor, not a result average.
  ProbeResult annotation_probe(const SimResults::VtkResultsData& data, const Representation& display,
    const EntityReference& entity);
}
#endif
