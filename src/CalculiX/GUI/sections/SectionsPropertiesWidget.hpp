#ifndef CALCULIX_SECTIONSPROPERTIESWIDGET_HPP
#define CALCULIX_SECTIONSPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class SectionsPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit SectionsPropertiesWidget(QWidget* parent = nullptr);
  bool setSection(int id);
};

#endif // CALCULIX_SECTIONSPROPERTIESWIDGET_HPP
