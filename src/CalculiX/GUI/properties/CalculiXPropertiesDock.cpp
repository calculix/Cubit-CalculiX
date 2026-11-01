#include "CalculiXPropertiesDock.hpp"

CalculiXPropertiesDock::CalculiXPropertiesDock(QWidget* parent)
  : QDockWidget(tr("CalculiX Properties"), parent)
{
  setObjectName("CalculiX.Properties");
  setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
  setWidget(new QWidget(this));
  hide();
}
