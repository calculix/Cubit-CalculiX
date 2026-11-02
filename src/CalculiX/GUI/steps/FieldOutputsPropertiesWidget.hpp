#ifndef CALCULIX_FIELDOUTPUTSPROPERTIESWIDGET_HPP
#define CALCULIX_FIELDOUTPUTSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class FieldOutputsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit FieldOutputsPropertiesWidget(QWidget* parent = nullptr);
  bool setFieldOutput(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_FIELDOUTPUTSPROPERTIESWIDGET_HPP
