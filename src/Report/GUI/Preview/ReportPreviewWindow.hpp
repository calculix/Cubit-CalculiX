#pragma once
#include <QDialog>
#include <functional>
#include <memory>
#include <string>
class ReportPreviewWidget;
class ReportDocumentLayout;
class QListWidget;
class QSpinBox;
class QSlider;

class ReportPreviewWindow final : public QDialog
{
public:
  ReportPreviewWindow(QWidget* parent, std::function<bool(const std::string&, std::string&)> submit,
    std::function<bool(const std::string&, std::string&)> silent);
  ~ReportPreviewWindow() override;
  void set_layout(std::shared_ptr<const ReportDocumentLayout> layout);
  void set_view(int page, int zoom, const std::string& mode, bool navigate = false, bool refit = false);

private:
  void export_file(const std::string& format);
  std::function<bool(const std::string&, std::string&)> submit;
  std::function<bool(const std::string&, std::string&)> silent;
  ReportPreviewWidget* page;
  QListWidget* contents;
  QSpinBox* number;
  QSlider* pageSlider;
  QSpinBox* zoom;
  std::shared_ptr<const ReportDocumentLayout> currentLayout;
};
