#pragma once
#include "Postprocessor/Core/filters/FilterDefinition.hpp"
#include <QWidget>
#include <functional>
#include <memory>
class PostprocessorApplicationInterface;
class QComboBox;
class QLineEdit;
class QCheckBox;
class QLabel;
class PostprocessorThresholdControls : public QWidget
{
public:
  PostprocessorThresholdControls(std::weak_ptr<PostprocessorApplicationInterface> application,
    std::function<void(const QString&)> submit,QWidget* parent);
  ~PostprocessorThresholdControls() override;
  void refresh();
private:
  void send(const QString& command);
  void submit_threshold(bool create);
  void refresh_components();
  void refresh_units();
  std::string source_id() const;
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<void(const QString&)> submit;
  QComboBox* filters;
  QComboBox* fields;
  QComboBox* components;
  QComboBox* datasets;
  QComboBox* nodeRule;
  QLineEdit* name;
  QLineEdit* lower;
  QLineEdit* upper;
  QCheckBox* lowerEnabled;
  QCheckBox* upperEnabled;
  QCheckBox* enabled;
  QCheckBox* visible;
  QCheckBox* reference;
  QLabel* sourceLabel;
  QLabel* unitLabel;
  std::vector<Postprocessor::ResultFieldGroup> catalogue;
  std::string lastActive;
  bool loading = false;
};
