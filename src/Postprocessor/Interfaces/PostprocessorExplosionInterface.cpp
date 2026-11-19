#include "PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include <algorithm>
bool PostprocessorApplicationInterface::change_explosion(const std::string& source,const std::string& view,
  const std::string& action,double value,const std::string& option,const std::string& block,
  const std::array<double,3>& offset,const std::string& unit,std::string& error)
{
  std::vector<std::string> sources;
  if (!source.empty())
  {
    const auto id = core->source_id_from_name(source);
    if (id.empty())
    {
      error = "No opened source has that name.";
      return false;
    }
    sources.push_back(id);
  }
  auto target = core->session().active_view_id;
  if (!view.empty())
  {
    const auto found = std::find_if(core->session().views.begin(),core->session().views.end(),[&](const auto& item)
    {
      return item.first == view || item.second.name == view;
    });
    if (found == core->session().views.end())
    {
      error = "No viewport has that name or ID.";
      return false;
    }
    target = found->first;
  }
  return core->change_explosion(target,sources,action,value,option,block,offset,unit,error);
}

bool PostprocessorApplicationInterface::explosion_active(const Postprocessor::Representation& display) const
{
  return core->explosion_active(display);
}
