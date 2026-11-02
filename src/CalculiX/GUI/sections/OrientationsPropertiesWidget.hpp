#ifndef CALCULIX_ORIENTATIONSPROPERTIESWIDGET_HPP
#define CALCULIX_ORIENTATIONSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class OrientationsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit OrientationsPropertiesWidget(QWidget* parent = nullptr);
  bool setOrientation(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_ORIENTATIONSPROPERTIESWIDGET_HPP
