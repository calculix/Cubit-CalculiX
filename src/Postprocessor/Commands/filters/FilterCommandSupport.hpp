#pragma once
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitCommandInterface.hpp"
#include "CubitMessage.hpp"
#include <exception>
#include <algorithm>
namespace Postprocessor
{
  inline std::string plane_syntax()
  {
    return " origin <value:label='ox'> <value:label='oy'> <value:label='oz'> normal <value:label='nx'> <value:label='ny'> <value:label='nz'>";
  }
  inline bool execute_filter_command(CubitCommandData& data,const std::weak_ptr<PostprocessorApplicationInterface>& owner,
    std::optional<FilterKind> kind)
  {
    std::string error;
    try
    {
      const auto application = owner.lock();
      if (!application)
      {
        PRINT_ERROR("Postprocessor filters are unavailable.\n");
        return false;
      }
      std::string name,source,view,value;
      data.get_string("filter_name",name);
      data.get_string("source_name",source);
      data.get_string("view_id",view);
      data.get_string("replacement",value);
      PlaneDefinition plane;
      if (data.find_keyword("LIST"))
      {
        for (const auto& entry : application->session().filters)
        {
          const auto& filter = entry.second;
          const auto input = application->session().sources.find(filter.input.object_id);
          PRINT_INFO("%s | %s | source %s\n",filter.name.c_str(),filter.kind == FilterKind::Clip ? "Clip" : "Section",
            input == application->session().sources.end() ? "unavailable" : input->second.name.c_str());
        }
        return true;
      }
      if (data.find_keyword("CREATE") || data.find_keyword("PLANE"))
      {
        if (!data.get_value("ox",plane.origin[0]) || !data.get_value("oy",plane.origin[1]) || !data.get_value("oz",plane.origin[2]) ||
            !data.get_value("nx",plane.normal[0]) || !data.get_value("ny",plane.normal[1]) || !data.get_value("nz",plane.normal[2]))
        {
          error = "A plane requires three origin and three normal coordinates.";
        }
      }
      bool accepted = false;
      if (error.empty() && data.find_keyword("CREATE") && kind)
      {
        FilterDefinition requested;
        requested.name = name;
        requested.kind = *kind;
        requested.plane = plane;
        requested.keep = data.find_keyword("NEGATIVE") ? ClipSide::Negative : ClipSide::Positive;
        accepted = application->create_filter(requested,source,error);
      }
      else if (error.empty())
      {
        if (kind)
        {
          const auto& filters = application->session().filters;
          const auto found = std::find_if(filters.begin(),filters.end(),[&](const auto& entry)
          {
            return entry.second.name == name && entry.second.kind == *kind;
          });
          if (found == filters.end())
          {
            error = "No filter of this kind has that name.";
          }
        }
        const std::string action = data.find_keyword("PLANE") ? "plane" : data.find_keyword("KEEP") ? "keep" :
          data.find_keyword("SHOW") ? "show" : data.find_keyword("HIDE") ? "hide" : data.find_keyword("RENAME") ? "rename" : "remove";
        if (action == "keep")
        {
          value = data.find_keyword("NEGATIVE") ? "negative" : "positive";
        }
        if (error.empty())
        {
          accepted = application->change_filter(name,action,view,plane,value,error);
        }
      }
      if (accepted)
      {
        return true;
      }
    }
    catch (const std::exception& exception)
    {
      error = exception.what();
    }
    PRINT_ERROR("Cannot change filter: %s\n",error.empty() ? "Invalid filter arguments." : error.c_str());
    return false;
  }
}
