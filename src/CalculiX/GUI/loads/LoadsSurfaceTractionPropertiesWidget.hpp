#ifndef CALCULIX_LOADSSURFACETRACTIONPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSSURFACETRACTIONPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsSurfaceTractionPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsSurfaceTractionPropertiesWidget(QWidget* parent = nullptr);
  bool setSurfaceTraction(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSSURFACETRACTIONPROPERTIESWIDGET_HPP
