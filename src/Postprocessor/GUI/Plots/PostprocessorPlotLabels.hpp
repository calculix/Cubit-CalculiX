#pragma once
#include "Postprocessor/Core/plots/PlotDefinition.hpp"
#include <QString>
#include <functional>
class QWidget;
QString plot_label_command(const std::string& plot,const std::string& target,const std::string& value,const std::string& curve = {});
void edit_plot_labels(QWidget* parent,const Postprocessor::PlotDefinition& definition,const std::function<void(const QString&)>& submit);
