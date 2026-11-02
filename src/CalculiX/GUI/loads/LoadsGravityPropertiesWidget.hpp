#ifndef CALCULIX_LOADSGRAVITYPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSGRAVITYPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsGravityPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsGravityPropertiesWidget(QWidget* parent = nullptr);
  bool setGravity(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSGRAVITYPROPERTIESWIDGET_HPP
