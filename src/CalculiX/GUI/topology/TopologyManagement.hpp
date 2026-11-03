#ifndef TOPOLOGYMANAGEMENT_HPP
#define TOPOLOGYMANAGEMENT_HPP
#include <QWidget>
#include <QPointer>
#include <QStringList>
#include <array>
#include <map>
#include <string>
#include <vector>

struct TopologyOptions;
class JobsMonitor;
class QTreeWidget;
class QTabWidget;
class QProgressBar;
class QVBoxLayout;
class QCloseEvent;
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

class TopologyManagement : public QWidget
{
public:
  explicit TopologyManagement(JobsMonitor* monitor, QWidget* parent = nullptr);
  void clear();
  void invalidateTopology(int topology_id);
  void refresh();
  void setTopology(int topology_id);
  void setIteration(int topology_id, int iteration_id = -1);

protected:
  void closeEvent(QCloseEvent* event) override;
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
    Displacement,
    DisplacementNode,
    DisplacementNodeset,
    DisplacementComponent,
    ColumnCount
  };


  enum Tab { SettingsTab, PreservationTab, RunTab, IterationsTab, AnimationTab, WrapTab };
  int currentTopologyId = -1;
  bool creating = false;
  bool refreshing = false;
  bool submitting = false;
  bool loadingSettings = false;
  bool settingsDirty = false;
  QPointer<JobsMonitor> jobsMonitor;
  QTreeWidget* caseTree = nullptr;
  QTreeWidget* iterationTree = nullptr;
  QTabWidget* tabs = nullptr;
  QTimer* refreshTimer = nullptr;
  QLabel* heading = nullptr;
  QPushButton* newButton = nullptr;
  QPushButton* deleteButton = nullptr;
  QPushButton* adjointMonitorButton = nullptr;
  std::vector<std::vector<std::string>> iterationRecords;
  std::string lastRunState;

  QWidget* settingsForm = nullptr;
  QSpinBox* settingsId = nullptr;
  std::array<QLineEdit*, 9> settingsFields{};
  QComboBox* settingsMethod = nullptr;
  QComboBox* settingsObjective = nullptr;
  QComboBox* settingsNodeset = nullptr;
  QComboBox* settingsComponent = nullptr;
  QComboBox* settingsAdaptive = nullptr;
  QLineEdit* settingsMinimumMove = nullptr;
  QPushButton* settingsApply = nullptr;
  QLabel* settingsMessage = nullptr;
  QStringList settingsOriginal;
  std::map<int, QStringList> settingsDrafts;

  QComboBox* setType = nullptr;
  QTreeWidget* available = nullptr;
  QTreeWidget* preserved = nullptr;
  QPushButton* addButton = nullptr;
  QPushButton* removeButton = nullptr;
  QPushButton* resetButton = nullptr;
  QLabel* information = nullptr;
  int loaded_id = -1;
  int loaded_type = 0;
  bool editable = false;

  QCheckBox* wait = nullptr;
  QLabel* status = nullptr;
  QPushButton* runButton = nullptr;
  QPushButton* waitButton = nullptr;
  QPushButton* killButton = nullptr;
  QPushButton* runMonitorButton = nullptr;
  struct ConvergenceCriterion { QProgressBar* bar = nullptr; QLabel* badge = nullptr; };
  std::array<ConvergenceCriterion, 3> criteria;
  QLabel* convergenceIteration = nullptr;
  QLabel* convergenceSummary = nullptr;
  QLabel* displacementMeasurement = nullptr;

  void createSettingsTab(QWidget* tab);
  void createPreservationTab(QWidget* tab);
  void createRunTab(QWidget* tab);
  void createIterationsTab(QWidget* tab);
  void createAnimationTab(QWidget* tab);
  void createWrapTab(QWidget* tab);
  void connectIterationControls();
  void selectTopology(int topology_id);
  bool submitCommand(const QString& command);
  void synchronizeResultControls();
  void setResultThreshold(double value);
  bool hasAdjointJob(int iteration_id);
  void openAdjointJob();
  void beginCreate();
  void deleteTopology();
  void loadSettings();
  void applySettings();
  void rememberSettingsDraft();
  QStringList settingsValues() const;
  QStringList settingsValues(const TopologyOptions& options) const;
  void setSettingsValues(const QStringList& values);
  void refreshSettingsNodesets(int selected);
  void refreshLists();
  void updateButtons();
  void changeSelection(bool adding);
  void resetDefaults();
  void updateControls();
  void execute(const QString& action);
  void openActiveJob();
  void createConvergenceDisplay(QVBoxLayout* layout);
  void updateConvergence();
  int updateCriterion(int index, const QString& value, double limit);
  QComboBox *iterations = nullptr;
  QDoubleSpinBox *threshold = nullptr;
  QLabel *metrics = nullptr;
  QPushButton *firstButton = nullptr;
  QPushButton *previousButton = nullptr;
  QPushButton *nextButton = nullptr;
  QPushButton *lastButton = nullptr;
  QPushButton *previewButton = nullptr;
  QPushButton *deleteElementsButton = nullptr;
  QPushButton *monitorButton = nullptr;
  QPushButton *gifButton = nullptr;
  QPushButton *cancelGifButton = nullptr;
  bool animation_was_active = false;

  QDoubleSpinBox* gifThreshold = nullptr;
  QDoubleSpinBox* wrapThreshold = nullptr;
  QComboBox* wrapIterations = nullptr;
  QLineEdit *gifPath = nullptr;
  QSpinBox *fps = nullptr;
  QCheckBox *overwrite = nullptr;

  QLineEdit *wrapAlpha = nullptr;
  QLineEdit *wrapOffset = nullptr;
  QLineEdit *wrapPath = nullptr;
  QCheckBox *wrapImport = nullptr;
  QCheckBox *wrapOverwrite = nullptr;
  QPushButton *wrapButton = nullptr;

  QCheckBox *autoPreview = nullptr;
  QSlider *iterationSlider = nullptr;
  QPushButton *playButton = nullptr;
  QTimer *playTimer = nullptr;
  QTimer *previewTimer = nullptr;
  bool playing = false;
  bool playback_change = false;
  bool preview_busy = false;

  int loaded_case = -1;
  std::map<int, std::vector<std::string>> rows;

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
  void browseWrap();
  void wrapIteration();
  void browseGif();
  void exportGif();
  void updateAnimationControls();
};

#endif // TOPOLOGYMANAGEMENT_HPP
