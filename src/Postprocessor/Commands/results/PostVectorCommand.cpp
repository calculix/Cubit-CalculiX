#include "PostVectorCommand.hpp"
#include "Postprocessor/Commands/FieldCommandOptions.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <exception>
#include <utility>
PostVectorCommand::PostVectorCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}
std::vector<std::string> PostVectorCommand::get_syntax()
{
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  return {"post vector {on|off}"+source,
    "post vector field <string:type='quoted',number='1',label='field_name'>"+Postprocessor::field_qualifier_syntax()+" [dataset <value:label='dataset_id'>]"+source,
    "post vector length {proportional|uniform}"+source,
    "post vector scale <value:label='vector_scale'>"+source,
    "post vector samples {surface|all}"+source,
    "post vector density maximum <value:label='vector_maximum'>"+source,
    "post vector density every <value:label='vector_stride'>"+source,
    "post vector color {magnitude|solid}"+source};
}
std::vector<std::string> PostVectorCommand::get_syntax_help()
{
  return {"post vector field \"DISP\" [origin \"FRD\"] [association node|element|integrationpoint] [set \"name\"] [dataset N] [source \"name\"]",
    "post vector {on|off|length proportional/uniform|scale VALUE|samples surface/all|density maximum N|density every N|color magnitude/solid} [source \"name\"]"};
}
std::vector<std::string> PostVectorCommand::get_help()
{
  return {"Draw declared spatial XYZ vectors in the active view. Select a field before enabling; off retains settings.",
    "Arrow tails follow displayed geometry, including deformation and filters. Components remain in global directions.",
    "Automatic length is 5% of the source diagonal. Proportional lengths use the all-frame maximum magnitude.",
    "Default: off, proportional, surface samples, at most 2000 arrows, magnitude colors. Maximum zero selects all; every N uses stable identity sampling."};
}
bool PostVectorCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name,error;
    if (!active || (data.find_keyword("SOURCE") && (!data.get_string("source_name",name) || name.empty())))
    {
      PRINT_ERROR("Vector plots require an available source.\n");
      return false;
    }
    if (data.find_keyword("FIELD"))
    {
      std::string field;
      std::optional<std::size_t> dataset;
      Postprocessor::FieldQualifiers qualifiers;
      double number = 0;
      if (data.find_keyword("DATASET"))
      {
        if (!data.get_value("dataset_id",number) || !std::isfinite(number) || number < 0 || number > 2147483647 || number != std::floor(number))
        {
          PRINT_ERROR("Vector dataset must be an integer in [0,2147483647].\n");
          return false;
        }
        dataset = static_cast<std::size_t>(number);
      }
      if (!data.get_string("field_name",field) || !Postprocessor::read_field_qualifiers(data,qualifiers,error) ||
          !active->vector_field(name,field,dataset,error,qualifiers))
      {
        PRINT_ERROR("Cannot select vector field: %s\n",error.c_str());
        return false;
      }
      return true;
    }
    auto settings = active->vector_plot(name,error);
    if (!settings)
    {
      PRINT_ERROR("Cannot configure vectors: %s\n",error.c_str());
      return false;
    }
    if (data.find_keyword("SCALE"))
    {
      if (!data.get_value("vector_scale",settings->scale))
      {
        PRINT_ERROR("A vector scale is required.\n");
        return false;
      }
    }
    else if (data.find_keyword("DENSITY"))
    {
      double number = 0;
      const bool maximum = data.find_keyword("MAXIMUM");
      std::string label = "vector_stride";
      if (maximum)
      {
        label = "vector_maximum";
      }
      if (!data.get_value(label.c_str(),number) || !std::isfinite(number) || number < 0 || number > 1000000 || number != std::floor(number) || (!maximum && number == 0))
      {
        PRINT_ERROR("Vector density must be an integer at most 1000000; stride must be positive.\n");
        return false;
      }
      if (maximum)
      {
        settings->maximum = static_cast<unsigned int>(number);
      }
      else
      {
        settings->stride = static_cast<unsigned int>(number);
      }
    }
    else if (data.find_keyword("LENGTH"))
    {
      settings->proportional = data.find_keyword("PROPORTIONAL");
    }
    else if (data.find_keyword("SAMPLES"))
    {
      settings->surface_only = data.find_keyword("SURFACE");
    }
    else if (data.find_keyword("COLOR"))
    {
      settings->magnitude_color = data.find_keyword("MAGNITUDE");
    }
    else
    {
      settings->enabled = data.find_keyword("ON");
    }
    if (!active->set_vector_plot(name,*settings,error))
    {
      PRINT_ERROR("Cannot configure vectors: %s\n",error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot configure vectors: %s\n",exception.what());
  }
  catch (...)
  {
    PRINT_ERROR("Cannot configure vectors: unknown failure.\n");
  }
  return false;
}
