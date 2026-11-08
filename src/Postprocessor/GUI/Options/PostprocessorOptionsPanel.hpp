#ifndef POSTPROCESSOROPTIONSPANEL_HPP
#define POSTPROCESSOROPTIONSPANEL_HPP

#include <QJsonObject>
#include <QWidget>
#include <functional>
#include <memory>
#include <string>

class PostprocessorApplicationInterface;
class QLineEdit;
class QLabel;
class QCheckBox;
class QPushButton;

class PostprocessorOptionsPanel : public QWidget
{
public:
  PostprocessorOptionsPanel(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit, QWidget* parent);
  void refresh_settings();
  bool save_settings(std::string& error);

private:
  QJsonObject option_values() const;
  QJsonObject baseline;
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  QLineEdit* iconsPath = nullptr;
  QLabel* restartNotice = nullptr;
  QCheckBox* journalCamera = nullptr;
  QCheckBox* triad = nullptr;
  QPushButton* backgroundTop = nullptr;
  QPushButton* backgroundBottom = nullptr;
  void choose_color(QPushButton* button);
};

#endif
