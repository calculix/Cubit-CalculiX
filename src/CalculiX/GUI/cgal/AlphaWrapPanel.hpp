#ifndef ALPHAWRAPPANEL_HPP
#define ALPHAWRAPPANEL_HPP

#include <QWidget>

class QComboBox;
class QTreeWidget;
class QGroupBox;
class QDoubleSpinBox;
class QLineEdit;
class QCheckBox;
class QPushButton;
class QLabel;
class QShowEvent;

// Independent command panel; Topology Management has its own wrap controls.
class AlphaWrapPanel : public QWidget
{
public:
  void clearTopology(int topology_id = -1);
  explicit AlphaWrapPanel(QWidget* parent = nullptr);

protected:
  void showEvent(QShowEvent* event) override;

private:
  QComboBox* source = nullptr;
  QTreeWidget* blocks = nullptr;
  QGroupBox* blocksGroup = nullptr;
  QGroupBox* topologyGroup = nullptr;
  QComboBox* cases = nullptr;
  QComboBox* iterations = nullptr;
  QDoubleSpinBox* threshold = nullptr;
  QLineEdit* alpha = nullptr;
  QLineEdit* offset = nullptr;
  QLineEdit* filename = nullptr;
  QCheckBox* importStl = nullptr;
  QCheckBox* overwrite = nullptr;
  QPushButton* wrapButton = nullptr;
  QLabel* information = nullptr;
  bool executing = false;

  void refresh();
  void refreshIterations();
  void updateControls();
  void browse();
  void wrap();
};

#endif
