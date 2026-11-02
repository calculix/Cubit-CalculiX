#ifndef CALCULIX_SIDESETPROPERTIESWIDGET_HPP
#define CALCULIX_SIDESETPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class SidesetPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit SidesetPropertiesWidget(QWidget* parent = nullptr);
  bool setSideset(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_SIDESETPROPERTIESWIDGET_HPP
