#include <type_traits>
#include "ResultFieldSelector.hpp"
#include "ResultDeformation.hpp"
#include <algorithm>
#include <set>
namespace Postprocessor
{
  const char* field_association_name(SimResults::Association association)
  {
    switch (association)
    {
      case SimResults::Association::Node: return "node";
      case SimResults::Association::Element: return "element";
      case SimResults::Association::IntegrationPoint: return "integrationpoint";
      case SimResults::Association::Table: return "table";
    }
    return "unknown";
  }
  bool parse_field_association(const std::string& text,SimResults::Association& association)
  {
    for (auto value : {SimResults::Association::Node,SimResults::Association::Element,SimResults::Association::IntegrationPoint})
    {
      if (text == field_association_name(value))
      {
        association = value;
        return true;
      }
    }
    return false;
  }
  namespace
  {
    bool compatible(const ResultFieldGroup& group,const ResultFieldSelector& selector,
      std::optional<std::size_t> dataset,const std::string& component,bool magnitude,bool vectorOnly)
    {
      const auto& field = group.field;
      return field.association != SimResults::Association::Table &&
        (!selector.qualifiers.origin || *selector.qualifiers.origin == field.source) &&
        (!selector.qualifiers.association || *selector.qualifiers.association == field.association) &&
        (!dataset || std::find(group.datasets.begin(),group.datasets.end(),*dataset) != group.datasets.end()) &&
        (!magnitude || field.vector_components.has_value()) && (!vectorOnly || deformation_vector(field)) &&
        (magnitude || component.empty() || std::find(field.components.begin(),field.components.end(),component) != field.components.end());
    }
    std::string describe(const ResultFieldGroup& group)
    {
      const auto& field = group.field;
      std::string text = field.name + " (origin " + ([&]() -> std::common_type_t<decltype("unspecified"), decltype(field.source)> {
        if (field.source.empty()) {
          return "unspecified";
        } else {
          return field.source;
        }
      }()) +
        ", association " + field_association_name(field.association);
      if (!field.set_name.empty())
      {
        text += ", set " + field.set_name;
      }
      text += ", components";
      for (const auto& component : field.components)
      {
        text += " " + component;
      }
      return text + ")";
    }
  }
  bool resolve_result_field(const std::vector<ResultFieldGroup>& groups,const ResultFieldSelector& selector,
    std::optional<std::size_t> dataset,const std::string& component,bool magnitude,bool vectorOnly,
    ResultFieldGroup& output,std::string& error)
  {
    error.clear();
    if (selector.name.empty())
    {
      error = "Provide a result field name before origin/association qualifiers.";
      return false;
    }
    const bool exact = std::any_of(groups.begin(),groups.end(),[&](const auto& group)
    {
      return group.key == selector.name;
    });
    const ResultFieldGroup* match = nullptr;
    std::set<std::string> identities;
    std::string candidates;
    for (const auto& group : groups)
    {
      if (([&]() -> std::common_type_t<decltype(group.key != selector.name), decltype(group.field.name != selector.name && group.field.array_name != selector.name)> {
        if (exact) {
          return group.key != selector.name;
        } else {
          return group.field.name != selector.name && group.field.array_name != selector.name;
        }
      }()) ||
          !compatible(group,selector,dataset,component,magnitude,vectorOnly) || !identities.insert(group.key).second)
      {
        continue;
      }
      match = &group;
      if (identities.size() <= 8)
      {
        candidates += ([&]() -> const char* {
          if (candidates.empty()) {
            return "";
          } else {
            return "; ";
          }
        }()) + describe(group);
      }
    }
    if (identities.size() > 8)
    {
      candidates += "; and " + std::to_string(identities.size()-8) + " more";
    }
    if (identities.size() != 1)
    {
      if (identities.empty()) {
        error = "No spatial field matches the name, dataset, component and qualifiers.";
      } else {
        error = "Ambiguous result field. Specify origin, association or dataset. Candidates: " + candidates;
      }
      return false;
    }
    output = *match;
    return true;
  }
  bool readable_result_field(const std::vector<ResultFieldGroup>& groups,const FieldSelection& field,
    ResultFieldSelector& output,std::string& error)
  {
    ResultFieldGroup selected;
    ResultFieldSelector exact{[&]() -> std::common_type_t<decltype(field.array_name), decltype(field.group_key)> {
      if (field.group_key.empty()) {
        return field.array_name;
      } else {
        return field.group_key;
      }
    }(),{}};
    exact.qualifiers.association = field.association;
    if (!resolve_result_field(groups,exact,field.dataset_id,field.component,field.magnitude,false,selected,error))
    {
      return false;
    }
    ResultFieldSelector candidate{selected.field.name,{}};
    if (candidate.name.empty())
    {
      error = "This field has no readable name for a journal command.";
      return false;
    }
    for (int qualifiers = 0; qualifiers < 4; ++qualifiers)
    {
      candidate.qualifiers = {};
      if ((qualifiers & 1) && !selected.field.source.empty())
      {
        candidate.qualifiers.origin = selected.field.source;
      }
      if (qualifiers & 2)
      {
        candidate.qualifiers.association = selected.field.association;
      }
      ResultFieldGroup resolved;
      std::string ignored;
      if (resolve_result_field(groups,candidate,field.dataset_id,field.component,field.magnitude,false,resolved,ignored) &&
          resolved.key == selected.key)
      {
        output = candidate;
        error.clear();
        return true;
      }
    }
    error = "The field cannot be uniquely journaled by name, origin and association; specify a dataset. " + describe(selected);
    return false;
  }
}
