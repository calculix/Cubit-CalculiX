#ifndef CALCULIX_LOADSFILMPROPERTIESWIDGET_HPP
#define CALCULIX_LOADSFILMPROPERTIESWIDGET_HPP

#include "GUI/properties/PropertyTreeWidget.hpp"

class LoadsFilmPropertiesWidget : public PropertyTreeWidget
{
public:
  explicit LoadsFilmPropertiesWidget(QWidget* parent = nullptr);
  bool setFilm(int id);
protected:
  void applyEdit(const PropertyEditRequest& request) override;

private:
  int currentEntityId = -1;
};

#endif // CALCULIX_LOADSFILMPROPERTIESWIDGET_HPP
