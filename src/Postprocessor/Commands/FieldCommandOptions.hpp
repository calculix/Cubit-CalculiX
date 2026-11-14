#pragma once
#include "Postprocessor/Core/representations/ResultFieldSelector.hpp"
#include "CubitCommandInterface.hpp"
#include <algorithm>
#include <cctype>
namespace Postprocessor
{
  inline std::string field_qualifier_syntax()
  {
    return " [origin <string:type='quoted',number='1',label='field_origin'>]"
      " [association <string:type='unquoted',number='1',label='field_association'>]";
  }
  inline bool read_field_qualifiers(CubitCommandData& data,FieldQualifiers& output,std::string& error)
  {
    FieldQualifiers candidate;
    if (data.find_keyword("ORIGIN"))
    {
      std::string origin;
      if (!data.get_string("field_origin",origin) || origin.empty())
      {
        error = "Origin requires a nonempty quoted result origin such as FRD or DAT.";
        return false;
      }
      candidate.origin = origin;
    }
    if (data.find_keyword("ASSOCIATION"))
    {
      std::string text;
      SimResults::Association association;
      data.get_string("field_association",text);
      std::transform(text.begin(),text.end(),text.begin(),[](unsigned char c)
      {
        return static_cast<char>(std::tolower(c));
      });
      if (!parse_field_association(text,association))
      {
        error = "Field association must be node, element or integrationpoint.";
        return false;
      }
      candidate.association = association;
    }
    output = candidate;
    error.clear();
    return true;
  }
}
