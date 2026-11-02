#ifndef CALCULIX_PHYSICALCONSTANTSPROPERTIESWIDGET_HPP
#define CALCULIX_PHYSICALCONSTANTSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class PhysicalConstantsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit PhysicalConstantsPropertiesWidget(QWidget* parent = nullptr);
  bool setPhysicalConstants(int id);
};

#endif // CALCULIX_PHYSICALCONSTANTSPROPERTIESWIDGET_HPP
