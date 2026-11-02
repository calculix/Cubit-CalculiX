#ifndef CALCULIX_CUSTOMLINESPROPERTIESWIDGET_HPP
#define CALCULIX_CUSTOMLINESPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class CustomLinesPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit CustomLinesPropertiesWidget(QWidget* parent = nullptr);
  bool setCustomLine(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_CUSTOMLINESPROPERTIESWIDGET_HPP
