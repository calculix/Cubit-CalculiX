#pragma once
#include "GUI/properties/PropertyTreeWidget.hpp"

class DiscreteConnectionProperties : public PropertyTreeWidget
{
public:
  explicit DiscreteConnectionProperties(QWidget* parent);
  bool load(PropertyFeature feature, int id);
};
