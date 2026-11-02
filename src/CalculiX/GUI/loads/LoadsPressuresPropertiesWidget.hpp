#ifndef CALCULIX_LOADSPRESSURESPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSPRESSURESPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsPressuresPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsPressuresPropertiesWidget(QWidget* parent = nullptr);
  bool setPressure(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSPRESSURESPROPERTIESWIDGET_HPP
