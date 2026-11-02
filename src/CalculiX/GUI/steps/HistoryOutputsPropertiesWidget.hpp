#ifndef CALCULIX_HISTORYOUTPUTSPROPERTIESWIDGET_HPP
#define CALCULIX_HISTORYOUTPUTSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class HistoryOutputsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit HistoryOutputsPropertiesWidget(QWidget* parent = nullptr);
  bool setHistoryOutput(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_HISTORYOUTPUTSPROPERTIESWIDGET_HPP
