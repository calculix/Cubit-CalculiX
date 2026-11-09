#include "PostDeformCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <exception>
#include <utility>

PostDeformCommand::PostDeformCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}
std::vector<std::string> PostDeformCommand::get_syntax()
{
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  return {"post deform {on|off}"+source,
    "post deform field auto"+source,
    "post deform field <string:type='quoted',number='1',label='field_name'> [dataset <value:label='dataset_id'>]"+source,
    "post deform scale <value:label='scale'>"+source,
    "post deform overlay {on|off}"+source};
}
std::vector<std::string> PostDeformCommand::get_syntax_help()
{
  return {"post deform {on|off|scale VALUE|overlay on/off} [source \"name\"]",
    "post deform field auto [source \"name\"]",
    "post deform field \"name\" [dataset N] [source \"name\"]"};
}
std::vector<std::string> PostDeformCommand::get_help()
{
  return {"Configure deformation of a source in the active view. Source defaults to the active source.",
    "Automatic displacement is the default and resolves role-marked vectors per dataset/frame. Explicit XYZ vectors can override it.",
    "Field selection retains enabled state. Missing or ambiguous automatic vectors retain original coordinates with a warning.",
    "Position = original position + scale * displacement. Scale is finite and nonnegative; units must match.",
    "Off retains settings. Overlay shows the original wireframe only while deformation is enabled."};
}
bool PostDeformCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, error;
    if (!active || (data.find_keyword("SOURCE") && (!data.get_string("source_name",name) || name.empty())))
    {
      PRINT_ERROR("Deformation requires an available application and valid source.\n");
      return false;
    }
    if (data.find_keyword("FIELD"))
    {
      if (data.find_keyword("AUTO"))
      {
        if (!active->deform_auto(name,error))
        {
          PRINT_ERROR("Cannot select automatic displacement: %s\n",error.c_str());
          return false;
        }
        return true;
      }
      std::string field;
      std::optional<std::size_t> dataset;
      double number = 0;
      if (data.find_keyword("DATASET"))
      {
        if (!data.get_value("dataset_id",number) || !std::isfinite(number) ||
            number < 0 || number > 2147483647 || number != std::floor(number))
        {
          PRINT_ERROR("Dataset must be an integer in [0,2147483647].\n");
          return false;
        }
        dataset = static_cast<std::size_t>(number);
      }
      if (!data.get_string("field_name",field) || !active->deform_field(name,field,dataset,error))
      {
        PRINT_ERROR("Cannot select displacement field: %s\n",error.c_str());
        return false;
      }
      return true;
    }
    auto settings = active->deformation(name,error);
    if (!settings)
    {
      PRINT_ERROR("Cannot deform source: %s\n",error.c_str());
      return false;
    }
    if (data.find_keyword("SCALE"))
    {
      if (!data.get_value("scale",settings->scale))
      {
        PRINT_ERROR("Deformation scale is required.\n");
        return false;
      }
    }
    else if (data.find_keyword("OVERLAY"))
    {
      settings->overlay = data.find_keyword("ON");
    }
    else if (data.find_keyword("ON") || data.find_keyword("OFF"))
    {
      settings->enabled = data.find_keyword("ON");
    }
    else
    {
      PRINT_ERROR("Unknown deformation operation.\n");
      return false;
    }
    if (!active->set_deformation(name,*settings,error))
    {
      PRINT_ERROR("Cannot change deformation: %s\n",error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot change deformation: %s\n",exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot change deformation: unknown failure.\n");
  }
  return false;
}
