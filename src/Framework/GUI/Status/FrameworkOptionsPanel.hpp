#ifndef CUBITSIM_FRAMEWORKOPTIONSPANEL_HPP
#define CUBITSIM_FRAMEWORKOPTIONSPANEL_HPP

#include <QJsonObject>
#include <QWidget>
#include <string>

class QCheckBox;
class QSpinBox;
class QPushButton;

class FrameworkOptionsPanel : public QWidget
{
public:
  explicit FrameworkOptionsPanel(QWidget* parent);
  void refresh_settings();
  bool save_settings(std::string& error);

private:
  QJsonObject option_values() const;
  QJsonObject baseline;
  QCheckBox* enabled = nullptr;
  QSpinBox* interval = nullptr;
  QCheckBox* cpu = nullptr;
  QCheckBox* memory = nullptr;
  QCheckBox* cubit = nullptr;
  QCheckBox* jobs = nullptr;
  QCheckBox* jobAttribution = nullptr;
  QPushButton* systemColor = nullptr;
  QPushButton* cubitColor = nullptr;
  QPushButton* jobsColor = nullptr;
};

#endif
