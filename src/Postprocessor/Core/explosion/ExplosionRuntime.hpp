#pragma once
#include "ExplosionDefinition.hpp"
#include "../filters/FilterRuntime.hpp"
#include <set>
class vtkDataArray;
namespace Postprocessor
{
  inline constexpr const char* explosion_owner_array = "PostprocessorExplosionElement";
  inline constexpr const char* explosion_block_array = "PostprocessorExplosionBlock";
  struct ExplosionReferenceBlock
  {
    ExplosionBlockReference identity;
    std::array<double,6> bounds;
    std::array<double,3> center;
    std::set<std::int64_t> elements;
  };
  struct ExplosionReferenceElement
  {
    std::int64_t id = -1;
    std::size_t block = 0;
    std::array<double,3> center;
  };
  struct ExplosionReference
  {
    std::shared_ptr<const SimResults::VtkResultsData> source;
    std::vector<ExplosionReferenceBlock> blocks;
    std::map<std::int64_t,ExplosionReferenceElement> elements;
    std::map<std::int64_t,std::vector<std::int64_t>> nodes;
  };
  bool prepare_explosion_reference(std::shared_ptr<const SimResults::VtkResultsData> source,ExplosionReference& output,std::string& error);
  bool explosion_enabled(const ExplosionState& state,const std::string& source);
  // Updates only privately owned displayed points using cached displacement
  // arrays. Topology, solver arrays and immutable source buffers are unchanged.
  void apply_explosion_pulse(const EvaluatedOutput& output,const ExplosionState::Pulse& pulse);
  // Key by coordinate array because distinct vtkPoints can share storage.
  using ExplosionPointBuffers = std::set<const vtkDataArray*>;
  void remember_explosion_pulse_buffers(const EvaluatedOutput& output,ExplosionPointBuffers& updated);
  void apply_explosion_pulse_mesh(vtkDataSet* mesh,const ExplosionState::Pulse& pulse,ExplosionPointBuffers& updated);
  // Native-coordinate geometry. Offsets stored in scene coordinates are divided
  // by sceneScale here; every renderer subsequently applies sceneScale once.
  bool explode_output(const ExplosionState& settings,const Representation& display,const ExplosionReference& reference,
    double sceneScale,std::shared_ptr<const EvaluatedOutput> input,std::shared_ptr<const EvaluatedOutput>& output,std::string& error);
  struct CachedExplosionOutput
  {
    std::shared_ptr<const EvaluatedOutput> input, output;
    std::uint64_t revision = 0;
    std::size_t frame = 0;
    ResultDeformation deformation;
    std::set<std::size_t> hidden;
  };
}
