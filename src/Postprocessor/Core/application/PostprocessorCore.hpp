#ifndef POSTPROCESSOR_CORE_HPP
#define POSTPROCESSOR_CORE_HPP

#include "../session/PostprocessorSession.hpp"
#include "../sources/SourceRuntime.hpp"
#include "Framework/Results/ResultSourceReference.hpp"

#include <cstdint>
#include <map>
#include <optional>
#include <string>

class ResultSourceProviderInterface;

namespace Postprocessor
{
  /** Host-thread owner of Postprocessor definitions and runtime result handles.
   * Callers resolve provider prefixes through Framework, then pass a borrowed
   * provider for one operation. No provider or CalculiX reader pointer is retained.
   * All mutations reject reentry during a provider operation. Query references
   * and iterators are valid only until the next successful mutation; returned
   * owning result handles can outlive refresh, closure and this Core instance.
   * This class does not create displays, activate workspaces or submit commands.
   */
  class PostprocessorCore
  {
  public:
    PostprocessorCore() = default;

    PostprocessorCore(const PostprocessorCore&) = delete;
    PostprocessorCore& operator=(const PostprocessorCore&) = delete;

    const PostprocessorSession& session() const noexcept;

    // Exact, case-sensitive lookup; an empty result means no match.
    std::string source_id_from_name(const std::string& name) const;
    std::shared_ptr<const SimResults::VtkResultsData> result_data(const std::string& source_id) const;

    // nullopt uses the provider's suggested name. An explicit empty/blank name
    // is invalid. Names are preserved verbatim and must be unique in the session.
    // Failure preserves session, runtime handles and the source ID allocator.
    // Provider-side reader/cache changes are outside this transaction boundary.
    bool open_result(
      ResultSourceProviderInterface& provider,
      const SimResults::ResultSourceReference& request,
      const std::optional<std::string>& name,
      std::string& error);

    // Preserves source ID/name and active context; replaces resolved provenance
    // and runtime data and increments the revision only after full validation.
    bool refresh_result(
      ResultSourceProviderInterface& provider,
      const std::string& source_id,
      std::string& error);

    // Selects a registered source, clearing stale active representation context.
    bool select_source(const std::string& source_id, std::string& error);

    // Removes the source and its direct representations. Future feature
    // dependencies require explicit closure rules when features are implemented.
    // Closing the active source clears active source/representation selection.
    bool close_source(const std::string& source_id, std::string& error);

    // Removes sources, runtime data and representations, retaining views/layout.
    bool close_all_sources(std::string& error);

  private:
    PostprocessorSession state;
    std::map<std::string, SourceRuntime> runtime;
    std::uint64_t next_source_number = 1;
    bool source_operation_active = false;
  };
}

#endif // POSTPROCESSOR_CORE_HPP
