#ifndef CALCULIX_CONTACTPAIRSPROPERTIESWIDGET_HPP
#define CALCULIX_CONTACTPAIRSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class ContactPairsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit ContactPairsPropertiesWidget(QWidget* parent = nullptr);
  bool setContactPair(int id);
};

#endif // CALCULIX_CONTACTPAIRSPROPERTIESWIDGET_HPP
