#pragma once
#include "ResultTables.hpp"
namespace Postprocessor
{
  struct TableQualifiers
  {
    std::optional<bool> global;
    std::optional<std::string> type;
    std::optional<std::size_t> entry;
  };
  struct ResultTableSelector
  {
    std::string name;
    TableQualifiers qualifiers;
  };
  // Entry is the 1-based occurrence within the name/type/scope family in provider
  // order, not a frame or a global catalogue ordinal. Resolve across all frames.
  // Existing encoded keys remain accepted, with qualifiers still enforced.
  bool resolve_result_table(const std::vector<ResultTableDescriptor>& catalogue,const ResultTableSelector& selector,
    ResultTableDescriptor& output,std::string& error);
  // Verify the shortest readable spelling against the same resolver. No key fallback.
  bool readable_result_table(const std::vector<ResultTableDescriptor>& catalogue,const std::string& key,
    ResultTableSelector& output,std::string& error);
}
