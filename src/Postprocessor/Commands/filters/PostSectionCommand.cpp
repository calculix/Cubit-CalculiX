#include "PostSectionCommand.hpp"
#include "FilterCommandSupport.hpp"
#include <algorithm>
#include <utility>
PostSectionCommand::PostSectionCommand(std::weak_ptr<PostprocessorApplicationInterface> value) : application(std::move(value))
{
}
std::vector<std::string> PostSectionCommand::get_syntax()
{
  const std::string name = " <string:type='quoted',number='1',label='filter_name'>";
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  return {"post section create"+name+source+Postprocessor::plane_syntax(),
    "post section plane"+name+Postprocessor::plane_syntax()};
}
std::vector<std::string> PostSectionCommand::get_syntax_help()
{
  return get_syntax();
}
std::vector<std::string> PostSectionCommand::get_help()
{
  return {"Named displayed-coordinate filters; originals stay immutable. Creating an output hides its original source representation only in the active view.",
    "Plane normals are normalized. Filter show/hide is per view. Generated cut points are not solver nodes."};
}
bool PostSectionCommand::execute(CubitCommandData& data)
{
  return Postprocessor::execute_filter_command(data,application,Postprocessor::FilterKind::Section);
}
