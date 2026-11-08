#ifndef POSTPROCESSOR_MOUSEOPTIONSPANEL_HPP
#define POSTPROCESSOR_MOUSEOPTIONSPANEL_HPP
#include <QJsonObject>
#include <QWidget>
#include <functional>
#include <memory>
#include <string>

class PostprocessorApplicationInterface;
class QComboBox;
class QDoubleSpinBox;
class QCheckBox;

class PostprocessorMouseOptionsPanel : public QWidget
{
public:
  PostprocessorMouseOptionsPanel(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<bool(const std::string&, std::string&)> submit, QWidget* parent);
  void refresh_settings();
  bool save_settings(std::string& error);
private:
  QJsonObject option_values() const;
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<bool(const std::string&, std::string&)> submitCommand;
  QJsonObject baseline;
  QComboBox* left = nullptr;
  QComboBox* middle = nullptr;
  QComboBox* right = nullptr;
  QDoubleSpinBox* sensitivity = nullptr;
  QCheckBox* reverseZoom = nullptr;
};
#endif
