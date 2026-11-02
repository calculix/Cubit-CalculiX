#ifndef CALCULIX_LOADSFORCESPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSFORCESPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsForcesPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsForcesPropertiesWidget(QWidget* parent = nullptr);
  bool setForce(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSFORCESPROPERTIESWIDGET_HPP
