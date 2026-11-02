#ifndef DRAWINGSELECTIONCONTROLLER_HPP
#define DRAWINGSELECTIONCONTROLLER_HPP

#include "Core/drawing/CoreDrawTypes.hpp"
#include "GUI/properties/PropertySelection.hpp"
#include <QObject>

class DrawingSelectionController : public QObject
{
public:
  explicit DrawingSelectionController(QObject* parent = nullptr);
  void showSelection(const PropertySelection& selection);
  void clearSelection();

private:
  void selectFeature(DrawFeature feature, int id, int stepId);
  void selectConstraint(int id);
};

#endif
