#ifndef POSTPROCESSOR_PROJECTSTATE_HPP
#define POSTPROCESSOR_PROJECTSTATE_HPP
#include "../session/PostprocessorSession.hpp"
#include "../sources/SourceRuntime.hpp"
#include <map>
namespace Postprocessor
{
  namespace PersistenceDefaults
  {
    inline constexpr int schema_version = 1;
    inline constexpr std::size_t maximum_payload_bytes = 64*1024*1024;
    inline constexpr std::size_t maximum_records = 100000;
  }
  struct SavedDatasetReference
  {
    std::string kind, name;
    int model_id = -1;
  };
  struct SavedFrameReference
  {
    bool valid = false;
    int step = -1, increment = -1, analysis_type = -1, mode = -1, subcase = -1, nodal_diameter = -1;
    double time = 0;
  };
  struct SavedRepresentationBinding
  {
    SavedFrameReference frame;
    std::vector<SavedDatasetReference> hidden;
    std::optional<SavedDatasetReference> color_dataset, deformation_dataset;
  };
  struct PostprocessorProjectState
  {
    PostprocessorSession session;
    std::map<std::string,SavedRepresentationBinding> bindings;
    std::map<std::string,SavedFrameReference> table_frames;
    std::map<std::string,std::vector<SimResults::Dataset>> catalogues;
  };
  struct PreparedPostprocessorProject
  {
    PostprocessorProjectState definitions;
    std::map<std::string,SourceRuntime> runtime;
  };
}
#endif
