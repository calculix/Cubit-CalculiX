#pragma once
#include "Postprocessor/Core/representations/DatasetAppearance.hpp"
#include <QWidget>
#include <functional>
#include <memory>
#include <set>
class PostprocessorApplicationInterface;
class QComboBox;
class QCheckBox;
class QDoubleSpinBox;
class QPushButton;
class QLabel;
class PostprocessorAppearanceControls : public QWidget
{
public:
  using Submit = std::function<bool(const std::string&,std::string&)>;
  PostprocessorAppearanceControls(std::weak_ptr<PostprocessorApplicationInterface> application,Submit submit,QWidget* parent);
  void set_targets(const std::vector<Postprocessor::AppearanceTarget>& targets);
  void refresh();
private:
  void apply(bool reset);
  void update_enabled();
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Submit submit;
  std::vector<Postprocessor::AppearanceTarget> targets;
  std::vector<Postprocessor::DatasetAppearance> baseline;
  std::set<std::string> edited;
  Postprocessor::ColorChange colorChange = Postprocessor::ColorChange::Unchanged;
  std::array<double,3> chosenColor{};
  bool loading = false;
  QComboBox* style;
  QCheckBox* edges;
  QDoubleSpinBox* opacity;
  QDoubleSpinBox* pointSize;
  QPushButton* color;
  QPushButton* native;
  QPushButton* applyButton;
  QPushButton* resetButton;
  QLabel* selected;
  QLabel* errorLabel;
};
