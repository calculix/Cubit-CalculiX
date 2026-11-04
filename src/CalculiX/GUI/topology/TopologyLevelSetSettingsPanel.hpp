#ifndef TOPOLOGYLEVELSETSETTINGSPANEL_HPP
#define TOPOLOGYLEVELSETSETTINGSPANEL_HPP
#include <QWidget>
#include <QStringList>
#include <array>
#include <map>
enum class TopologyMethod;
struct TopologyOptions;
class QLabel;

class QComboBox;
class QLineEdit;
class QPushButton;
class QShowEvent;
class QSpinBox;

class TopologyLevelSetSettingsPanel : public QWidget
{
public:
  void clearTopology(int topology_id = -1);
  explicit TopologyLevelSetSettingsPanel(bool modify, QWidget *parent = nullptr);
  bool isModify() const;
  void setTopology(int topology_id);

protected:
  void showEvent(QShowEvent *event) override;

private:
  bool modifying;
  QSpinBox *id;
  QComboBox *cases;
  QWidget *settings;
  // Complete method-specific fields; order matches values() and command keywords.
  std::array<QLineEdit *, 14> fields;
  TopologyMethod storedMethod;
  std::map<int, QStringList> drafts;
  void rememberDraft();
  QComboBox *objective;
  QComboBox *displacementNodeset;
  QComboBox *displacementComponent;
  QPushButton *applyButton;
  QLabel* feedback = nullptr;
  QStringList original;
  int loaded_id = -1;

  void refreshCases(int preferred_id = -1);
  void refreshNodesets(int preferred_id = -1);
  void load();
  void apply();
  QStringList values() const;
  QStringList values(const TopologyOptions& options) const;
};

#endif // TOPOLOGYLEVELSETSETTINGSPANEL_HPP
