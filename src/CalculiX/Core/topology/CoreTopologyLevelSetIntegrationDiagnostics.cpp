#include "CoreTopologyLevelSetIntegrationDiagnostics.hpp"
#include "CoreTopologyLevelSet.hpp"
#include "ElementInterpolation.hpp"
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <QSaveFile>
#include <QString>
#include <QUuid>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <limits>
#include <locale>
#include <set>
#include <sstream>
#include <utility>

namespace
{
using Diagnostics = CoreTopologyLevelSetIntegrationDiagnostics;

bool fail(const std::string& message, std::string& error)
{
  error = message;
  return false;
}

bool number(const QJsonObject& object, const char* key, double& result)
{
  const auto value = object.value(QLatin1String(key));
  if (!value.isDouble() || !std::isfinite(value.toDouble()))
  {
    return false;
  }
  result = value.toDouble();
  return true;
}

bool integer(const QJsonObject& object, const char* key, int& result)
{
  double value = 0.0;
  if (!number(object, key, value) || std::floor(value) != value ||
      value < (std::numeric_limits<int>::min)() || value > (std::numeric_limits<int>::max)())
  {
    return false;
  }
  result = static_cast<int>(value);
  return true;
}

bool valid(const Diagnostics::Capture& capture, std::string& error)
{
  if (capture.iteration_id < 1 || (capture.source_iteration != -1 &&
      (capture.source_iteration < 1 || capture.source_iteration >= capture.iteration_id)) ||
      (capture.phase != "geometry" && capture.phase != "sensitivities") || capture.element_id < 1 ||
      !ElementInterpolation::matches_cubit_type(capture.calculix_type, capture.cubit_type) ||
      capture.nodes.empty() || capture.nodes.size() > 20 ||
      capture.nodes.size() != ElementInterpolation::node_count(capture.calculix_type) ||
      capture.coordinates.size() != capture.nodes.size() || capture.values.size() != capture.nodes.size() ||
      !std::isfinite(capture.volume) || capture.volume <= 0.0 || capture.error.empty() ||
      !capture.diagnostics.refinement_limit_reached ||
      !CoreTopologyLevelSet::validate_settings(capture.settings, &error))
  {
    return fail("Invalid level-set integration capture input/settings.", error);
  }
  std::set<int> nodes;
  for (std::size_t node = 0; node < capture.nodes.size(); ++node)
  {
    if (capture.nodes[node] < 1 || !nodes.insert(capture.nodes[node]).second ||
        !std::isfinite(capture.values[node]) || capture.values[node] < -1.0 || capture.values[node] > 1.0)
    {
      return fail("Invalid level-set integration capture nodal data.", error);
    }
    for (double coordinate : capture.coordinates[node])
    {
      if (!std::isfinite(coordinate))
      {
        return fail("Non-finite level-set integration capture coordinate.", error);
      }
    }
  }
  if (capture.diagnostics.comparisons.size() != static_cast<std::size_t>(capture.settings.integration_max_refinements))
  {
    return fail("Incomplete level-set integration capture comparisons.", error);
  }
  int subdivisions = 2;
  for (const auto& comparison : capture.diagnostics.comparisons)
  {
    if (comparison.subdivisions != subdivisions || comparison.tolerance != capture.settings.integration_tolerance ||
        comparison.derivative_node_index < -1 || comparison.derivative_node_index >= static_cast<int>(capture.nodes.size()) ||
        (comparison.scaled_derivative_error > 0.0 && comparison.derivative_node_index < 0) ||
        !std::isfinite(comparison.relative_volume_error) || comparison.relative_volume_error < 0.0 ||
        !std::isfinite(comparison.solid_fraction_error) || comparison.solid_fraction_error < 0.0 ||
        !std::isfinite(comparison.scaled_derivative_error) || comparison.scaled_derivative_error < 0.0 ||
        !std::isfinite(comparison.maximum_error) || comparison.maximum_error <= comparison.tolerance ||
        comparison.maximum_error != (std::max)(comparison.relative_volume_error,
          (std::max)(comparison.solid_fraction_error, comparison.scaled_derivative_error)))
    {
      return fail("Invalid level-set integration capture comparison.", error);
    }
    subdivisions *= 2;
  }
  return true;
}
}

CoreTopologyLevelSetIntegrationDiagnostics::Comparison CoreTopologyLevelSetIntegrationDiagnostics::compare(
  const Estimate& previous, const Estimate& current, double width, int subdivisions, double tolerance)
{
  Comparison result;
  result.subdivisions = subdivisions;
  result.tolerance = tolerance;
  result.relative_volume_error = std::abs(current.volume - previous.volume) / (std::max)(current.volume, previous.volume);
  result.solid_fraction_error = std::abs(current.solid - previous.solid);
  for (std::size_t node = 0; node < current.derivatives.size(); ++node)
  {
    const double error = width * std::abs(current.derivatives[node] - previous.derivatives[node]);
    if (error > result.scaled_derivative_error)
    {
      result.scaled_derivative_error = error;
      result.derivative_node_index = static_cast<int>(node);
    }
  }
  result.maximum_error = (std::max)(result.relative_volume_error,
    (std::max)(result.solid_fraction_error, result.scaled_derivative_error));
  return result;
}

std::string CoreTopologyLevelSetIntegrationDiagnostics::describe(const Result& result)
{
  if (result.comparisons.empty())
  {
    return {};
  }
  const auto& last = result.comparisons.back();
  std::ostringstream text;
  text.imbue(std::locale::classic());
  text << std::setprecision(17) << " Subdivisions " << last.subdivisions
       << "; relative volume error " << last.relative_volume_error
       << "; solid-fraction error " << last.solid_fraction_error
       << "; width-scaled derivative error " << last.scaled_derivative_error
       << "; derivative node index (zero-based Cubit order) " << last.derivative_node_index
       << "; maximum error " << last.maximum_error << "; tolerance " << last.tolerance << ".";
  return text.str();
}

bool CoreTopologyLevelSetIntegrationDiagnostics::write_capture(const std::string& directory, const Capture& capture, std::string& path, std::string& error)
{
  path.clear();
  error.clear();
  if (directory.empty() || !valid(capture, error))
  {
    return fail("Cannot write integration capture: " + error, error);
  }
  QJsonObject object;
  object["iteration_id"] = capture.iteration_id;
  object["source_iteration"] = capture.source_iteration;
  object["phase"] = QString::fromStdString(capture.phase);
  object["element_id"] = capture.element_id;
  object["calculix_type"] = QString::fromStdString(capture.calculix_type);
  object["cubit_type"] = capture.cubit_type;
  object["volume"] = capture.volume;
  object["preserved"] = capture.preserved;
  object["error"] = QString::fromStdString(capture.error);
  QJsonObject settings;
  settings["interface_width"] = capture.settings.interface_width;
  settings["regularization_length"] = capture.settings.regularization_length;
  settings["move_limit"] = capture.settings.move_limit;
  settings["field_tolerance"] = capture.settings.field_tolerance;
  settings["seed_spacing"] = capture.settings.seed_spacing;
  settings["seed_radius"] = capture.settings.seed_radius;
  settings["integration_tolerance"] = capture.settings.integration_tolerance;
  settings["integration_max_refinements"] = capture.settings.integration_max_refinements;
  object["settings"] = settings;
  QJsonArray nodes;
  for (std::size_t index = 0; index < capture.nodes.size(); ++index)
  {
    QJsonObject node;
    node["id"] = capture.nodes[index];
    node["x"] = capture.coordinates[index][0];
    node["y"] = capture.coordinates[index][1];
    node["z"] = capture.coordinates[index][2];
    node["value"] = capture.values[index];
    nodes.append(node);
  }
  object["nodes"] = nodes;
  QJsonArray comparisons;
  for (const auto& comparison : capture.diagnostics.comparisons)
  {
    QJsonObject row;
    row["subdivisions"] = comparison.subdivisions;
    row["relative_volume_error"] = comparison.relative_volume_error;
    row["solid_fraction_error"] = comparison.solid_fraction_error;
    row["scaled_derivative_error"] = comparison.scaled_derivative_error;
    row["derivative_node_index"] = comparison.derivative_node_index;
    row["maximum_error"] = comparison.maximum_error;
    row["tolerance"] = comparison.tolerance;
    comparisons.append(row);
  }
  object["comparisons"] = comparisons;
  QDir folder(QString::fromStdString(directory));
  if (!folder.mkpath("."))
  {
    return fail("Cannot create integration capture directory.", error);
  }
  const QString name = QString("iteration_%1_element_%2_%3.json").arg(capture.iteration_id).arg(capture.element_id)
    .arg(QUuid::createUuid().toString(QUuid::WithoutBraces));
  QSaveFile file(folder.filePath(name));
  file.setDirectWriteFallback(false);
  const QByteArray bytes = QJsonDocument(object).toJson(QJsonDocument::Indented);
  if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
  {
    return fail("Cannot commit integration capture: " + file.errorString().toStdString(), error);
  }
  path = folder.filePath(name).toStdString();
  return true;
}

bool CoreTopologyLevelSetIntegrationDiagnostics::read_capture(const std::string& path, Capture& capture, std::string& error)
{
  capture = {};
  error.clear();
  QFile file(QString::fromStdString(path));
  if (!file.open(QIODevice::ReadOnly) || file.size() <= 0 || file.size() > 1024 * 1024)
  {
    return fail("Cannot read integration capture or capture exceeds 1 MiB.", error);
  }
  QJsonParseError parse;
  const auto document = QJsonDocument::fromJson(file.readAll(), &parse);
  if (parse.error != QJsonParseError::NoError || !document.isObject())
  {
    return fail("Invalid integration capture JSON.", error);
  }
  const auto object = document.object();
  Capture candidate;
  if (!integer(object, "iteration_id", candidate.iteration_id) || !integer(object, "source_iteration", candidate.source_iteration) ||
      !integer(object, "element_id", candidate.element_id) || !integer(object, "cubit_type", candidate.cubit_type) ||
      !number(object, "volume", candidate.volume) || !object["preserved"].isBool() ||
      !object["phase"].isString() || !object["calculix_type"].isString() || !object["error"].isString() ||
      !object["settings"].isObject() || !object["nodes"].isArray() || !object["comparisons"].isArray())
  {
    return fail("Missing or invalid integration capture fields.", error);
  }
  candidate.phase = object["phase"].toString().toStdString();
  candidate.calculix_type = object["calculix_type"].toString().toStdString();
  candidate.error = object["error"].toString().toStdString();
  candidate.preserved = object["preserved"].toBool();
  const auto settings = object["settings"].toObject();
  if (!number(settings, "interface_width", candidate.settings.interface_width) ||
      !number(settings, "regularization_length", candidate.settings.regularization_length) ||
      !number(settings, "move_limit", candidate.settings.move_limit) ||
      !number(settings, "field_tolerance", candidate.settings.field_tolerance) ||
      !number(settings, "seed_spacing", candidate.settings.seed_spacing) ||
      !number(settings, "seed_radius", candidate.settings.seed_radius) ||
      !number(settings, "integration_tolerance", candidate.settings.integration_tolerance) ||
      !integer(settings, "integration_max_refinements", candidate.settings.integration_max_refinements))
  {
    return fail("Invalid integration capture settings.", error);
  }
  const auto nodes = object["nodes"].toArray();
  const auto comparisons = object["comparisons"].toArray();
  if (nodes.isEmpty() || nodes.size() > 20 || comparisons.isEmpty() || comparisons.size() > 5)
  {
    return fail("Integration capture exceeds element/refinement bounds.", error);
  }
  for (const auto& entry : nodes)
  {
    const auto node = entry.toObject();
    int id = -1;
    std::array<double, 3> coordinates{};
    double value = 0.0;
    if (!entry.isObject() || !integer(node, "id", id) || !number(node, "x", coordinates[0]) ||
        !number(node, "y", coordinates[1]) || !number(node, "z", coordinates[2]) || !number(node, "value", value))
    {
      return fail("Invalid integration capture node.", error);
    }
    candidate.nodes.push_back(id);
    candidate.coordinates.push_back(coordinates);
    candidate.values.push_back(value);
  }
  for (const auto& entry : comparisons)
  {
    const auto row = entry.toObject();
    Comparison comparison;
    if (!entry.isObject() || !integer(row, "subdivisions", comparison.subdivisions) ||
        !integer(row, "derivative_node_index", comparison.derivative_node_index) ||
        !number(row, "relative_volume_error", comparison.relative_volume_error) ||
        !number(row, "solid_fraction_error", comparison.solid_fraction_error) ||
        !number(row, "scaled_derivative_error", comparison.scaled_derivative_error) ||
        !number(row, "maximum_error", comparison.maximum_error) || !number(row, "tolerance", comparison.tolerance))
    {
      return fail("Invalid integration capture comparison fields.", error);
    }
    candidate.diagnostics.comparisons.push_back(comparison);
  }
  candidate.diagnostics.refinement_limit_reached = true;
  if (!valid(candidate, error))
  {
    return false;
  }
  capture = std::move(candidate);
  return true;
}
