#include "PostVectorLegendCommand.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <cmath>
#include <cctype>
#include <exception>
#include <utility>
PostVectorLegendCommand::PostVectorLegendCommand(std::weak_ptr<PostprocessorApplicationInterface> app)
  : application(std::move(app))
{
}
std::vector<std::string> PostVectorLegendCommand::get_syntax()
{
  const std::string source = " [source <string:type='quoted',number='1',label='source_name'>]";
  return {"post vector legend {on|off|defaults|auto}"+source,
    "post vector legend position <value:label='x'> <value:label='y'>"+source,
    "post vector legend palette reset"+source,
    "post vector legend palette {low|high} color <value:label='red'> <value:label='green'> <value:label='blue'>"+source,
    "post vector legend palette stop <value:label='stop'> color <value:label='red'> <value:label='green'> <value:label='blue'>"+source,
    "post vector legend palette {viridis|bluewhitered|turbo|rainbow|grayscale}"+source,
    "post vector legend reverse {on|off}"+source,
    "post vector legend bands continuous"+source,
    "post vector legend bands <value:label='count'>"+source,
    "post vector legend range {auto|allframes|symmetric}"+source,
    "post vector legend range <value:label='low'> <value:label='high'>"+source,
    "post vector legend limits minimum <string:type='quoted',number='1',label='minimum_state'> <value:label='minimum_value'> color <value:label='minimum_red'> <value:label='minimum_green'> <value:label='minimum_blue'> maximum <string:type='quoted',number='1',label='maximum_state'> <value:label='maximum_value'> color <value:label='maximum_red'> <value:label='maximum_green'> <value:label='maximum_blue'>"+source};
}
std::vector<std::string> PostVectorLegendCommand::get_syntax_help()
{
  return {"post vector legend on|off|defaults|auto [source \"name\"]",
    "post vector legend palette low|high color R G B [source \"name\"]",
    "post vector legend position X Y; palette NAME; reverse on|off; bands continuous|N; range auto|allframes|symmetric|LOW HIGH",
    "post vector legend limits minimum \"on|off\" VALUE color R G B maximum \"on|off\" VALUE color R G B [source \"name\"]"};
}
std::vector<std::string> PostVectorLegendCommand::get_help()
{
  return {"Independent vector magnitude coloring and legend. Defaults retain the vector field and arrow settings.",
    "Position uses normalized bottom-left viewport coordinates. Auto restores placement. Allframes uses zero to the all-frame maximum magnitude.",
    "Numeric bounds and limits use the vector result's current presentation units. Both limits are validated atomically."};
}
bool PostVectorLegendCommand::execute(CubitCommandData& data)
{
  std::string error;
  try
  {
    const auto app = application.lock();
    std::string source;
    if (!app || (data.find_keyword("SOURCE") && (!data.get_string("source_name",source) || source.empty())))
    {
      PRINT_ERROR("Vector legend requires an available source.\n");
      return false;
    }
    auto settings = app->vector_plot(source,error);
    if (!settings)
    {
      PRINT_ERROR("Cannot find vector legend: %s\n",error.c_str());
      return false;
    }
    auto& legend = settings->legend;
    using namespace Postprocessor;
    if (data.find_keyword("DEFAULTS"))
    {
      legend = ResultColoring{};
    }
    else if (data.find_keyword("POSITION"))
    {
      std::array<double,2> position;
      if (!data.get_value("x",position[0]) || !data.get_value("y",position[1]))
      {
        PRINT_ERROR("Vector legend position requires X and Y.\n");
        return false;
      }
      legend.legend_position = position;
    }
    else if (data.find_keyword("PALETTE") && (data.find_keyword("LOW") || data.find_keyword("HIGH")))
    {
      std::array<double,3> rgb;
      if (!data.get_value("red",rgb[0]) || !data.get_value("green",rgb[1]) || !data.get_value("blue",rgb[2]))
      {
        PRINT_ERROR("Palette endpoint requires three RGB channels.\n");
        return false;
      }
      if (!set_palette_endpoint(legend,data.find_keyword("HIGH"),rgb,error))
      {
        PRINT_ERROR("Cannot edit palette endpoint: %s\n",error.c_str());
        return false;
      }
    }
    else if (data.find_keyword("STOP"))
    {
      double stop = 0;
      std::array<double,3> rgb;
      if (!data.get_value("stop",stop) || !std::isfinite(stop) || stop < 1 || stop > ColoringDefaults::maximum_bands || stop != std::floor(stop) ||
          !data.get_value("red",rgb[0]) || !data.get_value("green",rgb[1]) || !data.get_value("blue",rgb[2]) ||
          !set_palette_stop(legend,static_cast<std::size_t>(stop),rgb,error))
      {
        PRINT_ERROR("Cannot edit palette stop: %s\n",error.c_str());
        return false;
      }
    }
    else if (data.find_keyword("RESET"))
    {
      reset_custom_palette(legend);
    }
    else if (data.find_keyword("PALETTE"))
    {
      reset_custom_palette(legend);
      for (const auto palette : {ColorPalette::Viridis,ColorPalette::BlueWhiteRed,ColorPalette::Turbo,ColorPalette::Rainbow,ColorPalette::Grayscale})
      {
        std::string keyword = color_palette_name(palette);
        for (auto& c : keyword)
        {
          c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        }
        if (data.find_keyword(keyword.c_str()))
        {
          legend.palette = palette;
        }
      }
    }
    else if (data.find_keyword("REVERSE"))
    {
      legend.reversed = data.find_keyword("ON");
    }
    else if (data.find_keyword("BANDS"))
    {
      if (data.find_keyword("CONTINUOUS"))
      {
        legend.bands.reset();
      }
      else
      {
        double count = 0;
        if (!data.get_value("count",count) || !std::isfinite(count) || count < 2 || count > ColoringDefaults::maximum_bands || count != std::floor(count))
        {
          PRINT_ERROR("Vector legend bands require an integer from 2 to 256.\n");
          return false;
        }
        legend.bands = static_cast<unsigned int>(count);
      }
    }
    else if (data.find_keyword("RANGE"))
    {
      legend.range_mode = ColorRangeMode::Fixed;
      if (data.find_keyword("AUTO"))
      {
        legend.range_mode = ColorRangeMode::Automatic;
      }
      else if (data.find_keyword("ALLFRAMES"))
      {
        legend.range_mode = ColorRangeMode::AllFrames;
      }
      else if (data.find_keyword("SYMMETRIC"))
      {
        legend.range_mode = ColorRangeMode::Symmetric;
      }
      else if (!data.get_value("low",legend.fixed_range[0]) || !data.get_value("high",legend.fixed_range[1]))
      {
        PRINT_ERROR("Vector legend range requires LOW and HIGH.\n");
        return false;
      }
    }
    else if (data.find_keyword("LIMITS"))
    {
      for (const auto& pair : {std::pair<const char*,ColorLimit*>{"minimum",&legend.minimum},{"maximum",&legend.maximum}})
      {
        const std::string prefix(pair.first);
        std::string state;
        if (!data.get_string((prefix+"_state").c_str(),state) || (state != "on" && state != "off") ||
            !data.get_value((prefix+"_value").c_str(),pair.second->value) ||
            !data.get_value((prefix+"_red").c_str(),pair.second->color[0]) ||
            !data.get_value((prefix+"_green").c_str(),pair.second->color[1]) ||
            !data.get_value((prefix+"_blue").c_str(),pair.second->color[2]))
        {
          PRINT_ERROR("Vector limits require on/off states, values and RGB colors.\n");
          return false;
        }
        pair.second->enabled = state == "on";
      }
    }
    else if (data.find_keyword("AUTO"))
    {
      legend.legend_position.reset();
    }
    else
    {
      legend.legend_visible = data.find_keyword("ON");
    }
    if (app->set_vector_plot(source,*settings,error))
    {
      return true;
    }
  }
  catch (const std::exception& exception)
  {
    error = exception.what();
  }
  PRINT_ERROR("Cannot change vector legend: %s\n",error.c_str());
  return false;
}
