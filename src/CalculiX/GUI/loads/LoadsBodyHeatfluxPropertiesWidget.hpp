#ifndef CALCULIX_LOADSBODYHEATFLUXPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSBODYHEATFLUXPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsBodyHeatfluxPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsBodyHeatfluxPropertiesWidget(QWidget* parent = nullptr);
  bool setBodyHeatflux(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSBODYHEATFLUXPROPERTIESWIDGET_HPP
