#ifndef CALCULIX_LOADSTRAJECTORYPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSTRAJECTORYPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsTrajectoryPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsTrajectoryPropertiesWidget(QWidget* parent = nullptr);
  bool setTrajectory(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSTRAJECTORYPROPERTIESWIDGET_HPP
