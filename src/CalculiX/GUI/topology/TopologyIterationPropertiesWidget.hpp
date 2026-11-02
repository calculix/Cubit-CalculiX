#ifndef CALCULIX_TOPOLOGYITERATIONPROPERTIESWIDGET_HPP
#define CALCULIX_TOPOLOGYITERATIONPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class TopologyIterationPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit TopologyIterationPropertiesWidget(QWidget* parent = nullptr);
  bool setTopologyIteration(int topologyId, int iterationId);
};

#endif // CALCULIX_TOPOLOGYITERATIONPROPERTIESWIDGET_HPP
