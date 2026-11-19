#include "Report/Core/ReportCore.hpp"
#include <functional>
#include <utility>

ReportCore::ReportCore(ReportProviderRegistryInterface& providers) : registry(providers) {
  state.request.provider = "ccx";
}

ReportCore::~ReportCore() = default;

const ReportSession& ReportCore::session() const noexcept
{
  return state;
}

std::shared_ptr<const ReportDocument> ReportCore::document() const noexcept
{
  const auto found = state.documents.find(state.active_id);
  if (found == state.documents.end()) {
    return nullptr;
  }
  return found->second;
}

bool ReportCore::note(const std::string& section, const std::string& text, std::string& error) {
  const auto current = document();
  if (!current || text.size() > 65536) {
    error = "Select a report and provide a note no longer than 64 KiB.";
    return false;
  }
  std::function<bool(const std::vector<ReportSection>&)> contains = [&](const std::vector<ReportSection>& sections) {
    for (const auto& item : sections) {
      if (item.id == section || contains(item.children)) {
        return true;
      }
    }
    return false;
  };
  if (!contains(current->sections)) {
    error = "The note must target a generated section ID.";
    return false;
  }
  auto candidate = std::make_shared<ReportDocument>(*current);
  candidate->notes[section] = text;
  state.documents[state.active_id] = std::move(candidate);
  error.clear();
  return true;
}

void ReportCore::restore(ReportSession session) {
  state = std::move(session);
}

void ReportCore::reset() {
  state = ReportSession();
  state.request.provider = "ccx";
}
