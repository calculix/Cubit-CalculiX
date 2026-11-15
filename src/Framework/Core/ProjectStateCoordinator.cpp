#include "ProjectStateCoordinator.hpp"
#include "CubitSimVersion.hpp"
#include "materials/FrameworkMaterialsInterface.hpp"
#include "materials/MaterialProjectPersistence.hpp"
#include "materials/WorkspaceMaterialStore.hpp"
#include "units/CoreFrameworkUnits.hpp"
#include "units/CoreFrameworkUnitsPersistence.hpp"
#include "Framework/Storage/HDF5Tool.hpp"
#include "Framework/Storage/ModelFileMetadata.hpp"
#include "CubitMessage.hpp"
#include <QScopedValueRollback>
#include <QScopeGuard>
#include <QFileInfo>
#include <QUuid>
#include <exception>
#include <limits>
#include <utility>

namespace
{
std::string workspaceLabel(ProjectStateProviderInterface* provider) {
  const auto id = provider->project_state_id();
  if (id == "calculix") {
    return "CalculiX";
  }
  if (id == "postprocessor") {
    return "Postprocessor";
  }
  if (id == "report") {
    return "Report";
  }
  return id;
}

bool projectMetadata(const std::string& filename, bool saving, bool& present, std::string& error) {
  try {
    HDF5Tool file(filename, false);
    if (file.file == nullptr) {
      error = file.last_error();
      return false;
    }
    present = file.nameExists(CubitSimModelFile::Metadata);
    std::string version;
    bool completed = true;
    if (saving) {
      completed = CubitSimModelFile::write(file, CubitSim::Version, error);
    } else if (present) {
      completed = CubitSimModelFile::read(file, version, error);
    }
    SimUnits::CoreFrameworkUnitsPersistence units;
    FrameworkMaterialsInterface materials("calculix", MaterialValueSpace::Canonical);
    SimUnits::CoreFrameworkUnits candidateUnits = framework_units();
    WorkspaceMaterialStore candidateMaterials("calculix", framework_materials().definitions());
    std::string candidateIdentity;
    const std::string identityGroup = "cubit-sim/framework/model";
    const std::string materialGroup = "cubit-sim/framework/materials/calculix";
    if (completed && saving) {
      completed = units.write(file, framework_units(), error) && materials.save(file, materialGroup, error) && file.ensureGroup(identityGroup) && file.write_dataset_string_rank_1("project_id", identityGroup, {framework_materials().project_id()});
    } else if (completed) {
      completed = units.read(file, candidateUnits, error) && MaterialProjectPersistence::load(file, materialGroup, candidateMaterials, framework_materials().definitions(), error);
      if (completed && file.nameExists(identityGroup)) {
        std::vector<std::string> identity;
        completed = file.read_dataset_string_rank_1("project_id", identityGroup, identity) && identity.size() == 1;
        if (completed) {
          candidateIdentity = identity.front();
          completed = !QUuid(QString::fromStdString(candidateIdentity)).isNull();
        }
        if (!completed) {
          error = "Invalid material project identity.";
        }
      } else if (completed) {
        candidateIdentity = QUuid::createUuid().toString(QUuid::WithoutBraces).toStdString();
      }
    }
    if (!completed && error.empty()) {
      error = "Invalid framework model data: " + file.last_error();
    }
    const bool closed = file.close();
    if (completed && !closed) {
      error = file.last_error();
    }
    if (!completed || !closed) {
      return false;
    }
    if (!saving) {
      auto& targetMaterials = framework_materials().workspace("calculix");
      if (!framework_materials().set_project_id(candidateIdentity, error)) {
        return false;
      }
      targetMaterials.swap_state(candidateMaterials);
      framework_units().swap_project_state(candidateUnits);
    }
    return true;
  }
  catch (const H5::Exception& exception) {
    error = exception.getDetailMsg();
  }
  catch (const std::exception& exception) {
    error = exception.what();
  }
  catch (...) {
    error = "Cannot read or write framework project metadata.";
  }
  return false;
}
}

ProjectStateCoordinator::ProjectStateCoordinator(std::vector<ProjectStateProviderInterface*> active) : providers(std::move(active)) {
  for (auto* provider : providers) {
    provider->set_project_coordinated(true);
  }
  register_observer();
}
ProjectStateCoordinator::~ProjectStateCoordinator() {
  unregister_observer();
  discard();
  for (auto* provider : providers) {
    provider->set_project_coordinated(false);
  }
}
void ProjectStateCoordinator::discard() noexcept
{
  for (auto* provider : providers) {
    provider->discard_project_read();
  }
}
void ProjectStateCoordinator::notify_model_reset() {
  if (!updating) {
    discard();
    framework_materials().begin_project();
    framework_units().reset_project();
  }
}
void ProjectStateCoordinator::notify_cub_file_read(std::string filename) {
  if (updating) {
    PRINT_ERROR("Cannot restore a second project during project restoration.\n");
    return;
  }
  if (generation == std::numeric_limits<std::uint64_t>::max()) {
    PRINT_ERROR("Project restore generation identifiers are exhausted.\n");
    return;
  }
  QScopedValueRollback<bool> guard(updating,true);
  const ProjectStateContext context{QFileInfo(QString::fromUtf8(filename.c_str())).absoluteFilePath().toUtf8().toStdString(),++generation};
  auto previousUnits = framework_units();
  auto previousMaterials = framework_materials().workspace("calculix");
  const std::string previousIdentity = framework_materials().project_id();
  bool restored = false;
  const auto rollbackMetadata = qScopeGuard([&]() {
    if (!restored) {
      framework_units().swap_project_state(previousUnits);
      framework_materials().workspace("calculix").swap_state(previousMaterials);
      std::string identityError;
      if (!framework_materials().set_project_id(previousIdentity, identityError)) {
        PRINT_ERROR("Cannot restore material project identity: %s\n", identityError.c_str());
      }
    }
  });
  try {
    std::string error;
    bool metadataPresent = false;
    if (!projectMetadata(context.filename, false, metadataPresent, error)) {
      PRINT_ERROR("[Framework] Cannot load project metadata: %s\n", error.c_str());
      discard();
      return;
    }
    using Phase = bool (ProjectStateProviderInterface::*)(const ProjectStateContext&,std::string&);
    for (Phase phase : {&ProjectStateProviderInterface::prepare_project_read, &ProjectStateProviderInterface::restore_project_model, &ProjectStateProviderInterface::resolve_project_sources, &ProjectStateProviderInterface::commit_project_read}) {
      for (auto* provider : providers) {
        error.clear();
        if (!(provider->*phase)(context,error)) {
          PRINT_ERROR("Cannot restore %s project data: %s\n",provider->project_state_id().c_str(),error.c_str());
          discard();
          return;
        }
        if (phase == &ProjectStateProviderInterface::commit_project_read) {
          PRINT_INFO("[%s] Loaded project state.\n", workspaceLabel(provider).c_str());
        }
      }
    }
    restored = true;
    if (metadataPresent) {
      PRINT_INFO("[Framework] Loaded project metadata.\n");
    } else {
      PRINT_INFO("[Framework] No saved project metadata.\n");
    }
  }
  catch (const std::exception& exception) {
    PRINT_ERROR("Project restore failed: %s\n",exception.what());
  }
  catch (...) {
    PRINT_ERROR("Project restore failed with an unknown exception.\n");
  }
  discard();
}
void ProjectStateCoordinator::notify_cub_file_saved(std::string filename, bool temporary) {
  if (temporary || updating) {
    return;
  }
  QScopedValueRollback<bool> guard(updating,true);
  const ProjectStateContext context{QFileInfo(QString::fromUtf8(filename.c_str())).absoluteFilePath().toUtf8().toStdString(),generation};
  try {
    std::string error;
    // Capture definitions before any participant writes its HDF5 group.
    for (auto* provider : providers) {
      error.clear();
      if (!provider->prepare_project_save(context,error)) {
        PRINT_ERROR("Cubit saved the project, but %s state could not be prepared: %s\n", provider->project_state_id().c_str(), error.c_str());
        discard();
        return;
      }
    }
    bool allSaved = true;
    for (auto* provider : providers) {
      error.clear();
      if (!provider->write_project_state(context,error)) {
        allSaved = false;
        PRINT_ERROR("Cubit saved the project, but %s state was not fully saved: %s\n", provider->project_state_id().c_str(), error.c_str());
      } else {
        PRINT_INFO("[%s] Saved project state.\n", workspaceLabel(provider).c_str());
      }
    }
    bool metadataPresent = false;
    if (!projectMetadata(context.filename, true, metadataPresent, error)) {
      PRINT_ERROR("[Framework] Cubit saved the file, but project metadata was not saved: %s\n", error.c_str());
    } else if (allSaved) {
      PRINT_INFO("[Framework] Saved project metadata.\n");
    } else {
      PRINT_WARNING("[Framework] Saved project metadata; some workspace data failed to save.\n");
    }
  }
  catch (const std::exception& exception) {
    PRINT_ERROR("Cubit saved the project, but workspace persistence failed: %s\n",exception.what());
  }
  catch (...) {
    PRINT_ERROR("Cubit saved the project, but workspace persistence failed.\n");
  }
  discard();
}
