#ifndef CALCULIX_CONSTRAINTSPROPERTIESWIDGET_HPP
#define CALCULIX_CONSTRAINTSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class ConstraintsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit ConstraintsPropertiesWidget(QWidget* parent = nullptr);
  bool setConstraint(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_CONSTRAINTSPROPERTIESWIDGET_HPP
