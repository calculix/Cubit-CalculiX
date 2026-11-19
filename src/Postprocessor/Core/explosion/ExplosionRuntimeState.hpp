#pragma once
#include "ExplosionRuntime.hpp"
#include "../application/PostprocessorCore.hpp"

namespace Postprocessor
{
  struct PostprocessorCore::ExplosionRuntimeState
  {
    struct CachedExplosionOutput
    {
      std::shared_ptr<const EvaluatedOutput> input, output;
      std::uint64_t revision = 0;
      std::size_t frame = 0;
      ResultDeformation deformation;
      std::set<std::size_t> hidden;
    };
    std::map<std::string,std::shared_ptr<ExplosionReference>> references;
    std::map<std::string,CachedExplosionOutput> outputs;
  };
}
