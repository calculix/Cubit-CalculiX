#ifndef CALCULIXPROPERTIESDOCK_HPP
#define CALCULIXPROPERTIESDOCK_HPP

#include "PropertySelection.hpp"
#include "PropertyViewState.hpp"
#include <QDockWidget>
#include <map>

class QLabel;
class QStackedWidget;
class PropertyTreeWidget;

class CalculiXPropertiesDock : public QDockWidget
{
public:
  explicit CalculiXPropertiesDock(QWidget* parent = nullptr);
  void showSelection(const PropertySelection& selection);
  void refreshSelection();
  void clearSelection();
  void reset();

private:
  void saveCurrentViewState();
  void showEmptyPage();
  PropertySelection currentSelection;
  std::map<PropertyFeature, PropertyViewState> viewStates;
  QStackedWidget* pages = nullptr;
  QWidget* emptyPage = nullptr;
  QWidget* heading = nullptr;
  QLabel* icon = nullptr;
  QLabel* caption = nullptr;
  std::map<PropertyFeature, PropertyTreeWidget*> widgets;
};

#endif // CALCULIXPROPERTIESDOCK_HPP
