#ifndef CALCULIX_BCSDISPLACEMENTSPROPERTIESWIDGET_HPP
#define CALCULIX_BCSDISPLACEMENTSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class BCsDisplacementsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit BCsDisplacementsPropertiesWidget(QWidget* parent = nullptr);
  bool setDisplacement(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_BCSDISPLACEMENTSPROPERTIESWIDGET_HPP
