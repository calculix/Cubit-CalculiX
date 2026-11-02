#include "Core/drawing/CoreDraw.hpp"
#include "Core/CalculiXCore.hpp"
#include "Core/sections/CoreSectionsCleanup.hpp"
#include "Core/interactions/CoreConstraintsCleanup.hpp"
#include "Core/interactions/CoreContactPairsCleanup.hpp"
#include "Core/loads/CoreLoadsCleanup.hpp"
#include "Core/loads/CoreLoadsQuery.hpp"
#include "Core/bcs/CoreBCsCleanup.hpp"
#include "Core/steps/CoreInitialConditionsCleanup.hpp"
#include "Core/steps/CoreOutputsCleanup.hpp"
#include "Core/steps/CoreStepsCleanup.hpp"
#include "CoreTimer.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/materials/CoreMaterialsLibrary.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/topology/CoreTopology.hpp"
#include "Core/results/CoreResults.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "CubitMessage.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"

bool CalculiXCore::update()
{
  cb->update();
  loadsforces->update();
  loadspressures->update();
  loadsheatfluxes->update();
  bcsdisplacements->update();
  bcstemperatures->update();

  if (use_ccx_autocleanup)
  {
    const std::string cleanup_log = autocleanup();

    if (!cleanup_log.empty())
    {
      print_to_log(cleanup_log);
    }
  }

  if (use_ccx_logfile)
  {
    print_to_log("UPDATE");
    print_to_log(print_data());
  }

  return true;
}

bool CalculiXCore::reset()
{
  if (draw != nullptr)
  {
    draw->resetDrawing();
  }

  cancel_topology_animation();

  if (checking_jobs)
  {
    PRINT_ERROR("Cannot reset CalculiX data while job results are being processed.\n");
    return false;
  }

  if (timer)
  {
    timer->stop();
  }

  if (!cancel_topology_runs())
  {
    if (timer)
    {
      timer->start(1000);
    }

    return false;
  }

  cb->reset();
  mat->reset(); // Reset topology material data.
  mat_library->reset();
  sections->reset();
  constraints->reset();
  surfaceinteractions->reset();
  contactpairs->reset();
  amplitudes->reset();
  orientations->reset();
  damping->reset();
  physicalconstants->reset();
  loadsforces->reset();
  loadspressures->reset();
  loadsheatfluxes->reset();
  loadsgravity->reset();
  loadscentrifugal->reset();
  loadstrajectory->reset();
  loadsfilm->reset();
  loadsradiation->reset();
  loadssurfacetraction->reset();
  loadsbodyheatflux->reset();
  bcsdisplacements->reset();
  bcstemperatures->reset();
  historyoutputs->reset();
  fieldoutputs->reset();
  initialconditions->reset();
  hbcs->reset();
  steps->reset();
  jobs->reset();
  topology->reset();
  results->reset();
  customlines->reset();

  sideset_faces.clear();
  PRINT_INFO("Reset Cubit-CalculiX\n");

  if (timer)
  {
    timer->start(1000);
  }

  return true;
}

std::string CalculiXCore::print_data()
{
  std::string data;
  data.append(cb->print_data());
  data.append(sections->print_data());
  data.append(constraints->print_data());
  data.append(surfaceinteractions->print_data());
  data.append(contactpairs->print_data());
  data.append(amplitudes->print_data());
  data.append(orientations->print_data());
  data.append(damping->print_data());
  data.append(physicalconstants->print_data());
  data.append(loadsforces->print_data());
  data.append(loadspressures->print_data());
  data.append(loadsheatfluxes->print_data());
  data.append(loadsgravity->print_data());
  data.append(loadscentrifugal->print_data());
  data.append(loadstrajectory->print_data());
  data.append(loadsfilm->print_data());
  data.append(loadsradiation->print_data());
  data.append(loadssurfacetraction->print_data());
  data.append(loadsbodyheatflux->print_data());
  data.append(bcsdisplacements->print_data());
  data.append(bcstemperatures->print_data());
  data.append(historyoutputs->print_data());
  data.append(fieldoutputs->print_data());
  data.append(initialconditions->print_data());
  data.append(hbcs->print_data());
  data.append(steps->print_data());
  data.append(jobs->print_data());
  data.append(results->print_data());
  data.append(customlines->print_data());

  return data;
}

std::string CalculiXCore::autocleanup()
{
  std::string log = "CCX Autocleanup\n";
  bool print_log = false;

  // SECTIONS
  if (CoreSectionsCleanup::cleanup(*sections, *mat, *cb, orientations, log))
  {
    print_log = true;
  }

  // CONSTRAINTS
  if (CoreConstraintsCleanup::cleanup(*constraints, *cb, *me_iface, log))
  {
    print_log = true;
  }

  // CONTACT PAIRS
  if (CoreContactPairsCleanup::cleanup(*contactpairs, *surfaceinteractions, *me_iface, log))
  {
    print_log = true;
  }

  const CoreLoadsCleanupData load_cleanup =
  {
    *loadsforces,
    *loadspressures,
    *loadsheatfluxes,
    *loadsgravity,
    *loadscentrifugal,
    *loadstrajectory,
    *loadsfilm,
    *loadsradiation,
    *loadssurfacetraction,
    *loadsbodyheatflux,
    *amplitudes,
    *steps,
    *cb,
    *me_iface
  };

  // LOADS
  if (CoreLoadsCleanup::cleanup(load_cleanup, log))
  {
    print_log = true;
  }

  const CoreBCsCleanupData bc_cleanup = {
    *amplitudes, *steps, *hbcs, *initialconditions, *me_iface
  };

  // BCS DISPLACEMENTS
  if (CoreBCsCleanup::cleanup(*bcsdisplacements, bc_cleanup, log))
  {
    print_log = true;
  }

  // BCS TEMPERATURES
  if (CoreBCsCleanup::cleanup(*bcstemperatures, bc_cleanup, log))
  {
    print_log = true;
  }

  // INITIAL CONDITIONS
  if (CoreInitialConditionsCleanup::cleanup(*initialconditions, *bcsdisplacements, *bcstemperatures, log))
  {
    print_log = true;
  }

  // HISTORY OUTPUTS
  if (CoreOutputsCleanup::cleanup(*historyoutputs, *cb, *contactpairs, *me_iface, log))
  {
    print_log = true;
  }

  // FIELD OUTPUTS
  if (CoreOutputsCleanup::cleanup(*fieldoutputs, *cb, *contactpairs, *me_iface, log))
  {
    print_log = true;
  }

  // HBCS
  if (CoreBCsCleanup::cleanup(*hbcs, *bcsdisplacements, *bcstemperatures, log))
  {
    print_log = true;
  }

  // STEPS
  const auto load_data = get_load_query_data();

  const auto load_exists = [load_data](int load_type, int load_id)
  {
    return CoreLoadsQuery::load_exists(load_data, load_type, load_id);
  };

  if (CoreStepsCleanup::cleanup(*steps, *bcsdisplacements, *bcstemperatures, *historyoutputs, *fieldoutputs, load_exists, log))
  {
    print_log = true;
  }
  
  if (print_log)
  {
    PRINT_INFO("%s", log.c_str());
    return log;
  }
  return "";
}
