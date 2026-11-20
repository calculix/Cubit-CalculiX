#pragma once
#include "Core/connections/ConnectionEndpoint.hpp"
#include <QWidget>

class QComboBox;
class QSpinBox;
class QPushButton;

class ConnectionEndpointWidget : public QWidget
{
public:
  explicit ConnectionEndpointWidget(QWidget* parent);
  QString command() const;
  void setEndpoint(const ConnectionEndpoint& endpoint);
  void refresh();
private:
  QComboBox* kind;
  QComboBox* points;
  QSpinBox* vertex;
  QPushButton* pick;
};
