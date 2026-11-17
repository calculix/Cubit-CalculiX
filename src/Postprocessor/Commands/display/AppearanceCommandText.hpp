#pragma once
#include "Postprocessor/Core/representations/DatasetAppearance.hpp"
#include <QString>
class PostprocessorApplicationInterface;
namespace Postprocessor
{
  std::vector<QString> appearance_commands(const PostprocessorApplicationInterface& app,
    const std::vector<AppearanceTarget>& targets,const AppearancePatch& patch,std::string& error);
}
