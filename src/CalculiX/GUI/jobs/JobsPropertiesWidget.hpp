#ifndef CALCULIX_JOBSPROPERTIESWIDGET_HPP
#define CALCULIX_JOBSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class JobsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit JobsPropertiesWidget(QWidget* parent = nullptr);
  bool setJob(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_JOBSPROPERTIESWIDGET_HPP
