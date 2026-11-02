#ifndef CALCULIX_BCSTEMPERATURESPROPERTIESWIDGET_HPP
#define CALCULIX_BCSTEMPERATURESPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class BCsTemperaturesPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit BCsTemperaturesPropertiesWidget(QWidget* parent = nullptr);
  bool setTemperature(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_BCSTEMPERATURESPROPERTIESWIDGET_HPP
