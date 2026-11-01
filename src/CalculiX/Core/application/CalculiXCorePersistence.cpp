#include "Core/CalculiXCore.hpp"
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

namespace
{
void advance_progress(ProgressTool& progress)
{
  progress.step();
  progress.check_interrupt();
}
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

  log = "Reading Cubit-CalculiX data from \"" + filename + "\"\n";
  PRINT_INFO("%s", log.c_str());
  
  if (H5Fis_accessible(filename.c_str(), H5P_DEFAULT) <= 0)
  {
    PRINT_ERROR(
      "Cannot read Cubit-CalculiX data: \"%s\" is not accessible as an HDF5 file.\n",
      filename.c_str());
    return false;
  }

  HDF5Tool cubTool(filename);
 
  if (!cubTool.nameExists("Cubit-CalculiX"))
  {
    log = "No Cubit-CalculiX data in \"" + filename + "\"\n";
    PRINT_INFO("%s", log.c_str());
    return true;
  }

  progressbar->start(0, 29, "Reading Cubit-CalculiX data");
  progressbar->check_interrupt();
  //General
  CoreModelMetadataPersistence::read(cubTool);
  //Blocks
  CoreBlocksPersistence::read(cubTool, *cb);
  advance_progress(*progressbar);
  //Sections
  CoreSectionsPersistence::read(cubTool, *sections);
  advance_progress(*progressbar);
  //Constraints
  CoreInteractionsPersistence::read(cubTool, *constraints);
  advance_progress(*progressbar);
  //SurfaceInteractions
  CoreInteractionsPersistence::read(cubTool, *surfaceinteractions);
  advance_progress(*progressbar);
  //ContactPairs
  CoreInteractionsPersistence::read(cubTool, *contactpairs);
  advance_progress(*progressbar);
  //Amplitudes
  CoreAmplitudesPersistence::read(cubTool, *amplitudes);
  advance_progress(*progressbar);
  //Orientations
  CoreOrientationsPersistence::read(cubTool, *orientations);
  advance_progress(*progressbar);
  //Damping
  CoreStepSettingsPersistence::read(cubTool, *damping);
  advance_progress(*progressbar);
  //Physical Constants
  CoreStepSettingsPersistence::read(cubTool, *physicalconstants);
  advance_progress(*progressbar);
  //LoadsForces
  CoreLoadsPersistence::read(cubTool, *loadsforces);
  advance_progress(*progressbar);
  //LoadsPressures
  CoreLoadsPersistence::read(cubTool, *loadspressures);
  advance_progress(*progressbar);
  //LoadsHeatFluxes
  CoreLoadsPersistence::read(cubTool, *loadsheatfluxes);
  advance_progress(*progressbar);
  //LoadsGravity
  CoreLoadsPersistence::read(cubTool, *loadsgravity);
  advance_progress(*progressbar);
  //LoadsCentrifugal
  CoreLoadsPersistence::read(cubTool, *loadscentrifugal);
  advance_progress(*progressbar);
  //LoadsTrajectory
  CoreLoadsPersistence::read(cubTool, *loadstrajectory);
  advance_progress(*progressbar);
  //LoadsFilm
  CoreLoadsPersistence::read(cubTool, *loadsfilm);
  advance_progress(*progressbar);
  //LoadsRadiation
  CoreLoadsPersistence::read(cubTool, *loadsradiation);
  advance_progress(*progressbar);
  //LoadsSurfaceTraction
  CoreLoadsPersistence::read(cubTool, *loadssurfacetraction);
  advance_progress(*progressbar);
  //LoadsBodyHeatflux
  CoreLoadsPersistence::read(cubTool, *loadsbodyheatflux);
  advance_progress(*progressbar);
  //BCs
  //BCsDisplacements
  CoreBCsPersistence::read(cubTool, *bcsdisplacements);
  advance_progress(*progressbar);
  //BCsTemperatures
  CoreBCsPersistence::read(cubTool, *bcstemperatures);
  advance_progress(*progressbar);
  //HistoryOutputs
  CoreOutputsPersistence::read(cubTool, *historyoutputs);
  advance_progress(*progressbar);
  //FieldOutputs
  CoreOutputsPersistence::read(cubTool, *fieldoutputs);
  advance_progress(*progressbar);
  //InitialConditions
  CoreInitialConditionsPersistence::read(cubTool, *initialconditions);
  advance_progress(*progressbar);
  //HBCs
  CoreBCsPersistence::read(cubTool, *hbcs);
  advance_progress(*progressbar);
  //Steps
  CoreStepsPersistence::read(cubTool, *steps);
  advance_progress(*progressbar);
  //Jobs
  CoreJobsPersistence::read(cubTool, *jobs);
  advance_progress(*progressbar);
  //Results
  CoreResultsPersistence::read_registry(cubTool, *results);
  advance_progress(*progressbar);
  if (!CoreResultsPersistence::read_loaded_results(cubTool, *results))
  {
    progressbar->end();
    return false;
  }
  advance_progress(*progressbar);
  //CustomLines
  CoreCustomLinesPersistence::read(cubTool, *customlines);
  advance_progress(*progressbar);
  if (!CoreTopologyPersistence::read(cubTool, *topology, *mat, *sections, *jobs))
  {
    progressbar->end();
    return false;
  }
  progressbar->end();
  log = "Finished reading Cubit-CalculiX data from \"" + filename + "\"\n";
  PRINT_INFO("%s", log.c_str());

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

  return true;
}

bool CalculiXCore::save_cub(std::string filename)
{
  std::string log = "Saving Cubit-CaluliX data to \"" + filename + "\"\n";
  PRINT_INFO("%s", log.c_str());
  ProgressTool* progressbar = CubitInterface::app_util().get()->progress_tool();

  if (H5Fis_accessible(filename.c_str(), H5P_DEFAULT) <= 0)
  {
    PRINT_ERROR(
      "Cannot save Cubit-CalculiX data: \"%s\" is not accessible as an HDF5 file.\n",
      filename.c_str());
    return false;
  }

  HDF5Tool cubTool(filename);

  if (cubTool.nameExists("Cubit-CalculiX"))
  {
    log = "group already here. Can't write Cubit-CalculiX data into \"" + filename + "\"\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  progressbar->start(0, 29, "Writing Cubit-CalculiX data");
  progressbar->check_interrupt();
  //General
  CoreModelMetadataPersistence::write(cubTool, version);
  //Blocks
  CoreBlocksPersistence::write(cubTool, *cb);
  advance_progress(*progressbar);
  //Materials
  cubTool.createGroup("Cubit-CalculiX/Materials");
  advance_progress(*progressbar);
  //Sections
  CoreSectionsPersistence::write(cubTool, *sections);
  advance_progress(*progressbar);
  //Constraints
  CoreInteractionsPersistence::write(cubTool, *constraints);
  advance_progress(*progressbar);
  //SurfaceInteractions
  CoreInteractionsPersistence::write(cubTool, *surfaceinteractions);
  advance_progress(*progressbar);
  //ContactPairs
  CoreInteractionsPersistence::write(cubTool, *contactpairs);
  advance_progress(*progressbar);
  //Amplitudes
  CoreAmplitudesPersistence::write(cubTool, *amplitudes);
  advance_progress(*progressbar);
  //Orientations
  CoreOrientationsPersistence::write(cubTool, *orientations);
  advance_progress(*progressbar);
  //Damping
  CoreStepSettingsPersistence::write(cubTool, *damping);
  advance_progress(*progressbar);
  //Physical Constants
  CoreStepSettingsPersistence::write(cubTool, *physicalconstants);
  advance_progress(*progressbar);
  //Loads
  cubTool.createGroup("Cubit-CalculiX/Loads");
  //LoadsForces
  CoreLoadsPersistence::write(cubTool, *loadsforces);
  advance_progress(*progressbar);
  //LoadsPressures
  CoreLoadsPersistence::write(cubTool, *loadspressures);
  advance_progress(*progressbar);
  //LoadsHeatFluxes
  CoreLoadsPersistence::write(cubTool, *loadsheatfluxes);
  advance_progress(*progressbar);
  //LoadsGravity
  CoreLoadsPersistence::write(cubTool, *loadsgravity);
  advance_progress(*progressbar);
  //LoadsCentrifugal
  CoreLoadsPersistence::write(cubTool, *loadscentrifugal);
  advance_progress(*progressbar);
  //LoadsTrajectory
  CoreLoadsPersistence::write(cubTool, *loadstrajectory);
  advance_progress(*progressbar);
  //LoadsFilm
  CoreLoadsPersistence::write(cubTool, *loadsfilm);
  advance_progress(*progressbar);
  //LoadsRadiation
  CoreLoadsPersistence::write(cubTool, *loadsradiation);
  advance_progress(*progressbar);
  //LoadsSurfaceTraction
  CoreLoadsPersistence::write(cubTool, *loadssurfacetraction);
  advance_progress(*progressbar);
  //LoadsBodyHeatflux
  CoreLoadsPersistence::write(cubTool, *loadsbodyheatflux);
  advance_progress(*progressbar);
  //BCs
  cubTool.createGroup("Cubit-CalculiX/BCs");
  //BCsDisplacements
  CoreBCsPersistence::write(cubTool, *bcsdisplacements);
  advance_progress(*progressbar);
  //BCsTemperatures
  CoreBCsPersistence::write(cubTool, *bcstemperatures);
  advance_progress(*progressbar);
  //HistoryOutputs
  CoreOutputsPersistence::write(cubTool, *historyoutputs);
  advance_progress(*progressbar);
  //FieldOutputs
  CoreOutputsPersistence::write(cubTool, *fieldoutputs);
  advance_progress(*progressbar);
  //InitialConditions
  CoreInitialConditionsPersistence::write(cubTool, *initialconditions);
  advance_progress(*progressbar);
  //HBCs
  CoreBCsPersistence::write(cubTool, *hbcs);
  advance_progress(*progressbar);
  //Steps
  CoreStepsPersistence::write(cubTool, *steps);
  advance_progress(*progressbar);
  //Jobs
  CoreJobsPersistence::write(cubTool, *jobs);
  advance_progress(*progressbar);
  //Results
  CoreResultsPersistence::write(cubTool, *results, ccx_uo.mSaveLoadedResults);
  advance_progress(*progressbar);
  //CustomLines
  CoreCustomLinesPersistence::write(cubTool, *customlines);
  advance_progress(*progressbar);
  if (!CoreTopologyPersistence::write(cubTool, *topology, *mat, *sections))
  {
    progressbar->end();
    return false;
  }
  progressbar->end();

  log = "Finished saving Cubit-CalculiX data.\n";
  PRINT_INFO("%s", log.c_str());

  return true;
}

