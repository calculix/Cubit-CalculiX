#ifndef POSTPROCESSOR_PICKCOMMANDTEXT_HPP
#define POSTPROCESSOR_PICKCOMMANDTEXT_HPP
#include "Postprocessor/Core/selection/EntityReference.hpp"
class PostprocessorApplicationInterface;
namespace Postprocessor
{
  bool pick_command_text(const PostprocessorApplicationInterface& application,
    const EntityReference& entity, std::string& command, std::string& error);
}
#endif
