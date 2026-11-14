#include "PostFilterCommand.hpp"
#include "FilterCommandSupport.hpp"
#include <algorithm>
#include <utility>
PostFilterCommand::PostFilterCommand(std::weak_ptr<PostprocessorApplicationInterface> value) : application(std::move(value))
{
}
std::vector<std::string> PostFilterCommand::get_syntax()
{
  const std::string name = " <string:type='quoted',number='1',label='filter_name'>";
  const std::string view = " [view <string:type='quoted',number='1',label='view_id'>]";
  return {"post filter {show|hide}"+name+view,"post filter rename"+name+" <string:type='quoted',number='1',label='replacement'>",
    "post filter remove"+name,"post filter list"};
}
std::vector<std::string> PostFilterCommand::get_syntax_help()
{
  return get_syntax();
}
std::vector<std::string> PostFilterCommand::get_help()
{
  return {"Named displayed-coordinate filters; originals stay immutable. Creating an output hides its original source representation only in the active view.",
    "Plane normals are normalized. Filter show/hide is per view. Generated cut points are not solver nodes."};
}
bool PostFilterCommand::execute(CubitCommandData& data)
{
  return Postprocessor::execute_filter_command(data,application,std::nullopt);
}
