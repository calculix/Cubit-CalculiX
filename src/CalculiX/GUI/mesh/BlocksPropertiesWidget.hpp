#ifndef CALCULIX_BLOCKSPROPERTIESWIDGET_HPP
#define CALCULIX_BLOCKSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class BlocksPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit BlocksPropertiesWidget(QWidget* parent = nullptr);
  bool setBlock(int id);

protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  void reload();
  int currentBlockId = -1;
};

#endif // CALCULIX_BLOCKSPROPERTIESWIDGET_HPP
