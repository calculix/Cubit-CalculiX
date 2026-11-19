#pragma once
#include <QWidget>
#include <map>
#include "Framework/Reporting/ReportConfiguration.hpp"
class QComboBox;
class QStackedWidget;
class QTableWidget;
class ReportMeshQualitySettings final : public QWidget
{
public:
  ReportMeshQualitySettings(const ReportConfiguration& configuration, QWidget* parent);
  bool configuration_patch(ReportConfiguration& output, std::string& error) const;
private:
  void restore_family_defaults();
  QComboBox* familySelector;
  QStackedWidget* pages;
  std::map<std::string, QTableWidget*> familyTables;
};
