#ifndef CALCULIX_NODESETPROPERTIESWIDGET_HPP
#define CALCULIX_NODESETPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class NodesetPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit NodesetPropertiesWidget(QWidget* parent = nullptr);
  bool setNodeset(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_NODESETPROPERTIESWIDGET_HPP
