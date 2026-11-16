#pragma once
#include "ResultColoring.hpp"
namespace Postprocessor
{
  struct FieldQualifiers
  {
    std::optional<std::string> origin;
    std::optional<SimResults::Association> association;
    std::optional<std::string> set_name;
  };
  struct ResultFieldSelector
  {
    std::string name;
    FieldQualifiers qualifiers;
  };
  const char* field_association_name(SimResults::Association association);
  bool parse_field_association(const std::string& text,SimResults::Association& association);
  // Match metadata across the full source catalogue, never first-match or frame order.
  // Empty component leaves component choice to the caller. Legacy keys/array names
  // remain accepted; qualifiers and dataset/component constraints still apply.
  bool resolve_result_field(const std::vector<ResultFieldGroup>& groups,const ResultFieldSelector& selector,
    std::optional<std::size_t> dataset,const std::string& component,bool magnitude,bool vectorOnly,
    ResultFieldGroup& output,std::string& error);
  // Produce the shortest readable selector that resolves to this exact identity.
  // Failure preserves output; never substitutes an opaque key into new journals.
  bool readable_result_field(const std::vector<ResultFieldGroup>& groups,const FieldSelection& field,
    ResultFieldSelector& output,std::string& error);
}
