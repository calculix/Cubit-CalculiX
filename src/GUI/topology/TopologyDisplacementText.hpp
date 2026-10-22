#ifndef TOPOLOGYDISPLACEMENTTEXT_HPP
#define TOPOLOGYDISPLACEMENTTEXT_HPP

#include <QLocale>
#include <QString>
#include <cmath>
#include <string>
#include <vector>

inline QString topologyDisplacementText(const std::vector<std::string>& row)
{
  if (row.size() < 13 || row[9].empty())
  {
    return "Displacement: not available for this iteration";
  }

  bool value_valid = false;
  bool node_valid = false;
  bool nodeset_valid = false;
  bool component_valid = false;

  const double value = QString::fromStdString(row[9]).toDouble(&value_valid);
  const int node = QString::fromStdString(row[10]).toInt(&node_valid);
  const int nodeset = QString::fromStdString(row[11]).toInt(&nodeset_valid);
  const int component = QString::fromStdString(row[12]).toInt(&component_valid);

  if (!value_valid || !std::isfinite(value) || value < 0.0 || !node_valid || node <= 0 || !nodeset_valid || nodeset <= 0 || !component_valid || component < 0 || component > 3)
  {
    return "Displacement: invalid measurement";
  }

  const QString components[] = {"magnitude", "|X|", "|Y|", "|Z|"};
  return QString("Maximum displacement (%1): %2\nNode: %3; nodeset: %4; global coordinates").arg(components[component]).arg(QLocale().toString(value, 'g', 8)).arg(node).arg(nodeset);
}

#endif // TOPOLOGYDISPLACEMENTTEXT_HPP
