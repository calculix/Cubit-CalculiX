#include "PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include <utility>
void PostprocessorApplicationInterface::set_properties_handler(PropertiesHandler handler)
{
  propertiesHandler = std::move(handler);
}
bool PostprocessorApplicationInterface::show_properties(const std::string& page,const std::string& source,std::string& error)
{
  error.clear();
  if (!propertiesHandler)
  {
    error = "Properties require the Postprocessor GUI.";
    return false;
  }
  if (page != "source" && page != "legend" && page != "vectors" && page != "vectorlegend" && page != "clip" && page != "section" && page != "threshold" && page != "explosion" && page != "close" && page != "treeids-on" && page != "treeids-off")
  {
    error = "Unknown properties page.";
    return false;
  }
  std::string id;
  if (page == "source" || ((page == "legend" || page == "vectors" || page == "vectorlegend") && !source.empty()))
  {
    id = core->source_id_from_name(source);
    if (id.empty())
    {
      error = "No opened source has that name.";
      return false;
    }
  }
  if ((page == "legend" || page == "vectors" || page == "vectorlegend") && !id.empty() && core->session().active_source_id != id && !core->select_source(id,error))
  {
    return false;
  }
  return propertiesHandler(page,id,error);
}
bool PostprocessorApplicationInterface::standard_view(const std::string& name,std::string& error)
{
  return core->standard_view(name,error);
}
bool PostprocessorApplicationInterface::set_scale_visible(bool visible,std::string& error)
{
  return core->set_scale_visible(visible,error);
}
bool PostprocessorApplicationInterface::scene_scale(const Postprocessor::Representation& display,double& factor,bool& known,std::string& error) const
{
  return core->scene_scale(display,factor,known,error);
}
