#ifndef POSTPROCESSOR_PROJECTPERSISTENCE_HPP
#define POSTPROCESSOR_PROJECTPERSISTENCE_HPP
#include "PostprocessorProjectState.hpp"
namespace Postprocessor
{
  class PostprocessorProjectPersistence
  {
  public:
    bool read(const std::string& filename, PostprocessorProjectState& output, std::string& error) const;
    bool write(const std::string& filename, const PostprocessorProjectState& state, std::string& error) const;
  };
}
#endif
