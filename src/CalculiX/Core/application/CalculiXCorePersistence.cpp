#include "Core/application/CalculiXModelSchema.hpp"
#include "Core/CalculiXCore.hpp"
#include "Core/drawing/CoreDraw.hpp"
#include "Core/drawing/CoreDrawPersistence.hpp"
#include "CoreModelMetadataPersistence.hpp"
#include "Core/mesh/CoreBlocksPersistence.hpp"
#include "Core/sections/CoreSectionsPersistence.hpp"
#include "Core/sections/CoreOrientationsPersistence.hpp"
#include "Core/interactions/CoreInteractionsPersistence.hpp"
#include "Core/amplitudes/CoreAmplitudesPersistence.hpp"
#include "Core/steps/CoreStepSettingsPersistence.hpp"
#include "Core/loads/CoreLoadsPersistence.hpp"
#include "Core/bcs/CoreBCsPersistence.hpp"
#include "Core/steps/CoreOutputsPersistence.hpp"
#include "Core/steps/CoreInitialConditionsPersistence.hpp"
#include "Core/steps/CoreStepsPersistence.hpp"
#include "Core/jobs/CoreJobsPersistence.hpp"
#include "Core/results/CoreResultsPersistence.hpp"
#include "Core/export/CoreCustomLinesPersistence.hpp"
#include "Core/topology/CoreTopologyPersistence.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"
#include "AppUtil.hpp"
#include "HDF5Tool.hpp"
#include "loadUserOptions.hpp"

#include <hdf5.h>
#include <QScopedValueRollback>
#include <exception>
#include <memory>

namespace
{
std::unique_ptr<HDF5Tool> open_model_file(const std::string& filename, bool& hasCalculiXData)
{
  try
  {
    if (H5Fis_accessible(filename.c_str(), H5P_DEFAULT) <= 0)
    {
      PRINT_ERROR("Cannot access CalculiX model file as HDF5: %s\n", filename.c_str());
      return nullptr;
    }
    auto file = std::make_unique<HDF5Tool>(filename, false);
    hasCalculiXData = file->nameExists(CalculiXModelSchema::path(""));
    return file;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot open CalculiX model file %s: %s\n", filename.c_str(), error.getDetailMsg().c_str());
    return nullptr;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot open CalculiX model file %s: %s\n", filename.c_str(), error.what());
    return nullptr;
  }
}

class PersistenceProgress
{
public:
  explicit PersistenceProgress(ProgressTool* progress) : progress(progress)
  {
  }

  ~PersistenceProgress() noexcept
  {
    finish();
  }

  bool start(int steps, const char* title)
  {
    try
    {
      if (progress != nullptr)
      {
        started = true;
        progress->start(0, steps, title);
        progress->check_interrupt();
      }
      return true;
    }
    catch (const std::exception& error)
    {
      PRINT_WARNING("CalculiX persistence progress interrupted: %s\n", error.what());
      return false;
    }
    catch (...)
    {
      PRINT_WARNING("CalculiX persistence progress interrupted.\n");
      return false;
    }
  }

  bool advance()
  {
    try
    {
      if (started && progress != nullptr)
      {
        progress->step();
        progress->check_interrupt();
      }
      return true;
    }
    catch (const std::exception& error)
    {
      PRINT_WARNING("CalculiX persistence progress interrupted: %s\n", error.what());
      return false;
    }
    catch (...)
    {
      PRINT_WARNING("CalculiX persistence progress interrupted.\n");
      return false;
    }
  }

  void finish() noexcept
  {
    if (!started || progress == nullptr)
    {
      return;
    }
    started = false;
    try
    {
      progress->end();
    }
    catch (...)
    {
      PRINT_WARNING("Cannot finish CalculiX persistence progress display.\n");
    }
  }

private:
  ProgressTool* progress;
  bool started = false;
};
}

bool CalculiXCore::read_cub(std::string filename)
{
  cancel_topology_animation();
  if (checking_jobs)
  {
    PRINT_ERROR("Cannot read model data while job results are being processed.\n");
    return false;
  }
  const QScopedValueRollback<bool> guard(checking_jobs, true);
  if (!cancel_topology_runs())
  {
    return false;
  }
  std::string log = "";
  ProgressTool* progressbar = CubitInterface::app_util().get()->progress_tool();

  
  bool hasCalculiXData = false;
  auto opened = open_model_file(filename, hasCalculiXData);
  if (opened == nullptr)
  {
    return false;
  }
  auto& cubTool = *opened;
 
  if (!hasCalculiXData)
  {
    log = "[CalculiX] No saved model data in \"" + filename + "\"\n";
    PRINT_INFO("%s", log.c_str());
    if (!CoreDrawPersistence::read(cubTool, *draw))
    {
      return false;
    }
    return cubTool.close();
  }

  PersistenceProgress progress(progressbar);
  if (!progress.start(32, "Reading CalculiX data"))
  {
    return false;
  }
  //General
  if (!CoreModelMetadataPersistence::read(cubTool))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Blocks
  if (!CoreBlocksPersistence::read(cubTool, *cb))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Sections
  if (!CoreSectionsPersistence::read(cubTool, *sections))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Constraints
  if (!CoreInteractionsPersistence::read(cubTool, *constraints))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //SurfaceInteractions
  if (!CoreInteractionsPersistence::read(cubTool, *surfaceinteractions))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //ContactPairs
  if (!CoreInteractionsPersistence::read(cubTool, *contactpairs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Amplitudes
  if (!CoreAmplitudesPersistence::read(cubTool, *amplitudes))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Orientations
  if (!CoreOrientationsPersistence::read(cubTool, *orientations))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Damping
  if (!CoreStepSettingsPersistence::read(cubTool, *damping))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Physical Constants
  if (!CoreStepSettingsPersistence::read(cubTool, *physicalconstants))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsForces
  if (!CoreLoadsPersistence::read(cubTool, *loadsforces))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsPressures
  if (!CoreLoadsPersistence::read(cubTool, *loadspressures))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsHeatFluxes
  if (!CoreLoadsPersistence::read(cubTool, *loadsheatfluxes))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsGravity
  if (!CoreLoadsPersistence::read(cubTool, *loadsgravity))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsCentrifugal
  if (!CoreLoadsPersistence::read(cubTool, *loadscentrifugal))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsTrajectory
  if (!CoreLoadsPersistence::read(cubTool, *loadstrajectory))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsFilm
  if (!CoreLoadsPersistence::read(cubTool, *loadsfilm))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsRadiation
  if (!CoreLoadsPersistence::read(cubTool, *loadsradiation))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsSurfaceTraction
  if (!CoreLoadsPersistence::read(cubTool, *loadssurfacetraction))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsBodyHeatflux
  if (!CoreLoadsPersistence::read(cubTool, *loadsbodyheatflux))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //BCs
  //BCsDisplacements
  if (!CoreBCsPersistence::read(cubTool, *bcsdisplacements))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //BCsTemperatures
  if (!CoreBCsPersistence::read(cubTool, *bcstemperatures))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //HistoryOutputs
  if (!CoreOutputsPersistence::read(cubTool, *historyoutputs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //FieldOutputs
  if (!CoreOutputsPersistence::read(cubTool, *fieldoutputs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //InitialConditions
  if (!CoreInitialConditionsPersistence::read(cubTool, *initialconditions))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //HBCs
  if (!CoreBCsPersistence::read(cubTool, *hbcs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Steps
  if (!CoreStepsPersistence::read(cubTool, *steps))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Jobs
  if (!CoreJobsPersistence::read(cubTool, *jobs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Results
  if (!CoreResultsPersistence::read(cubTool, *results))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //CustomLines
  if (!CoreCustomLinesPersistence::read(cubTool, *customlines))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  if (!CoreTopologyPersistence::read(cubTool, *topology, *mat, *sections, *jobs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  // check data with autocleanup
  if (use_ccx_autocleanup)
  {
    log = autocleanup();
    if (log != "")
    {
      log.append("Something seems to be wrong with the model or save file import!!! \nNormally CCX Autocleanup shouldn't delete anything after loading from a save file.\n");
      PRINT_INFO("%s", log.c_str());
    }
  }

  if (!CoreDrawPersistence::read(cubTool, *draw))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  progress.finish();
  if (!cubTool.close())
  {
    return false;
  }

  return true;
}

bool CalculiXCore::save_cub(std::string filename)
{
  std::string log;
  ProgressTool* progressbar = CubitInterface::app_util().get()->progress_tool();

  bool hasCalculiXData = false;
  auto opened = open_model_file(filename, hasCalculiXData);
  if (opened == nullptr)
  {
    return false;
  }
  auto& cubTool = *opened;

  if (hasCalculiXData)
  {
    log = "group already here. Can't write cubit-sim data into \"" + filename + "\"\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  PersistenceProgress progress(progressbar);
  if (!progress.start(32, "Writing CalculiX data"))
  {
    return false;
  }
  //General
  if (!CoreModelMetadataPersistence::write(cubTool, version))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Blocks
  if (!CoreBlocksPersistence::write(cubTool, *cb))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Sections
  if (!CoreSectionsPersistence::write(cubTool, *sections))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Constraints
  if (!CoreInteractionsPersistence::write(cubTool, *constraints))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //SurfaceInteractions
  if (!CoreInteractionsPersistence::write(cubTool, *surfaceinteractions))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //ContactPairs
  if (!CoreInteractionsPersistence::write(cubTool, *contactpairs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Amplitudes
  if (!CoreAmplitudesPersistence::write(cubTool, *amplitudes))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Orientations
  if (!CoreOrientationsPersistence::write(cubTool, *orientations))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Damping
  if (!CoreStepSettingsPersistence::write(cubTool, *damping))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Physical Constants
  if (!CoreStepSettingsPersistence::write(cubTool, *physicalconstants))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Loads
  //LoadsForces
  if (!CoreLoadsPersistence::write(cubTool, *loadsforces))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsPressures
  if (!CoreLoadsPersistence::write(cubTool, *loadspressures))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsHeatFluxes
  if (!CoreLoadsPersistence::write(cubTool, *loadsheatfluxes))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsGravity
  if (!CoreLoadsPersistence::write(cubTool, *loadsgravity))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsCentrifugal
  if (!CoreLoadsPersistence::write(cubTool, *loadscentrifugal))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsTrajectory
  if (!CoreLoadsPersistence::write(cubTool, *loadstrajectory))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsFilm
  if (!CoreLoadsPersistence::write(cubTool, *loadsfilm))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsRadiation
  if (!CoreLoadsPersistence::write(cubTool, *loadsradiation))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsSurfaceTraction
  if (!CoreLoadsPersistence::write(cubTool, *loadssurfacetraction))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //LoadsBodyHeatflux
  if (!CoreLoadsPersistence::write(cubTool, *loadsbodyheatflux))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //BCs
  //BCsDisplacements
  if (!CoreBCsPersistence::write(cubTool, *bcsdisplacements))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //BCsTemperatures
  if (!CoreBCsPersistence::write(cubTool, *bcstemperatures))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //HistoryOutputs
  if (!CoreOutputsPersistence::write(cubTool, *historyoutputs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //FieldOutputs
  if (!CoreOutputsPersistence::write(cubTool, *fieldoutputs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //InitialConditions
  if (!CoreInitialConditionsPersistence::write(cubTool, *initialconditions))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //HBCs
  if (!CoreBCsPersistence::write(cubTool, *hbcs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Steps
  if (!CoreStepsPersistence::write(cubTool, *steps))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Jobs
  if (!CoreJobsPersistence::write(cubTool, *jobs))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //Results
  if (!CoreResultsPersistence::write(cubTool, *results, ccx_uo.mSaveLoadedResults))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  //CustomLines
  if (!CoreCustomLinesPersistence::write(cubTool, *customlines))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  if (!CoreTopologyPersistence::write(cubTool, *topology, *mat, *sections))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }
  if (!CoreDrawPersistence::write(cubTool, *draw))
  {
    return false;
  }
  if (!progress.advance())
  {
    return false;
  }

  progress.finish();
  if (!cubTool.close())
  {
    return false;
  }

  return true;
}
