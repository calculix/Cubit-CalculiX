#ifndef CALCULIX_HBCSPROPERTIESWIDGET_HPP
#define CALCULIX_HBCSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class HBCsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit HBCsPropertiesWidget(QWidget* parent = nullptr);
  bool setHBCs(int id);
};

#endif // CALCULIX_HBCSPROPERTIESWIDGET_HPP
