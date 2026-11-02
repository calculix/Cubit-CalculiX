#ifndef CALCULIX_DAMPINGPROPERTIESWIDGET_HPP
#define CALCULIX_DAMPINGPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class DampingPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit DampingPropertiesWidget(QWidget* parent = nullptr);
  bool setDamping(int id);
};

#endif // CALCULIX_DAMPINGPROPERTIESWIDGET_HPP
