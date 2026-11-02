#ifndef CALCULIX_SURFACEINTERACTIONSPROPERTIESWIDGET_HPP
#define CALCULIX_SURFACEINTERACTIONSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class SurfaceInteractionsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit SurfaceInteractionsPropertiesWidget(QWidget* parent = nullptr);
  bool setSurfaceInteraction(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_SURFACEINTERACTIONSPROPERTIESWIDGET_HPP
