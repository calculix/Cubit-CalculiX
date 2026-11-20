#pragma once
#include <QWidget>
class QComboBox;
class QLineEdit;
class QLabel;
class QSpinBox;
class QPushButton;
class ConnectionEndpointWidget;

class DiscreteConnectionPanel : public QWidget
{
public:
  enum class Feature { PointMass, Spring };
  explicit DiscreteConnectionPanel(Feature feature, QWidget* parent = nullptr);
  Feature type() const;
  void newRecord();
  void editRecord(int id);
  void setReferencePoint(int id);
private:
  void submit();
  Feature feature;
  bool editing = false;
  QSpinBox* id;
  QLineEdit* name;
  QLineEdit* value;
  ConnectionEndpointWidget* first;
  ConnectionEndpointWidget* second = nullptr;
  QComboBox* springType = nullptr;
  QComboBox* direction = nullptr;
  QComboBox* history;
  QSpinBox* step;
  QPushButton* apply;
};
