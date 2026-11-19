#pragma once
#include <QAbstractScrollArea>
#include <QPointF>
#include <functional>
#include <memory>
#include <string>
class ReportDocumentLayout;

class ReportPreviewWidget final : public QAbstractScrollArea
{
public:
  explicit ReportPreviewWidget(QWidget* parent = nullptr);
  ~ReportPreviewWidget() override;
  void set_layout(std::shared_ptr<const ReportDocumentLayout> layout);
  void set_view(int page, int percent, const std::string& mode, bool navigate = false, bool refit = false);
  void set_position_changed(std::function<void(int, int)> callback);
  void set_page(int page);
  void set_zoom(double zoom);
  int page() const noexcept;
  int percent() const noexcept;
  void set_preview_command(std::function<void(const std::string&)> command);

protected:
  void paintEvent(QPaintEvent* event) override;
  void resizeEvent(QResizeEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;
  void scrollContentsBy(int dx, int dy) override;

private:
  void update_page_geometry(const QPointF& anchor);
  double scale() const;
  double stride() const;
  std::shared_ptr<const ReportDocumentLayout> layout;
  std::function<void(const std::string&)> previewCommand;
  int requestedPage = -1;
  int zoomPercent;
  std::string zoomMode;
  double previousScale = 0;
  bool changingGeometry = false;
  std::function<void(int, int)> positionChanged;
};
