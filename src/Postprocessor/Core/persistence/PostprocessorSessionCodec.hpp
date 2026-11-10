#ifndef POSTPROCESSOR_SESSIONCODEC_HPP
#define POSTPROCESSOR_SESSIONCODEC_HPP
#include "PostprocessorProjectState.hpp"
#include <QJsonObject>
namespace Postprocessor
{
  bool validate_project(const PostprocessorProjectState& project, std::string& error);
  QJsonObject project_to_json(const PostprocessorProjectState& project, const std::string& directory);
  // Strict, bounded parse into a detached candidate. Failure preserves output.
  bool project_from_json(const QJsonObject& json, const std::string& directory,
    PostprocessorProjectState& output, std::string& error);
}
#endif
