#include "TopologyIterationPropertiesWidget.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include "CubitMessage.hpp"

TopologyIterationPropertiesWidget::TopologyIterationPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool TopologyIterationPropertiesWidget::setTopologyIteration(int topologyId, int iterationId)
{
  clear();
  CalculiXTopologyInterface core;
  for (const auto& row : core.get_topology_iteration_data(topologyId))
  {
    if (row.size() < 9 || row[0] != std::to_string(iterationId))
    {
      continue;
    }
    addIdentity("Topology iteration", iterationId);
    auto* results = addGroup("iteration", "Iteration", PropertySection::Data);
    TopologyMethod method = TopologyMethod::Unspecified;
    const bool methodAvailable = core.get_topology_iteration_method(topologyId, iterationId, method);
    const QString methodName = !methodAvailable ? "Unavailable" : (method == TopologyMethod::LevelSet ? "Level set" : "Density-based");
    addValue(results, "Method", methodName);
    addReference(results, PropertyReferenceKind::Job, "Job", QString::fromStdString(row[1]));
    QString status;
    const int statusCode = QString::fromStdString(row[2]).toInt();
    switch (statusCode)
    {
      case -1: status = "Not started"; break;
      case 1: status = "Running"; break;
      case 2: status = "Finished"; break;
      case 3: status = "Killed"; break;
      case 4: status = "Failed"; break;
      case 5: status = "Waiting"; break;
      default: status = "Unknown"; break;
    }
    addValue(results, "Job status", status);
    addValue(results, "Objective", QString::fromStdString(row[3]));
    addValue(results, "Volume fraction", QString::fromStdString(row[4]));
    addValue(results, "Selected for results", row[5].empty() ? QString() : (row[5] == "1" ? "Yes" : "No"));
    const QString changeLabel = !methodAvailable ? "Maximum design change" : (method == TopologyMethod::LevelSet ? "Maximum field change" : "Maximum density change");
    addValue(results, changeLabel, QString::fromStdString(row[6]));
    addValue(results, "Relative objective change", QString::fromStdString(row[7]));
    addValue(results, "Converged", row[8].empty() ? QString() : (row[8] == "1" ? "Yes" : "No"));
    if (row.size() >= 13)
    {
      addValue(results, "Displacement", QString::fromStdString(row[9]));
      addValue(results, "Displacement node", QString::fromStdString(row[10]));
      addValue(results, "Displacement nodeset", QString::fromStdString(row[11]));
      addValue(results, "Displacement component", QString::fromStdString(row[12]));
    }
    return true;
  }
  PRINT_ERROR("Topology %d iteration %d is unavailable.\n", topologyId, iterationId);
  return false;
}
