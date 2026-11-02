#ifndef CALCULIX_MATERIALPROPERTIESWIDGET_HPP
#define CALCULIX_MATERIALPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class MaterialPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit MaterialPropertiesWidget(QWidget* parent = nullptr);
  bool setMaterial(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_MATERIALPROPERTIESWIDGET_HPP
