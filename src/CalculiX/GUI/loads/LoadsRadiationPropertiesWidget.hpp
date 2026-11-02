#ifndef CALCULIX_LOADSRADIATIONPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSRADIATIONPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsRadiationPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsRadiationPropertiesWidget(QWidget* parent = nullptr);
  bool setRadiation(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSRADIATIONPROPERTIESWIDGET_HPP
