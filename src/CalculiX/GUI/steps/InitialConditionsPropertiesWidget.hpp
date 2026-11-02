#ifndef CALCULIX_INITIALCONDITIONSPROPERTIESWIDGET_HPP
#define CALCULIX_INITIALCONDITIONSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class InitialConditionsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit InitialConditionsPropertiesWidget(QWidget* parent = nullptr);
  bool setInitialCondition(int id);
};

#endif // CALCULIX_INITIALCONDITIONSPROPERTIESWIDGET_HPP
