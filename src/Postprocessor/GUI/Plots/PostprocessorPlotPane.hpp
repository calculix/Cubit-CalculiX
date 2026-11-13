#ifndef POSTPROCESSORPLOTPANE_HPP
#define POSTPROCESSORPLOTPANE_HPP
#include "Postprocessor/Core/plots/PlotData.hpp"
#include <QWidget>
#include <QString>
#include <functional>
#include <memory>
class QComboBox;
class QLabel;
class PostprocessorPlotChart;
class PostprocessorApplicationInterface;
class PostprocessorPlotPane : public QWidget
{
public:
  using Dispatch = std::function<bool(const QString&,std::string&)>;
  PostprocessorPlotPane(std::weak_ptr<PostprocessorApplicationInterface> application,
    Dispatch submit,Dispatch silent,QWidget* parent);
  void synchronize();
  QSize chart_size() const;
  void cancel();
private:
  void commands_menu();
  void axis_dialog();
  void submit(const QString& command);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Dispatch submitCommand, silentCommand;
  QComboBox* selector;
  QLabel* status;
  PostprocessorPlotChart* chart;
  Postprocessor::PlotData values;
  std::optional<Postprocessor::PlotDefinition> shown;
  std::vector<std::shared_ptr<const SimResults::VtkResultsData>> owners;
  bool synchronizing = false;
};
#endif
