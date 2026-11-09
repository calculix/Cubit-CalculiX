#include "PostColorCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <exception>
#include <utility>

PostColorCommand::PostColorCommand(std::weak_ptr<PostprocessorApplicationInterface> application)
  : application(std::move(application))
{
}
std::vector<std::string> PostColorCommand::get_syntax()
{
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  const std::string field = "post color field <string:type='quoted',number='1',label='field_name'> component ";
  const std::string dataset = " [dataset <value:label='dataset_id'>]";
  return {
    field + "magnitude" + dataset + source,
    field + "<string:type='quoted',number='1',label='component_name'>" + dataset + source,
    "post color solid" + source,
    "post color palette {viridis|bluewhitered|turbo|rainbow|grayscale}" + source,
    "post color reverse {on|off}" + source,
    "post color bands continuous" + source,
    "post color bands <value:label='bands_number'>" + source,
    "post color range {auto|allframes|symmetric}" + source,
    "post color range <value:label='range_min'> <value:label='range_max'>" + source,
    "post color limit {minimum|maximum} {on|off}" + source,
    "post color limit {minimum|maximum} <value:label='limit_value'> color "
      "<value:label='red'> <value:label='green'> <value:label='blue'>" + source};
}
std::vector<std::string> PostColorCommand::get_syntax_help()
{
  return {"post color field \"name\" component {magnitude|\"component\"} [dataset N] [source \"name\"]",
    "post color {solid|palette NAME|reverse on/off|bands continuous/N|range auto/allframes/symmetric/MIN MAX} [source \"name\"]",
    "post color limit minimum/maximum {on|off|VALUE color R G B} [source \"name\"]"};
}
std::vector<std::string> PostColorCommand::get_help()
{
  return {"Configure result coloring in the active view. Source defaults to the active source.",
    "Limits override palette colors strictly below/above their thresholds; equality uses the palette.",
    "Limit off preserves the saved threshold and RGB color; on re-enables it. RGB values are in [0,1]."};
}
bool PostColorCommand::execute(CubitCommandData& data)
{
  try
  {
    const auto active = application.lock();
    std::string name, error;
    if (!active || (data.find_keyword("SOURCE") && (!data.get_string("source_name", name) || name.empty())))
    {
      PRINT_ERROR("Coloring requires an available application and valid source.\n");
      return false;
    }
    if (data.find_keyword("FIELD"))
    {
      std::string field, component;
      double domain = 0;
      std::optional<std::size_t> dataset;
      if (data.find_keyword("DATASET"))
      {
        if (!data.get_value("dataset_id", domain) || !std::isfinite(domain) || domain < 0 || domain != std::floor(domain) || domain > 2147483647)
        {
          PRINT_ERROR("Dataset must be an integer in [0,2147483647].\n");
          return false;
        }
        dataset = static_cast<std::size_t>(domain);
      }
      const bool magnitude = data.find_keyword("MAGNITUDE");
      if (!data.get_string("field_name", field) || (!magnitude && !data.get_string("component_name", component)) ||
          !active->color_field(name, field, component, magnitude, dataset, error))
      {
        PRINT_ERROR("Cannot select result color field: %s\n", error.c_str());
        return false;
      }
      return true;
    }
    auto settings = active->coloring(name, error);
    if (!settings)
    {
      PRINT_ERROR("Cannot color source: %s\n", error.c_str());
      return false;
    }
    auto& c = *settings;
    using namespace Postprocessor;
    if (data.find_keyword("SOLID"))
    {
      c.field.reset();
    }
    else if (data.find_keyword("LIMIT"))
    {
      auto& limit = data.find_keyword("MINIMUM") ? c.minimum : c.maximum;
      if (data.find_keyword("OFF") || data.find_keyword("ON"))
      {
        limit.enabled = data.find_keyword("ON");
      }
      else if (data.get_value("limit_value", limit.value) && data.get_value("red", limit.color[0]) &&
        data.get_value("green", limit.color[1]) && data.get_value("blue", limit.color[2]))
      {
        limit.enabled = true;
      }
      else
      {
        PRINT_ERROR("Limit requires a threshold and RGB color.\n");
        return false;
      }
    }
    else if (data.find_keyword("PALETTE"))
    {
      if (data.find_keyword("VIRIDIS"))
      {
        c.palette = ColorPalette::Viridis;
      }
      else if (data.find_keyword("BLUEWHITERED"))
      {
        c.palette = ColorPalette::BlueWhiteRed;
      }
      else if (data.find_keyword("TURBO"))
      {
        c.palette = ColorPalette::Turbo;
      }
      else if (data.find_keyword("RAINBOW"))
      {
        c.palette = ColorPalette::Rainbow;
      }
      else
      {
        c.palette = ColorPalette::Grayscale;
      }
    }
    else if (data.find_keyword("REVERSE"))
    {
      c.reversed = data.find_keyword("ON");
    }
    else if (data.find_keyword("BANDS"))
    {
      double bands = 0;
      if (data.find_keyword("CONTINUOUS"))
      {
        c.bands.reset();
      }
      else if (data.get_value("bands_number", bands) && std::isfinite(bands) && bands >= 2 && bands <= 256 && bands == std::floor(bands))
      {
        c.bands = static_cast<unsigned int>(bands);
      }
      else
      {
        PRINT_ERROR("Band count must be an integer from 2 to 256.\n");
        return false;
      }
    }
    else if (data.find_keyword("RANGE"))
    {
      if (data.find_keyword("AUTO"))
      {
        c.range_mode = ColorRangeMode::Automatic;
      }
      else if (data.find_keyword("ALLFRAMES"))
      {
        c.range_mode = ColorRangeMode::AllFrames;
      }
      else if (data.find_keyword("SYMMETRIC"))
      {
        c.range_mode = ColorRangeMode::Symmetric;
      }
      else if (data.get_value("range_min", c.fixed_range[0]) && data.get_value("range_max", c.fixed_range[1]))
      {
        c.range_mode = ColorRangeMode::Fixed;
      }
      else
      {
        PRINT_ERROR("Range requires auto, allframes, symmetric or two finite bounds.\n");
        return false;
      }
    }
    else
    {
      PRINT_ERROR("Unknown result color operation.\n");
      return false;
    }
    if (!active->set_coloring(name, c, error))
    {
      PRINT_ERROR("Cannot change result coloring: %s\n", error.c_str());
      return false;
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    PRINT_ERROR("Cannot change result coloring: %s\n", exception.what());
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Cannot change result coloring: unknown failure.\n");
    return false;
  }
}
