#pragma once
#include <QWidget>
class QComboBox;
class DistributingAttachmentPanel : public QWidget
{
public:
  explicit DistributingAttachmentPanel(QWidget* parent = nullptr);
  void selectPoint(int point);
private:
  QComboBox* points;
};
