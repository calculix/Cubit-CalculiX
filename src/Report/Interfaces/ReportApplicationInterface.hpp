#pragma once
#include "Report/Core/ReportCore.hpp"
#include <memory>
#include <cstdint>

class ReportDocumentLayout;

// Commands mutate through this boundary; widgets read committed snapshots.
class ReportApplicationInterface
{
public:
  explicit ReportApplicationInterface(ReportProviderRegistryInterface& registry);
  ~ReportApplicationInterface();
  const ReportSession& session() const noexcept;
  std::shared_ptr<const ReportDocument> document() const noexcept;
  bool describe(ReportTemplate& output, std::string& error) const;
  const ReportRequest& configuration_request() const noexcept;
  bool available_sources(std::vector<ReportSourceEntry>& output, std::string& error) const;
  bool load_logo(const std::string& filename, ReportImage& output, std::string& error) const;
  bool choose_source(const ReportRequest& source, std::string& error);
  bool generate_request(const ReportRequest& source, const std::string& profile, std::string& error);
  bool validate_configuration_patch(const ReportConfiguration& patch, std::string& error) const;
  bool configure_patch(const ReportConfiguration& patch, std::string& error);
  bool generate(const std::string& provider, int job, const std::string& profile, std::string& error);
  bool update(std::string& error);
  bool configure(const std::string& key, const std::string& value, std::string& error);
  bool note(const std::string& section, const std::string& value, std::string& error);
  bool profile(const std::string& action, const std::string& name, std::vector<ReportProfile>& output, std::string& error);
  bool request_preview(std::string& error, int page = -1, int zoom = -1, const std::string& fit = "");
  const std::string& preview_mode() const noexcept;
  std::uint64_t preview_window_revision() const noexcept;
  std::uint64_t preview_page_revision() const noexcept;
  std::uint64_t preview_fit_revision() const noexcept;
  int preview_page() const noexcept;
  int preview_zoom() const noexcept;
  std::uint64_t preview_revision() const noexcept;
  bool layout(std::shared_ptr<const ReportDocumentLayout>& output, std::string& error);
  bool export_document(const std::string& format, const std::string& filename, std::string& error, bool overwrite = false);
  bool prepare_project_read(const std::string& filename, std::string& error);
  bool commit_project_read(std::string& error);
  void discard_project_read() noexcept;
  bool read_project(const std::string& filename, std::string& error);
  bool write_project(const std::string& filename, std::string& error) const;
  void reset();
  void set_gui_available(bool available) noexcept;

private:
  ReportCore core;
  std::unique_ptr<ReportSession> preparedProject;
  std::shared_ptr<const ReportDocument> layout_source;
  std::shared_ptr<ReportDocumentLayout> cached_layout;
  std::uint64_t previewRevision = 0;
  int previewPage;
  int previewZoom;
  std::string previewMode;
  std::uint64_t previewWindowRevision = 0;
  std::uint64_t previewPageRevision = 0;
  std::uint64_t previewFitRevision = 0;
  bool guiAvailable = false;
};
