#ifndef CALCULIX_STEPSPROPERTIESWIDGET_HPP
#define CALCULIX_STEPSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class StepsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit StepsPropertiesWidget(QWidget* parent = nullptr);
  bool setStep(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_STEPSPROPERTIESWIDGET_HPP
