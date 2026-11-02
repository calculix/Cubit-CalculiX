#ifndef CALCULIX_AMPLITUDESPROPERTIESWIDGET_HPP
#define CALCULIX_AMPLITUDESPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class AmplitudesPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit AmplitudesPropertiesWidget(QWidget* parent = nullptr);
  bool setAmplitude(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_AMPLITUDESPROPERTIESWIDGET_HPP
