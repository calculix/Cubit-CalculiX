#include "ResultTableSelector.hpp"
#include <algorithm>
#include <set>
namespace Postprocessor
{
  bool resolve_result_table(const std::vector<ResultTableDescriptor>& catalogue,const ResultTableSelector& selector,
    ResultTableDescriptor& output,std::string& error)
  {
    error.clear();
    const auto& qualifiers = selector.qualifiers;
    if (selector.name.empty() || (qualifiers.entry && *qualifiers.entry == 0))
    {
      error = "Provide a table name and a positive entry number.";
      return false;
    }
    const bool exact = std::any_of(catalogue.begin(),catalogue.end(),[&](const auto& table)
    {
      return table.key == selector.name;
    });
    const ResultTableDescriptor* found = nullptr;
    std::set<std::string> identities;
    std::string candidates;
    for (const auto& table : catalogue)
    {
      if ((exact ? table.key != selector.name : table.name != selector.name) ||
          (qualifiers.global && *qualifiers.global != table.global) ||
          (qualifiers.type && *qualifiers.type != table.type) ||
          (qualifiers.entry && *qualifiers.entry != table.entry) || !identities.insert(table.key).second)
      {
        continue;
      }
      found = &table;
      if (identities.size() <= 8)
      {
        candidates += (candidates.empty() ? "" : "; ") + table.name + " (scope " +
          (table.global ? "global" : "frame") + ", type " + (table.type.empty() ? "<empty>" : table.type) +
          ", entry " + std::to_string(table.entry) + ")";
      }
    }
    if (identities.size() != 1)
    {
      if (identities.size() > 8)
      {
        candidates += "; and " + std::to_string(identities.size()-8) + " more";
      }
      error = identities.empty() ? "No table matches the name and qualifiers." :
        "Ambiguous table. Specify scope, type or entry. Candidates: " + candidates;
      return false;
    }
    output = *found;
    return true;
  }
  bool readable_result_table(const std::vector<ResultTableDescriptor>& catalogue,const std::string& key,
    ResultTableSelector& output,std::string& error)
  {
    ResultTableDescriptor selected;
    if (!resolve_result_table(catalogue,{key,{}},selected,error))
    {
      return false;
    }
    ResultTableSelector candidate{selected.name,{}};
    // Prefer name alone, scope, type, both; occurrence is used only as a last resort.
    for (int flags : {0,1,2,3,4,5,6,7})
    {
      candidate.qualifiers = {};
      if (flags & 1)
      {
        candidate.qualifiers.global = selected.global;
      }
      if (flags & 2)
      {
        candidate.qualifiers.type = selected.type;
      }
      if (flags & 4)
      {
        candidate.qualifiers.entry = selected.entry;
      }
      ResultTableDescriptor resolved;
      std::string ignored;
      if (resolve_result_table(catalogue,candidate,resolved,ignored) && resolved.key == selected.key)
      {
        output = candidate;
        error.clear();
        return true;
      }
    }
    error = "Cannot uniquely journal this table by name, scope, type and entry.";
    return false;
  }
}
