#pragma once
#include <QWidget>
class QComboBox;
class QLineEdit;
class ForceTargetWidget : public QWidget
{
public:
  explicit ForceTargetWidget(QWidget* parent);
  QString idText() const;
  QString keyword() const;
  void setTarget(int referencePoint, const QString& nodeset);
private:
  void refreshPoints();
  QComboBox* kind;
  QComboBox* points;
  QLineEdit* nodes;
};
