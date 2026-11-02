#ifndef CALCULIX_TOPOLOGYPROPERTIESWIDGET_HPP
#define CALCULIX_TOPOLOGYPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class TopologyPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit TopologyPropertiesWidget(QWidget* parent = nullptr);
  bool setTopology(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_TOPOLOGYPROPERTIESWIDGET_HPP
