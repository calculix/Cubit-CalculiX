#pragma once
#include <QDialog>
#include <functional>
#include <string>
#include <memory>
#include <map>
#include "Framework/Reporting/ReportRequest.hpp"
class ReportApplicationInterface;
class ReportConfiguration;
class QLineEdit;
class QSpinBox;
class QComboBox;
class QLabel;
class QCheckBox;

class ReportSettingsDialog final : public QDialog
{
public:
  ReportSettingsDialog(QWidget* parent, const ReportConfiguration& configuration, std::weak_ptr<ReportApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit);
  ~ReportSettingsDialog() override;

private:
  void apply();
  std::weak_ptr<ReportApplicationInterface> application;
  ReportRequest source;
  std::function<bool(const std::string&, std::string&)> submit;
  QLineEdit* title;
  QLineEdit* author;
  QLineEdit* project;
  QLineEdit* revision;
  QLineEdit* steps;
  QComboBox* paper;
  QSpinBox* precision;
  QLabel* message;
  std::map<std::string, QLineEdit*> decorationFields;
  std::map<std::string, QCheckBox*> decorationFlags;
  QComboBox* logoPlacement;
  QSpinBox* logoWidth;
  QLabel* logoThumbnail;
  std::optional<ReportImage> stagedLogo;
  QString logoFilename;
  bool logoChanged = false;
};
