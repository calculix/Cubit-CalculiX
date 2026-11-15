#include "Framework/Core/units/CoreFrameworkUnits.hpp"
#include "Framework/Core/units/CoreFrameworkUnitsPersistence.hpp"
#include "Framework/Core/materials/CoreFrameworkMaterials.hpp"
#include "Framework/Core/materials/CoreFrameworkMaterialLibrary.hpp"
#include "Framework/Core/materials/MaterialDefinitions.hpp"
#include "Framework/Core/materials/MaterialProjectPersistence.hpp"
#include "Framework/Core/materials/WorkspaceMaterialStore.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "Framework/Storage/UserOptionsStore.hpp"
#include <QFileInfo>
#include "CubitMessage.hpp"
#include <QCoreApplication>
#include <QTemporaryDir>
#include <algorithm>
#include <array>
#include <cmath>
#include <exception>
#include <limits>
#include <stdexcept>
#include <string>

namespace {
void require(bool accepted, const std::string& message, const std::string& detail = {}) {
  if (!accepted) {
    throw std::runtime_error(message + ": " + detail);
  }
}
void near(double actual, double expected, const std::string& message) {
  const double tolerance = std::max(1e-14, std::abs(expected) * 1e-12);
  require(std::isfinite(actual) && std::abs(actual - expected) <= tolerance, message);
}
MaterialData sample_material(const std::string& name) {
  MaterialData data;
  data.name = name;
  MaterialCard elastic;
  elastic.state = MaterialCardState::Active;
  elastic.model = "iso";
  elastic.values.rows = {{{210e9, 0.3}}};
  data.cards.emplace("elastic", elastic);
  MaterialCard density;
  density.state = MaterialCardState::Inactive;
  density.model = "default";
  density.values.rows = {{{7850.0, 0.0}}, {{7800.0}}};
  data.cards.emplace("density", density);
  MaterialCard expansion;
  expansion.model = "iso";
  expansion.values.rows = {{{1.2e-5}}};
  expansion.values.referenceTemperature = 0.0;
  data.cards.emplace("expansion", expansion);
  return data;
}
void cleared_reference_temperature(const std::string& filename) {
  std::string error;
  MaterialDefinitions definitions;
  auto material = sample_material("Optional temperature fixture");
  {
    HDF5Tool file(filename);
    require(MaterialProjectPersistence::write_material(file, "material", material, error), "Write reference temperature", error);
    material.cards.at("expansion").values.referenceTemperature.reset();
    require(MaterialProjectPersistence::write_material(file, "material", material, error), "Rewrite cleared reference temperature", error);
    require(!file.nameExists("material/cards/expansion/reference_temperature_si"), "Remove stale reference temperature dataset");
    require(file.close(), "Close rewritten material", file.last_error());
  }
  {
    HDF5Tool file(filename, false);
    MaterialData restored;
    require(MaterialProjectPersistence::read_material(file, "material", definitions, restored, error), "Read rewritten material", error);
    require(!restored.cards.at("expansion").values.referenceTemperature.has_value(), "Keep cleared reference temperature absent");
    require(file.close(), "Close restored material", file.last_error());
  }
}
void unit_conversions() {
  SimUnits::CoreFrameworkUnits units;
  const std::array<std::string, 5> systems = {"SI", "N-mm-s", "kN-m-s", "N-mm-ms", "kN-mm-ms"};
  const std::array<double, 5> masses = {1.0, 1e-3, 1e-3, 1000.0, 1.0};
  const std::array<double, 5> densities = {7850.0, 7.85e-9, 7.85, 7.85e-3, 7.85e-6};
  const std::array<double, 5> stresses = {210e9, 210000.0, 210e6, 210000.0, 210.0};
  require(units.system_ids() == std::vector<std::string>(systems.begin(), systems.end()), "Exactly five supported presets");
  std::string error;
  for (std::size_t index = 0; index < systems.size(); ++index) {
    double value = 0.0;
    require(units.system_from_si(1.0, SimUnits::Mass, systems[index], value, error), "Mass conversion", error);
    near(value, masses[index], systems[index] + " coherent mass");
    require(units.system_from_si(7850.0, SimUnits::Density, systems[index], value, error), "Density conversion", error);
    near(value, densities[index], systems[index] + " coherent density");
    require(units.system_from_si(210e9, SimUnits::Stress, systems[index], value, error), "Stress conversion", error);
    near(value, stresses[index], systems[index] + " coherent stress");
    SimUnits::QuantityUnit label;
    require(units.system_unit(systems[index], "density", label, error), "Coherent source-unit label", error);
    require(units.to_si(densities[index], label.unit, label.dimension, label.role, value, error), "Source label matches numeric scale", error);
    near(value, 7850.0, "Density label round trip");
  }
  for (const auto& workspace : units.workspace_ids()) {
    SimUnits::WorkspaceUnitContext context;
    require(units.workspace_context(workspace, context, error), "Read independent workspace", error);
    require(context.system == "N-mm-s", "Default workspace preset");
    std::string display;
    require(units.display_unit(workspace, "mass", display, error) && display == "kg", "Mass input override", error);
    require(units.display_unit(workspace, "density", display, error) && display == "kg/m^3", "Density input override", error);
    require(units.display_unit(workspace, "stress", display, error) && display == "N/mm^2", "Stress input override", error);
  }
  require(units.set_workspace_system("calculix", "SI", error), "Change one workspace", error);
  SimUnits::WorkspaceUnitContext post;
  require(units.workspace_context("postprocessor", post, error) && post.system == "N-mm-s", "Workspace independence", error);
  double value = 0.0;
  require(units.to_si(0.0, "degC", SimUnits::Temperature, SimUnits::UnitRole::AbsoluteTemperature, value, error), "Absolute Celsius", error);
  near(value, 273.15, "Absolute temperature affine offset");
  require(units.to_si(10.0, "degC", SimUnits::Temperature, SimUnits::UnitRole::TemperatureDifference, value, error), "Celsius temperature difference", error);
  near(value, 10.0, "Temperature differences have no offset");
  value = 123.0;
  require(!units.convert(10.0, "degC", "delta_K", value, error), "Reject absolute/difference mismatch");
  near(value, 123.0, "Failed conversion leaves output unchanged");
  require(!units.convert(1.0, "mm", "s", value, error), "Reject incompatible dimensions");
  require(!units.convert(std::numeric_limits<double>::infinity(), "m", "mm", value, error), "Reject nonfinite input");
  require(!units.convert(1.0, "degC/m", "K/m", value, error), "Reject affine compound unit");
  require(!units.from_solver(1.0, SimUnits::Length, "unknown-workspace", value, error), "Reject unknown workspace");
  require(!units.system_to_si(1.0, SimUnits::Length, "", value, error), "Do not infer missing source system");
  for (const auto& system : units.system_ids()) {
    require(units.set_workspace_system("calculix", system, error), "Change preset without reset", error);
    require(units.workspace_context("postprocessor", post, error) && post.system == "N-mm-s", "Preserve other workspace", error);
  }

}
void project_round_trip(const std::string& filename) {
  std::string error;
  SimUnits::CoreFrameworkUnits units;
  require(units.set_workspace_system("calculix", "kN-mm-ms", error), "Set saved solver preset", error);
  require(units.set_workspace_system("report", "SI", error), "Set saved report preset", error);
  require(units.set_display_unit("report", "temperature", "degC", error), "Set saved display preference", error);
  require(units.declare_geometry_unit("mm", error), "Declare source geometry", error);
  MaterialDefinitions definitions;
  WorkspaceMaterialStore store("calculix", definitions);
  MaterialRef kept;
  MaterialRef erased;
  require(store.create(sample_material("Saved steel"), 7, kept, error), "Create typed material", error);
  require(store.create(sample_material("Removed steel"), 0, erased, error), "Advance ID counter", error);
  require(store.erase(erased, error), "Remove material without recycling ID", error);
  const int expectedNext = store.next_id();
  SimUnits::CoreFrameworkUnitsPersistence persistence;
  {
    HDF5Tool file(filename);
    require(file.file != nullptr, "Create temporary project", file.last_error());
    require(persistence.write(file, units, error), "Save unit contexts", error);
    require(MaterialProjectPersistence::save(file, "materials", store, error), "Save typed cards", error);
    require(file.close(), "Close saved project", file.last_error());
  }
  SimUnits::CoreFrameworkUnits restoredUnits;
  WorkspaceMaterialStore restored("calculix", definitions);
  {
    HDF5Tool file(filename, false);
    require(persistence.read(file, restoredUnits, error), "Restore unit contexts", error);
    require(MaterialProjectPersistence::load(file, "materials", restored, definitions, error), "Restore typed cards", error);
    require(file.close(), "Close restored project", file.last_error());
  }
  SimUnits::WorkspaceUnitContext context;
  require(restoredUnits.workspace_context("calculix", context, error), "Read restored context", error);
  require(context.system == "kN-mm-ms" && context.native_system.empty(), "Preserve selected preset without a native lock");
  require(restoredUnits.geometry_unit() == "mm", "Preserve geometry source declaration");
  std::string display;
  require(restoredUnits.display_unit("report", "temperature", display, error) && display == "degC", "Preserve report preference", error);
  const auto* material = restored.find(kept);
  require(material != nullptr && restored.next_id() == expectedNext, "Restore material identity and next ID");
  const auto& density = material->data.cards.at("density");
  require(density.state == MaterialCardState::Inactive, "Retain inactive state");
  near(density.values.rows.at(0).values.at(0), 7850.0, "Inactive values survive save");
  require(density.values.rows.at(0).values.size() == 2 && density.values.rows.at(0).values[1] == 0.0, "Explicit zero temperature survives");
  require(density.values.rows.at(1).values.size() == 1, "Missing optional temperature remains missing");
  require(material->data.cards.at("elastic").values.rows.front().values.size() == 2, "Missing elastic temperature remains missing");
  require(material->data.cards.at("expansion").values.referenceTemperature.has_value() && *material->data.cards.at("expansion").values.referenceTemperature == 0.0, "Optional reference temperature preserves zero");
  require(!density.values.scalar.has_value(), "Missing optional scalar stays missing");
  require(restored.find({"aster", kept.id}) == nullptr, "Reject reference from another workspace");
  require(!restored.erase({"aster", kept.id}, error), "Wrong workspace cannot delete a material");
  MaterialRef next;
  require(restored.create(sample_material("Next steel"), 0, next, error), "Create after restore", error);
  require(next.id == expectedNext && next.id > erased.id, "Deleted IDs are not recycled");
  const auto savedGeneration = restoredUnits.generation();
  {
    HDF5Tool file(filename, false);
    require(file.file != nullptr && file.write_dataset_int_rank_1("schema_version", "cubit-sim/framework/units", {999}), "Create invalid temporary unit schema", file.last_error());
    require(file.close(), "Close invalid temporary project", file.last_error());
  }
  require(!persistence.read(filename, restoredUnits, error), "Reject invalid unit schema");
  require(restoredUnits.workspace_context("calculix", context, error) && context.system == "kN-mm-ms" && context.native_system == "kN-mm-ms", "Failed restore preserves workspace context", error);
  require(restoredUnits.geometry_unit() == "mm" && restoredUnits.generation() == savedGeneration, "Failed restore preserves geometry and generation");
}
void library_file_preference(const QTemporaryDir& temporary) {
  std::string error;
  FrameworkPaths paths;
  paths.componentDirectory = temporary.path().toStdString();
  paths.optionsFilename = temporary.filePath("preferences.hdf5").toStdString();
  UserOptionsStore preferences(paths);
  CoreFrameworkMaterialLibrary library(framework_materials().definitions());
  require(library.initialize(preferences, error), "Initialize shared library preference", error);
  require(library.set_file_option("selected-library.hdf5", error), "Select shared library through Framework options", error);
  const std::string selected = library.filename();
  require(QFileInfo(QString::fromStdString(selected)).fileName() == "selected-library.hdf5", "Resolve relative library path against component directory");
  const std::string invalid = temporary.filePath("not-a-library.hdf5").toStdString();
  {
    HDF5Tool file(invalid);
    require(file.file != nullptr && file.ensureGroup("unrelated-data"), "Create invalid temporary library fixture", file.last_error());
    require(file.close(), "Close invalid temporary library fixture", file.last_error());
  }
  require(!library.set_file_option(invalid, error) && library.filename() == selected, "Reject invalid library without changing the selected file");
  library.shutdown();
  require(!library.set_file_option("another-library.hdf5", error), "Unbound preference service rejects changes");
  CoreFrameworkMaterialLibrary restored(framework_materials().definitions());
  require(restored.initialize(preferences, error) && restored.filename() == selected, "Restore persisted shared library preference", error);
  restored.shutdown();
}
void independent_library_copy(const std::string& filename) {
  std::string error;
  auto& service = framework_materials();
  service.begin_project();
  auto& local = service.workspace("calculix");
  MaterialRef source;
  require(local.create(sample_material("Local steel"), 0, source, error), "Create library source", error);
  CoreFrameworkMaterialLibrary library(service.definitions());
  require(library.set_filename(filename, error) && library.initialize(error), "Initialize isolated test library", error);
  require(library.create_group("Metals", error), "Create library group", error);
  require(library.export_material("calculix", source.id, "Library steel", "Metals", error), "Copy material to library", error);
  int importedId = 0;
  require(library.import_material("calculix", "Library steel", "Metals", "Imported steel", importedId, error), "Import independent material", error);
  require(importedId != source.id, "Imported material has its own identity");
  auto* imported = local.find({"calculix", importedId});
  require(imported != nullptr, "Find imported material");
  MaterialData modified = imported->data;
  modified.cards.at("elastic").values.rows.front().values[0] = 70e9;
  modified.cards.at("density").state = MaterialCardState::Active;
  require(local.replace({"calculix", importedId}, modified, error), "Modify imported material", error);
  MaterialData stored;
  require(library.read_material("calculix", "Library steel", "Metals", stored, error), "Read unchanged library source", error);
  near(stored.cards.at("elastic").values.rows.front().values[0], 210e9, "Library data is independent of imported edits");
  require(stored.cards.at("density").state == MaterialCardState::Inactive, "Library card state is independent");
  near(local.find(source)->data.cards.at("elastic").values.rows.front().values[0], 210e9, "Original local material is independent");
  require(!library.read_material("aster", "Library steel", "Metals", stored, error), "Reject library entry from another solver schema");
  require(!local.replace({"aster", importedId}, modified, error), "Reject a cross-workspace replacement");
  service.begin_project();
}
}

int main(int argc, char** argv) {
  QCoreApplication application(argc, argv);
  try {
    QTemporaryDir temporary;
    require(temporary.isValid(), "Create temporary test directory");
    unit_conversions();
    cleared_reference_temperature(temporary.filePath("cleared-reference.hdf5").toStdString());
    project_round_trip(temporary.filePath("project.cub").toStdString());
    independent_library_copy(temporary.filePath("library.hdf5").toStdString());
    library_file_preference(temporary);
    PRINT_INFO("Framework units and material service checks passed.\n");
    return 0;
  } catch (const H5::Exception& exception) {
    PRINT_ERROR("Framework model test failed: %s\n", exception.getDetailMsg().c_str());
  } catch (const std::exception& exception) {
    PRINT_ERROR("Framework model test failed: %s\n", exception.what());
  } catch (...) {
    PRINT_ERROR("Framework model test failed: unknown error.\n");
  }
  return 1;
}

