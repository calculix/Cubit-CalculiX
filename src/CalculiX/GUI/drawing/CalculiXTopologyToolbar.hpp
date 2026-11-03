#ifndef CALCULIXTOPOLOGYTOOLBAR_HPP
#define CALCULIXTOPOLOGYTOOLBAR_HPP
#include <QToolBar>
class QComboBox;
class QDoubleSpinBox;
class QAction;
class QTimer;
class CalculiXTopologyToolbar : public QToolBar
{
public:
  explicit CalculiXTopologyToolbar(QWidget* parent);
  void refresh();
  void cancelPendingPreview();
  void clear();
  void invalidateTopology(int id);
private:
  void refreshIterations(int selected = -1);
  void synchronize();
  void preview();
  void wrapIteration();
  void selectionChanged();
  QComboBox* cases;
  QComboBox* iterations;
  QDoubleSpinBox* threshold;
  QAction* first;
  QAction* previous;
  QAction* next;
  QAction* last;
  QAction* visibility;
  QAction* wrap = nullptr;
  QTimer* previewTimer;
  bool busy = false;
};
#endif
