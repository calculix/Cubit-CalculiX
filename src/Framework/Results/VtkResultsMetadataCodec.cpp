#include "VtkResultsMetadataCodec.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonParseError>
#include <vtkDataArray.h>
#include <vtkDataObject.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <algorithm>
#include <cmath>
#include <functional>
#include <limits>
#include <map>

namespace {
constexpr const char* metadataName = "CubitSim.ResultsMetadata";
constexpr int metadataVersion = 1;
constexpr std::size_t maximumMetadataBytes = 16 * 1024 * 1024;

QString text(const std::string& value) {
  return QString::fromUtf8(value.data(), static_cast<int>(value.size()));
}

QJsonObject quantity_json(const SimUnits::QuantityUnit& value) {
  QJsonArray dimensions;
  for (double exponent : value.dimension) {
    dimensions.append(exponent);
  }
  QJsonObject result;
  result.insert("quantity", text(value.quantity));
  result.insert("unit", text(value.unit));
  result.insert("dimension", dimensions);
  result.insert("role", static_cast<int>(value.role));
  return result;
}

bool quantity_read(const QJsonValue& input, SimUnits::QuantityUnit& output, std::string& error) {
  if (!input.isObject()) {
    error = "VTK quantity metadata must be an object.";
    return false;
  }
  const auto object = input.toObject();
  const auto dimensions = object.value("dimension").toArray();
  const auto role = object.value("role");
  if (!object.value("quantity").isString() || !object.value("unit").isString() || dimensions.size() != 7 || !role.isDouble() || role.toDouble() != role.toInt(-1) || role.toInt(-1) < 0 || role.toInt(-1) > 3) {
    error = "Invalid VTK quantity dimensions or temperature role.";
    return false;
  }
  SimUnits::QuantityUnit candidate;
  candidate.quantity = object.value("quantity").toString().toUtf8().toStdString();
  candidate.unit = object.value("unit").toString().toUtf8().toStdString();
  candidate.role = static_cast<SimUnits::UnitRole>(role.toInt());
  for (int i = 0; i < 7; ++i) {
    if (!dimensions[i].isDouble() || !std::isfinite(dimensions[i].toDouble())) {
      error = "VTK quantity dimensions must be finite numbers.";
      return false;
    }
    candidate.dimension[static_cast<std::size_t>(i)] = dimensions[i].toDouble();
  }
  if (!candidate.unit.empty()) {
    double checked = 0.0;
    if (!framework_units().to_si(0.0, candidate.unit, candidate.dimension, candidate.role, checked, error)) {
      return false;
    }
  }
  if (!candidate.quantity.empty()) {
    SimUnits::QuantityUnit definition;
    if (!framework_units().quantity(candidate.quantity, definition, error)) {
      return false;
    }
    if (definition.dimension != candidate.dimension || definition.role != candidate.role) {
      error = "VTK quantity identity disagrees with its dimension or temperature role.";
      return false;
    }
  }
  output = std::move(candidate);
  return true;
}

QJsonObject source_json(const SimResults::VtkResultsData& result) {
  QJsonObject source;
  source.insert("system", text(result.source_units.system));
  source.insert("coordinates", text(result.source_units.coordinate_unit));
  source.insert("time", text(result.source_units.time_unit));
  source.insert("workspace", text(result.source_workspace));
  source.insert("project", text(result.source_project));
  source.insert("run", text(result.run_identity));
  source.insert("result", text(result.result_identity));
  source.insert("solver_launch", text(result.solver_launch_id));
  source.insert("job", result.job_id);
  return source;
}

bool source_read(const QJsonObject& source, SimResults::VtkResultsData& result, std::string& error) {
  for (const auto* key : {"system", "coordinates", "time", "workspace", "project", "run", "result"}) {
    if (!source.value(key).isString()) {
      error = "Invalid VTK source unit or provenance metadata.";
      return false;
    }
  }
  SimUnits::SourceUnitContext units;
  units.system = source.value("system").toString().toUtf8().toStdString();
  units.coordinate_unit = source.value("coordinates").toString().toUtf8().toStdString();
  units.time_unit = source.value("time").toString().toUtf8().toStdString();
  if (!framework_units().validate_source(units, error)) {
    return false;
  }
  result.source_units = std::move(units);
  result.source_workspace = source.value("workspace").toString().toUtf8().toStdString();
  result.source_project = source.value("project").toString().toUtf8().toStdString();
  result.run_identity = source.value("run").toString().toUtf8().toStdString();
  result.result_identity = source.value("result").toString().toUtf8().toStdString();
  result.solver_launch_id = source.value("solver_launch").toString().toUtf8().toStdString();
  if (source.contains("job")) {
    const auto job = source.value("job").toDouble(-2.0);
    if (job < -1 || job > std::numeric_limits<int>::max() || std::floor(job) != job) {
      error = "Invalid VTK source job identity.";
      return false;
    }
    result.job_id = static_cast<int>(job);
  }
  return true;
}

bool visit(vtkDataObject* object, std::size_t id, const std::function<bool(vtkDataSet&, std::size_t)>& callback) {
  if (auto* collection = vtkPartitionedDataSetCollection::SafeDownCast(object)) {
    for (unsigned int i = 0; i < collection->GetNumberOfPartitionedDataSets(); ++i) {
      if (!visit(collection->GetPartitionedDataSet(i), i, callback)) {
        return false;
      }
    }
    return true;
  }
  if (auto* partition = vtkPartitionedDataSet::SafeDownCast(object)) {
    for (unsigned int i = 0; i < partition->GetNumberOfPartitions(); ++i) {
      if (!visit(partition->GetPartition(i), id, callback)) {
        return false;
      }
    }
    return true;
  }
  if (auto* mesh = vtkDataSet::SafeDownCast(object)) {
    return callback(*mesh, id);
  }
  return object == nullptr;
}

bool put(vtkDataObject& object, const QJsonObject& metadata, std::string& error) {
  const auto bytes = QJsonDocument(metadata).toJson(QJsonDocument::Compact);
  if (static_cast<std::size_t>(bytes.size()) > maximumMetadataBytes) {
    error = "VTK result metadata exceeds the supported size.";
    return false;
  }
  auto array = vtkSmartPointer<vtkStringArray>::New();
  array->SetName(metadataName);
  array->InsertNextValue(std::string(bytes.constData(), static_cast<std::size_t>(bytes.size())));
  object.GetFieldData()->RemoveArray(metadataName);
  object.GetFieldData()->AddArray(array);
  return true;
}

bool get(vtkDataObject& object, QJsonObject& metadata, bool& present, std::string& error) {
  present = object.GetFieldData()->HasArray(metadataName) != 0;
  if (!present) {
    return true;
  }
  auto* array = vtkStringArray::SafeDownCast(object.GetFieldData()->GetAbstractArray(metadataName));
  if (!array || array->GetNumberOfValues() != 1 || array->GetValue(0).size() > maximumMetadataBytes) {
    error = "Invalid VTK result metadata array.";
    return false;
  }
  const auto value = array->GetValue(0);
  QJsonParseError parse;
  const auto document = QJsonDocument::fromJson(QByteArray(value.data(), static_cast<int>(value.size())), &parse);
  if (parse.error != QJsonParseError::NoError || !document.isObject()) {
    error = "VTK result metadata is not valid JSON.";
    return false;
  }
  metadata = document.object();
  if (metadata.value("version").toInt(-1) != metadataVersion || !metadata.value("source").isObject()) {
    error = "Unsupported VTK result metadata version.";
    return false;
  }
  return true;
}

QJsonObject field_json(const SimResults::Field& field) {
  QJsonObject result;
  result.insert("array", text(field.array_name));
  result.insert("name", text(field.name));
  result.insert("origin", text(field.source));
  result.insert("set", text(field.set_name));
  result.insert("association", static_cast<int>(field.association));
  result.insert("validity", text(field.validity_array));
  result.insert("interpolated", field.interpolated);
  result.insert("categorical", field.categorical);
  result.insert("role", static_cast<int>(field.role));
  QJsonArray components;
  for (const auto& name : field.components) {
    components.append(text(name));
  }
  result.insert("components", components);
  QJsonArray quantities;
  for (const auto& quantity : field.component_units) {
    quantities.append(quantity_json(quantity));
  }
  result.insert("units", quantities);
  QJsonArray vector;
  if (field.vector_components) {
    for (auto index : *field.vector_components) {
      vector.append(static_cast<double>(index));
    }
  }
  result.insert("vector", vector);
  return result;
}

bool field_read(const QJsonObject& object, vtkDataSet& mesh, std::size_t dataset, SimResults::Field& output, std::string& error) {
  const int association = object.value("association").toInt(-1);
  if (association < 0 || association > 2) {
    error = "VTK mesh field metadata has an invalid association.";
    return false;
  }
  SimResults::Field candidate;
  candidate.dataset_id = dataset;
  candidate.association = static_cast<SimResults::Association>(association);
  candidate.array_name = object.value("array").toString().toUtf8().toStdString();
  vtkDataArray* array = nullptr;
  if (candidate.association == SimResults::Association::Element) {
    array = mesh.GetCellData()->GetArray(candidate.array_name.c_str());
  } else {
    array = mesh.GetPointData()->GetArray(candidate.array_name.c_str());
  }
  const auto components = object.value("components").toArray();
  const auto quantities = object.value("units").toArray();
  if (!array || components.size() != array->GetNumberOfComponents() || (!quantities.isEmpty() && quantities.size() != components.size())) {
    error = "VTK field metadata does not match its numeric array.";
    return false;
  }
  vtkIdType expected = mesh.GetNumberOfPoints();
  if (candidate.association == SimResults::Association::Element) {
    expected = mesh.GetNumberOfCells();
  }
  if (array->GetNumberOfTuples() != expected) {
    error = "VTK field metadata references an array with an invalid tuple count.";
    return false;
  }
  candidate.name = object.value("name").toString().toUtf8().toStdString();
  candidate.source = object.value("origin").toString().toUtf8().toStdString();
  candidate.set_name = object.value("set").toString().toUtf8().toStdString();
  candidate.validity_array = object.value("validity").toString().toUtf8().toStdString();
  if (!candidate.validity_array.empty()) {
    vtkDataArray* validity = mesh.GetPointData()->GetArray(candidate.validity_array.c_str());
    if (candidate.association == SimResults::Association::Element) {
      validity = mesh.GetCellData()->GetArray(candidate.validity_array.c_str());
    }
    if (!validity || validity->GetNumberOfComponents() != 1 || validity->GetNumberOfTuples() != expected) {
      error = "VTK field metadata references an invalid validity mask.";
      return false;
    }
  }
  candidate.interpolated = object.value("interpolated").toBool();
  candidate.categorical = object.value("categorical").toBool();
  const int role = object.value("role").toInt(-1);
  if (role < 0 || role > 1) {
    error = "VTK field metadata contains an unsupported physical role.";
    return false;
  }
  candidate.role = static_cast<SimResults::FieldRole>(role);
  for (const auto& component : components) {
    if (!component.isString()) {
      error = "VTK field component names must be strings.";
      return false;
    }
    candidate.components.push_back(component.toString().toUtf8().toStdString());
  }
  for (const auto& quantity : quantities) {
    SimUnits::QuantityUnit value;
    if (!quantity_read(quantity, value, error)) {
      return false;
    }
    candidate.component_units.push_back(std::move(value));
  }
  const auto vector = object.value("vector").toArray();
  if (!vector.isEmpty()) {
    if (vector.size() != 3) {
      error = "VTK physical vector metadata needs three components.";
      return false;
    }
    std::array<std::size_t, 3> indices{};
    for (int i = 0; i < 3; ++i) {
      const auto value = vector[i].toDouble(-1.0);
      if (value < 0 || value >= components.size() || std::floor(value) != value) {
        error = "VTK vector component index is outside its array.";
        return false;
      }
      indices[static_cast<std::size_t>(i)] = static_cast<std::size_t>(value);
    }
    candidate.vector_components = indices;
    if (indices[0] == indices[1] || indices[0] == indices[2] || indices[1] == indices[2]) {
      error = "VTK physical vector components must be distinct.";
      return false;
    }
  }
  output = std::move(candidate);
  return true;
}
}

bool SimResults::VtkResultsMetadataCodec::write_frame(vtkDataObject& target, const VtkResultsData& result, std::size_t frame, std::string& error) {
  if (frame >= result.increments.size()) {
    error = "VTK metadata frame index is invalid.";
    return false;
  }
  return visit(&target, 0, [&](vtkDataSet& mesh, std::size_t id) {
    QJsonObject metadata;
    metadata.insert("version", metadataVersion);
    metadata.insert("source", source_json(result));
    metadata.insert("independent_variable", quantity_json(result.increments[frame].independent_variable));
    std::string coordinates = result.increments[frame].coordinate_unit;
    if (coordinates.empty()) {
      coordinates = result.source_units.coordinate_unit;
    }
    metadata.insert("coordinates", text(coordinates));
    QJsonArray fields;
    for (const auto& field : result.increments[frame].fields) {
      if (field.dataset_id == id) {
        vtkDataArray* array = mesh.GetPointData()->GetArray(field.array_name.c_str());
        if (field.association == SimResults::Association::Element) {
          array = mesh.GetCellData()->GetArray(field.array_name.c_str());
        }
        if (!array) {
          continue;
        }
        fields.append(field_json(field));
      }
    }
    metadata.insert("fields", fields);
    const auto dataset = std::find_if(result.datasets.begin(), result.datasets.end(), [&](const auto& value) {
      return value.id == id;
    });
    if (dataset != result.datasets.end()) {
      if (dataset->solid_color)
      {
        QJsonArray rgb;
        for (double channel : *dataset->solid_color)
        {
          if (!std::isfinite(channel) || channel < 0 || channel > 1)
          {
            error = "VTK solid color requires finite RGB channels in [0,1].";
            return false;
          }
          rgb.append(channel);
        }
        metadata.insert("solid_color",rgb);
      }
      QJsonObject material;
      material.insert("array", text(dataset->material_id_array));
      material.insert("workspace", text(dataset->material_workspace));
      material.insert("project", text(dataset->material_project));
      metadata.insert("materials", material);
    }
    return put(mesh, metadata, error);
  });
}

bool SimResults::VtkResultsMetadataCodec::write_table(vtkTable& target, const VtkResultsData& result, const std::optional<std::size_t>& frame, std::size_t table, std::string& error) {
  const std::vector<ResultsTableMetadata>* entries = &result.table_metadata;
  if (frame) {
    if (*frame >= result.increments.size()) {
      error = "VTK table metadata frame index is invalid.";
      return false;
    }
    entries = &result.increments[*frame].table_metadata;
  }
  QJsonObject metadata;
  metadata.insert("version", metadataVersion);
  metadata.insert("source", source_json(result));
  QJsonArray columns;
  if (table < entries->size()) {
    const auto& entry = (*entries)[table];
    if (!entry.columns.empty() && entry.columns.size() != static_cast<std::size_t>(target.GetNumberOfColumns())) {
      error = "VTK table units do not match its columns.";
      return false;
    }
    metadata.insert("name", text(entry.name));
    for (const auto& quantity : entry.columns) {
      columns.append(quantity_json(quantity));
    }
  }
  metadata.insert("columns", columns);
  return put(target, metadata, error);
}

bool SimResults::VtkResultsMetadataCodec::write(VtkResultsData& result, std::string& error) {
  for (std::size_t frame = 0; frame < result.increments.size(); ++frame) {
    auto& increment = result.increments[frame];
    if (increment.dataset && !write_frame(*increment.dataset, result, frame, error)) {
      return false;
    }
    for (std::size_t table = 0; table < increment.tables.size(); ++table) {
      if (increment.tables[table] && !write_table(*increment.tables[table], result, frame, table, error)) {
        return false;
      }
    }
  }
  for (std::size_t table = 0; table < result.tables.size(); ++table) {
    if (result.tables[table] && !write_table(*result.tables[table], result, std::nullopt, table, error)) {
      return false;
    }
  }
  error.clear();
  return true;
}

bool SimResults::VtkResultsMetadataCodec::read(VtkResultsData& result, std::string& error) {
  VtkResultsData candidate = result;
  std::optional<QJsonObject> source;
  std::map<std::size_t, QJsonObject> materialsSeen;
  std::map<std::size_t,std::optional<std::array<double,3>>> colorsSeen;
  bool missingMeshMetadata = false;
  const auto acceptSource = [&](const QJsonObject& metadata) {
    const auto value = metadata.value("source").toObject();
    if (source && *source != value) {
      error = "VTK leaves contain inconsistent source units or provenance.";
      return false;
    }
    if (!source_read(value, candidate, error)) {
      return false;
    }
    source = value;
    return true;
  };
  const auto readTables = [&](const auto& tables, auto& entries) {
    entries.resize(tables.size());
    for (std::size_t i = 0; i < tables.size(); ++i) {
      if (!tables[i]) {
        continue;
      }
      QJsonObject metadata;
      bool present = false;
      if (!get(*tables[i], metadata, present, error)) {
        return false;
      }
      if (!present) {
        continue;
      }
      if (!acceptSource(metadata)) {
        return false;
      }
      const auto columns = metadata.value("columns").toArray();
      if (!columns.isEmpty() && columns.size() != tables[i]->GetNumberOfColumns()) {
        error = "VTK table metadata has the wrong number of columns.";
        return false;
      }
      entries[i].name = metadata.value("name").toString().toUtf8().toStdString();
      entries[i].columns.clear();
      for (const auto& column : columns) {
        SimUnits::QuantityUnit quantity;
        if (!quantity_read(column, quantity, error)) {
          return false;
        }
        entries[i].columns.push_back(std::move(quantity));
      }
    }
    return true;
  };
  for (auto& frame : candidate.increments) {
    std::map<std::string, QJsonObject> fieldsSeen;
    std::optional<QJsonValue> independentVariable;
    std::optional<std::string> coordinateUnit;
    if (!visit(frame.dataset.get(), 0, [&](vtkDataSet& mesh, std::size_t id) {
      QJsonObject metadata;
      bool present = false;
      if (!get(mesh, metadata, present, error)) {
        return false;
      }
      if (!present) {
        if (mesh.GetNumberOfPoints() != 0 || mesh.GetNumberOfCells() != 0) {
          missingMeshMetadata = true;
        }
        return true;
      }
      if (!acceptSource(metadata)) {
        return false;
      }
      frame.coordinate_unit = candidate.source_units.coordinate_unit;
      if (metadata.contains("coordinates")) {
        if (!metadata.value("coordinates").isString()) {
          error = "VTK frame coordinate units must be a string.";
          return false;
        }
        frame.coordinate_unit = metadata.value("coordinates").toString().toUtf8().toStdString();
      }
      SimUnits::SourceUnitContext frameUnits;
      frameUnits.coordinate_unit = frame.coordinate_unit;
      if (!framework_units().validate_source(frameUnits, error)) {
        return false;
      }
      if (coordinateUnit && *coordinateUnit != frame.coordinate_unit) {
        error = "VTK partitions contain inconsistent coordinate units.";
        return false;
      }
      coordinateUnit = frame.coordinate_unit;
      if (metadata.contains("independent_variable")) {
        const auto variable = metadata.value("independent_variable");
        if (independentVariable && *independentVariable != variable) {
          error = "VTK partitions contain inconsistent analysis-parameter units.";
          return false;
        }
        if (!quantity_read(variable, frame.independent_variable, error)) {
          return false;
        }
        independentVariable = variable;
      }
      for (const auto& value : metadata.value("fields").toArray()) {
        SimResults::Field field;
        if (!value.isObject() || !field_read(value.toObject(), mesh, id, field, error)) {
          return false;
        }
        const auto key = std::to_string(id) + "/" + std::to_string(static_cast<int>(field.association)) + "/" + field.array_name;
        const auto prior = fieldsSeen.find(key);
        if (prior != fieldsSeen.end() && prior->second != value.toObject()) {
          error = "VTK partitions contain inconsistent field quantities or roles.";
          return false;
        }
        fieldsSeen[key] = value.toObject();
        const auto existing = std::find_if(frame.fields.begin(), frame.fields.end(), [&](const auto& item) {
          return item.dataset_id == id && item.array_name == field.array_name && item.association == field.association;
        });
        if (existing == frame.fields.end()) {
          frame.fields.push_back(std::move(field));
        } else {
          *existing = std::move(field);
        }
      }
      std::optional<std::array<double,3>> solidColor;
      if (metadata.contains("solid_color"))
      {
        const auto rgb = metadata.value("solid_color").toArray();
        if (!metadata.value("solid_color").isArray() || rgb.size() != 3)
        {
          error = "VTK solid color requires three RGB channels.";
          return false;
        }
        std::array<double,3> channels{};
        for (int channel = 0; channel < 3; ++channel)
        {
          const auto value = rgb[channel];
          if (!value.isDouble() || !std::isfinite(value.toDouble()) || value.toDouble() < 0 || value.toDouble() > 1)
          {
            error = "VTK solid color requires finite RGB channels in [0,1].";
            return false;
          }
          channels[static_cast<std::size_t>(channel)] = value.toDouble();
        }
        solidColor = channels;
      }
      const auto priorColor = colorsSeen.find(id);
      if (priorColor != colorsSeen.end() && priorColor->second != solidColor)
      {
        error = "VTK frames/partitions contain inconsistent solid colors for a dataset.";
        return false;
      }
      colorsSeen[id] = solidColor;
      const auto material = metadata.value("materials").toObject();
      const auto priorMaterial = materialsSeen.find(id);
      if (priorMaterial != materialsSeen.end() && priorMaterial->second != material) {
        error = "VTK partitions contain inconsistent scoped material references.";
        return false;
      }
      materialsSeen[id] = material;
      for (auto& dataset : candidate.datasets) {
        if (dataset.id != id) {
          continue;
        }
        dataset.solid_color = solidColor;
        dataset.material_id_array = material.value("array").toString().toUtf8().toStdString();
        dataset.material_workspace = material.value("workspace").toString().toUtf8().toStdString();
        dataset.material_project = material.value("project").toString().toUtf8().toStdString();
        if (!dataset.material_id_array.empty()) {
          auto* ids = mesh.GetCellData()->GetArray(dataset.material_id_array.c_str());
          if (!ids || ids->GetNumberOfComponents() != 1 || ids->GetNumberOfTuples() != mesh.GetNumberOfCells() || dataset.material_workspace.empty() || dataset.material_project.empty()) {
            error = "VTK material references require scoped cell IDs.";
            return false;
          }
          for (vtkIdType cell = 0; cell < ids->GetNumberOfTuples(); ++cell) {
            const double value = ids->GetComponent(cell, 0);
            if (!std::isfinite(value) || value < 0 || value > std::numeric_limits<int>::max() || std::floor(value) != value) {
              error = "VTK material IDs must be nonnegative workspace integers.";
              return false;
            }
          }
          for (auto& field : frame.fields) {
            if (field.dataset_id == id && field.association == Association::Element && field.array_name == dataset.material_id_array) {
              field.categorical = true;
            }
          }
        }
      }
      return true;
    })) {
      return false;
    }
    if (!readTables(frame.tables, frame.table_metadata)) {
      return false;
    }
  }
  if (!readTables(candidate.tables, candidate.table_metadata)) {
    return false;
  }
  if (source && missingMeshMetadata) {
    error = "VTK composite sources cannot mix declared and undeclared mesh metadata.";
    return false;
  }
  result = std::move(candidate);
  error.clear();
  return true;
}
