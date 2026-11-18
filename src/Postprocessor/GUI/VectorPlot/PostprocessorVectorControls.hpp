#pragma once
#include <QWidget>
#include <functional>
#include <memory>
#include "Postprocessor/Core/representations/ResultFieldSelector.hpp"
class PostprocessorApplicationInterface;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class PostprocessorVectorControls : public QWidget
{
public:
  using Submit = std::function<bool(const std::string&,std::string&)>;
  PostprocessorVectorControls(std::weak_ptr<PostprocessorApplicationInterface> application,Submit submit,QWidget* parent);
  void refresh();
private:
  void send(const QString& operation);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Submit submit;
  bool refreshing = false;
  std::string source;
  std::vector<Postprocessor::ResultFieldSelector> selectors;
  QCheckBox* enabled = nullptr;
  QComboBox *fields = nullptr,*length = nullptr,*samples = nullptr,*color = nullptr;
  QDoubleSpinBox* scale = nullptr;
  QSpinBox *maximum = nullptr,*stride = nullptr;
};
