#pragma once
#include "Framework/Reporting/ReportTemplate.hpp"
#include <QTreeWidget>
#include <functional>
#include <string>

class ReportContentTree final : public QTreeWidget
{
public:
  explicit ReportContentTree(QWidget* parent, std::function<bool(const std::string&, std::string&)> submit);
  ~ReportContentTree() override;
  void refresh(const ReportTemplate& definition, const ReportConfiguration& configuration);

private:
  void change(QTreeWidgetItem* item);
  void schedule_refresh();
  void collect(QTreeWidgetItem* item, bool checked, ReportConfiguration& patch) const;
  std::function<bool(const std::string&, std::string&)> submit;
  ReportTemplate definition;
  ReportConfiguration configuration;
  bool refreshPending = false;
};
