#pragma once
#include "ExplosionDefinition.hpp"
#include "../filters/FilterRuntime.hpp"
#include <set>
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
  // Native-coordinate geometry. Offsets stored in scene coordinates are divided
  // by sceneScale here; every renderer subsequently applies sceneScale once.
  bool explode_output(const ExplosionState& settings,const Representation& display,const ExplosionReference& reference,
    double sceneScale,std::shared_ptr<const EvaluatedOutput> input,std::shared_ptr<const EvaluatedOutput>& output,std::string& error);
}
