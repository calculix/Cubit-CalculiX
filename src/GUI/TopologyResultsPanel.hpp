#ifndef TOPOLOGYRESULTSPANEL_HPP
#define TOPOLOGYRESULTSPANEL_HPP

#include "CalculiXCoreInterface.hpp"
#include <QWidget>
#include <map>
#include <string>
#include <vector>

class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QShowEvent;
class QHideEvent;
class QSlider;
class QTimer;
class QSpinBox;

class TopologyResultsPanel : public QWidget
{
public:
  explicit TopologyResultsPanel(QWidget *parent = nullptr);
  void setIteration(int topology_id, int iteration_id = -1);

protected:
  void showEvent(QShowEvent *event) override;
  void hideEvent(QHideEvent *event) override;

private:
  enum Column
  {
    IterationId,
    JobId,
    JobStatus,
    Objective,
    VolumeFraction,
    Selected,
    DensityChange,
    ObjectiveChange,
    Converged,
    ColumnCount
  };

  CalculiXCoreInterface ccx_iface;

  QComboBox *cases;
  QComboBox *iterations;
  QDoubleSpinBox *threshold;
  QLabel *metrics;
  QPushButton *firstButton;
  QPushButton *previousButton;
  QPushButton *nextButton;
  QPushButton *lastButton;
  QPushButton *previewButton;
  QPushButton *deleteElementsButton;
  QPushButton *monitorButton;
  QPushButton *paraviewButton;
  QPushButton *gifButton;

  QLineEdit *gifPath;
  QSpinBox *fps;
  QCheckBox *overwrite;

  QDoubleSpinBox *wrapAlpha;
  QDoubleSpinBox *wrapOffset;
  QLineEdit *wrapPath;
  QCheckBox *wrapImport;
  QCheckBox *wrapOverwrite;
  QPushButton *wrapButton;

  QCheckBox *autoPreview;
  QSlider *iterationSlider;
  QPushButton *playButton;
  QTimer *playTimer;
  QTimer *previewTimer;
  bool playing = false;
  bool playback_change = false;
  bool preview_busy = false;

  int loaded_case = -1;
  std::map<int, std::vector<std::string>> rows;

  void refreshCases(int preferred_case = -1, int preferred_iteration = -1);
  void refreshIterations(int preferred_iteration = -1);
  void updateDisplay();
  void preview();
  void pausePlayback();
  void togglePlayback();
  void playbackFrame();
  void requestPreview();
  bool selectedResult(
    int& topology_id, int& iteration_id, int& job_id,
    bool require_idle = false);
  bool meshAvailable(int topology_id, int iteration_id);
  void project(bool preview_only);
  void openJob();
  void openParaView();
  void browseWrap();
  void wrapIteration();
  void browseGif();
  void exportGif();
};

#endif // TOPOLOGYRESULTSPANEL_HPP
