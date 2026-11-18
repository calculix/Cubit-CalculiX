#pragma once
#include "Postprocessor/Core/representations/VectorPlotDefinition.hpp"
#include <QWidget>
#include <functional>
#include <memory>
class PostprocessorApplicationInterface;
class QLabel;
class QCheckBox;
class QComboBox;
class QSpinBox;
class QLineEdit;
class QPushButton;
class QGridLayout;
class PostprocessorVectorLegendProperties final : public QWidget
{
public:
  using Submit = std::function<bool(const std::string&,std::string&)>;
  PostprocessorVectorLegendProperties(std::weak_ptr<PostprocessorApplicationInterface> app,Submit submit,QWidget* parent);
  void refresh();
private:
  void send(const QString& operation);
  void range_changed();
  void limits_changed();
  void rebuild_colors(const Postprocessor::ResultColoring& coloring);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Submit submit;
  std::string source,viewId;
  bool loading = false;
  std::optional<Postprocessor::VectorPlotDefinition> baseline;
  QLabel *information,*errorLabel;
  QComboBox *palette,*range,*contours;
  QSpinBox* bands;
  QCheckBox *visible,*reversed,*minimumEnabled,*maximumEnabled;
  QLineEdit *low,*high,*minimumValue,*maximumValue;
  QPushButton *minimumColor,*maximumColor;
  QGridLayout* colors;
  std::vector<std::array<double,3>> swatches;
};
