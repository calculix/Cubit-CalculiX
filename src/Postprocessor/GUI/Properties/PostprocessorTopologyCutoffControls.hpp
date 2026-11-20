#pragma once
#include <QWidget>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>

class QDoubleSpinBox;
class QLabel;
class QPushButton;
namespace Postprocessor { struct DataSource; }

class PostprocessorTopologyCutoffControls : public QWidget
{
public:
  using Submit = std::function<bool(const std::string&,std::string&)>;
  explicit PostprocessorTopologyCutoffControls(Submit submit, QWidget* parent);
  void show_source(const Postprocessor::DataSource* source, std::size_t frames);

private:
  void update_apply();
  Submit submit;
  QLabel* iterations;
  QDoubleSpinBox* cutoff;
  QPushButton* apply;
  std::string sourceId;
  std::string sourceName;
  std::optional<double> committed;
  double displayedCutoff = 0;
};
