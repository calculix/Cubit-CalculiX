#pragma once
#include "Postprocessor/Core/tables/ResultTableSelector.hpp"
#include "CubitCommandInterface.hpp"
#include <algorithm>
#include <cctype>
#include <cmath>
namespace Postprocessor
{
  inline std::string table_qualifier_syntax()
  {
    return " [scope <string:type='unquoted',number='1',label='table_scope'>]"
      " [type <string:type='quoted',number='1',label='table_type'>] [entry <value:label='table_entry'>]";
  }
  inline bool read_table_qualifiers(CubitCommandData& data,TableQualifiers& output,std::string& error)
  {
    TableQualifiers candidate;
    if (data.find_keyword("SCOPE"))
    {
      std::string scope;
      data.get_string("table_scope",scope);
      std::transform(scope.begin(),scope.end(),scope.begin(),[](unsigned char c)
      {
        return static_cast<char>(std::tolower(c));
      });
      if (scope != "frame" && scope != "global")
      {
        error = "Table scope must be frame or global.";
        return false;
      }
      candidate.global = scope == "global";
    }
    if (data.find_keyword("TYPE"))
    {
      std::string type;
      if (!data.get_string("table_type",type))
      {
        error = "Table type requires a quoted metadata value (empty is allowed).";
        return false;
      }
      candidate.type = type;
    }
    if (data.find_keyword("ENTRY"))
    {
      double entry = 0;
      if (!data.get_value("table_entry",entry) || !std::isfinite(entry) || entry < 1 || entry > 2147483647 || std::floor(entry) != entry)
      {
        error = "Table entry must be an integer in [1,2147483647].";
        return false;
      }
      candidate.entry = static_cast<std::size_t>(entry);
    }
    output = candidate;
    error.clear();
    return true;
  }
}
