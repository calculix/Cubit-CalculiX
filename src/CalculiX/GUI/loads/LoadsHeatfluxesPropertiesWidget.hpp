#ifndef CALCULIX_LOADSHEATFLUXESPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSHEATFLUXESPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsHeatfluxesPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsHeatfluxesPropertiesWidget(QWidget* parent = nullptr);
  bool setHeatflux(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSHEATFLUXESPROPERTIESWIDGET_HPP
