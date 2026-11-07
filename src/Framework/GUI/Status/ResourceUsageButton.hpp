#ifndef CUBITSIM_RESOURCEUSAGEBUTTON_HPP
#define CUBITSIM_RESOURCEUSAGEBUTTON_HPP

#include <QAbstractButton>
#include <QColor>
#include <vector>

struct ResourceBarSegment
{
  double fraction = 0;
  QColor color;
};

class ResourceUsageButton : public QAbstractButton
{
public:
  explicit ResourceUsageButton(QWidget* parent = nullptr);
  void set_usage(const QString& label, const std::vector<ResourceBarSegment>& values);
  QSize sizeHint() const override;
  QSize minimumSizeHint() const override;

protected:
  void paintEvent(QPaintEvent* event) override;

private:
  std::vector<ResourceBarSegment> segments;
};

#endif
