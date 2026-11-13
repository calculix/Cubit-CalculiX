#ifndef CALCULIXDRAWINGTOOLBAR_HPP
#define CALCULIXDRAWINGTOOLBAR_HPP

#include "Core/drawing/CoreDrawTypes.hpp"
#include <QToolBar>
#include <map>
class QComboBox;
class QDoubleSpinBox;
class QAction;

class CalculiXDrawingToolbar : public QToolBar
{
public:
  explicit CalculiXDrawingToolbar(QWidget* parent = nullptr);
  void refreshSteps();
  void synchronize(const CalculiXDrawState& state);
private:
  bool allGroupsEnabled(const CalculiXDrawState& state) const;
  QAction* drawAll = nullptr;
  QAction* overlayVisibility = nullptr;
  QAction* symbolPicking = nullptr;
  QAction* labelVisibility = nullptr;
  QComboBox* scope = nullptr;
  QDoubleSpinBox* size = nullptr;
  std::map<DrawGroup, QAction*> groups;
  void submit(const QString& command);
};
#endif
