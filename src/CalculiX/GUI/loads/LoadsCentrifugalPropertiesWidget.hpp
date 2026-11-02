#ifndef CALCULIX_LOADSCENTRIFUGALPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSCENTRIFUGALPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsCentrifugalPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsCentrifugalPropertiesWidget(QWidget* parent = nullptr);
  bool setCentrifugal(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSCENTRIFUGALPROPERTIESWIDGET_HPP
