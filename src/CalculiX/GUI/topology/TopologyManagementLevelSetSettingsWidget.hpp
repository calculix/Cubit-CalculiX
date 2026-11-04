#ifndef TOPOLOGYMANAGEMENTLEVELSETSETTINGSWIDGET_HPP
#define TOPOLOGYMANAGEMENTLEVELSETSETTINGSWIDGET_HPP
#include <QWidget>
#include <QStringList>
#include <array>
#include <map>
#include <functional>
enum class TopologyMethod;
struct TopologyOptions;
class QLabel;

class QComboBox;
class QLineEdit;
class QPushButton;
class QShowEvent;
class QSpinBox;

class TopologyManagementLevelSetSettingsWidget : public QWidget
{
public:
  void clearTopology(int topology_id = -1);
  explicit TopologyManagementLevelSetSettingsWidget(std::function<bool(const QString&)> submit, std::function<void(int)> applied, QWidget* parent = nullptr);
  void setContext(int topology_id, bool create, int new_id = 1);
  void rememberDraft();
  void setEditable(bool enabled);
  int topologyId() const;

private:
  bool modifying = false;
  QSpinBox *id;
  int currentCase = -1;
  std::function<bool(const QString&)> submit;
  std::function<void(int)> applied;
  QWidget *settings;
  // Complete method-specific fields; order matches values() and command keywords.
  std::array<QLineEdit *, 14> fields;
  TopologyMethod storedMethod;
  std::map<int, QStringList> drafts;
  QComboBox *objective;
  QComboBox *displacementNodeset;
  QComboBox *displacementComponent;
  QPushButton *applyButton;
  QLabel* feedback = nullptr;
  QStringList original;
  int loaded_id = -1;

  void refreshNodesets(int preferred_id = -1);
  void load();
  void apply();
  QStringList values() const;
  QStringList values(const TopologyOptions& options) const;
};

#endif // TOPOLOGYMANAGEMENTLEVELSETSETTINGSWIDGET_HPP
