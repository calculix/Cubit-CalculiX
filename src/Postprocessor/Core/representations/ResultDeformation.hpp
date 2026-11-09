#ifndef POSTPROCESSOR_RESULTDEFORMATION_HPP
#define POSTPROCESSOR_RESULTDEFORMATION_HPP
#include "ResultColoring.hpp"
#include <memory>

namespace Postprocessor
{
  enum class DeformationFieldMode
  {
    Automatic,
    Explicit
  };
  namespace DeformationDefaults
  {
    inline constexpr DeformationFieldMode field_mode = DeformationFieldMode::Automatic;
    inline constexpr double scale = 1.0;
    inline constexpr int scale_preview_delay_ms = 250;
    inline constexpr std::array<double,3> overlay_color = {0.55,0.55,0.55};
  }
  struct ResultDeformation
  {
    bool enabled = false;
    std::optional<FieldSelection> field;
    double scale = DeformationDefaults::scale;
    bool overlay = false;
    // Explicit selection is retained while Automatic resolves independently for
    // each dataset/frame. No solver names are interpreted by Postprocessor.
    DeformationFieldMode field_mode = DeformationDefaults::field_mode;
  };
  bool same_deformation(const ResultDeformation& a, const ResultDeformation& b);
  bool valid_deformation(const ResultDeformation& value, std::string& error);
  bool deformation_vector(const SimResults::Field& field);
  // Borrowed metadata in frame. Automatic requires exactly one role-marked,
  // compatible vector for this dataset; ambiguity never chooses by array order.
  // An explicitly out-of-scope dataset returns null without a warning.
  const SimResults::Field* resolve_deformation_field(const SimResults::Increment& frame,
    std::size_t dataset, const ResultDeformation& settings, std::string& warning);
  // At least one displayed mesh dataset must resolve. Other datasets can use
  // warning/reference fallbacks. Used when enabling or changing active selection.
  bool deformation_available(const SimResults::Increment& frame,
    const ResultDeformation& settings, std::string& error);
  // x' = x + scale*u; caller guarantees compatible coordinate/displacement units.
  // Only declared XYZ columns are used, never derived magnitude columns. Source
  // buffers remain immutable. A null output means use the original mesh. Missing
  // fields/arrays and invalid samples retain original coordinates with a warning.
  // The returned mesh owns fresh double coordinates and shares immutable topology
  // and result arrays. No temporal cache: memory belongs to the current display.
  bool deform_mesh(vtkDataSet* mesh, std::size_t dataset, const SimResults::Increment& frame,
    const ResultDeformation& settings, std::shared_ptr<vtkDataSet>& output,
    std::string& warning, std::string& error);
  // Single-point equivalent for queries: no allocation of a full deformed mesh.
  // Precondition: mesh and the original point index are valid in this snapshot.
  // Missing/invalid displacement returns that point's original coordinates.
  std::array<double,3> deformed_position(vtkDataSet* mesh, std::size_t point, std::size_t dataset,
    const SimResults::Increment& frame, const ResultDeformation& settings);
}
#endif
