#pragma once
#include <QTreeWidget>
#include <functional>
#include <string>
class ReportTemplate;
class ReportConfiguration;

class ReportContentTree final : public QTreeWidget
{
public:
  explicit ReportContentTree(QWidget* parent, std::function<bool(const std::string&, std::string&)> submit);
  ~ReportContentTree() override;
  void refresh(const ReportTemplate& definition, const ReportConfiguration& configuration);
};
