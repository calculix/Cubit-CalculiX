#pragma once
#include "Postprocessor/Core/filters/FilterDefinition.hpp"
#include <QWidget>
#include <memory>
#include <functional>
class PostprocessorApplicationInterface;
class QComboBox;
class QLineEdit;
class QLabel;
class QCheckBox;
class PostprocessorClipControls : public QWidget
{
public:
  PostprocessorClipControls(std::weak_ptr<PostprocessorApplicationInterface> application,std::function<void(const QString&)> submit,QWidget* parent);
  void refresh();
private:
  void submit_plane(bool create);
  void send(const QString& command);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<void(const QString&)> submit;
  QComboBox* filters;
  QComboBox* side;
  QLineEdit* name;
  QLineEdit* coordinates[6];
  std::array<QLabel*,3> originLabels;
  QCheckBox* visible;
  std::string lastActive;
  bool loading = false;
};
