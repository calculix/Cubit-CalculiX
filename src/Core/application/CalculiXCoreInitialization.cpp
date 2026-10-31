#include "Core/CalculiXCore.hpp"
#include "CorePythonInterfaceInitialization.hpp"
#include "CoreFeatureInitialization.hpp"
#include "CoreTimer.hpp"
#include "Core/mesh/CoreBlocks.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/topology/CoreTopology.hpp"
#include "Core/topology/CoreTopologyOptimization.hpp"
#include "Core/results/CoreResults.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/drawing/CoreDraw.hpp"
#include "Core/materials/CoreMaterialsLibrary.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "MeshExportInterface.hpp"
#include "MaterialInterface.hpp"
#include "CubitInterface.hpp"
#include "loadUserOptions.hpp"

bool CalculiXCore::init_pythoninterface()
{
  if (bool_init_pythoninterface)
  {
    return false;
  }

  bool_init_pythoninterface = CorePythonInterfaceInitialization::initialize(CubitInterface::python_interpreter(), ccx_uo.mPathPythonInterface.toStdString());
  return bool_init_pythoninterface;
}

bool CalculiXCore::init_materiallibrary()
{
  // The GUI timer initializes the library after Cubit has fully loaded.
  if (!bool_init_materiallibrary)
  {
    bool_init_materiallibrary = mat_library->init();
    return bool_init_materiallibrary;
  }

  return false;
}

bool CalculiXCore::init2()
{
  me_iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));

  if (me_iface == nullptr)
  {
    return false;
  }

  me_iface->initialize_export();

  mat_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));

  if (mat_iface == nullptr)
  {
    return false;
  }

  if (!mat)
  {
    mat = new CoreMaterials;
  }

  mat->init();

  if (!mat_library)
  {
    mat_library = new CoreMaterialsLibrary;
  }

  bool_init2 = true;
  return true;
}

bool CalculiXCore::init()
{
  CoreFeatureInitialization::initialize(cb);
  CoreFeatureInitialization::initialize(sections);
  CoreFeatureInitialization::initialize(constraints);
  CoreFeatureInitialization::initialize(surfaceinteractions);
  CoreFeatureInitialization::initialize(contactpairs);
  CoreFeatureInitialization::initialize(amplitudes);
  CoreFeatureInitialization::initialize(orientations);
  CoreFeatureInitialization::initialize(damping);
  CoreFeatureInitialization::initialize(physicalconstants);
  CoreFeatureInitialization::initialize(loadsforces);
  CoreFeatureInitialization::initialize(loadspressures);
  CoreFeatureInitialization::initialize(loadsheatfluxes);
  CoreFeatureInitialization::initialize(loadsgravity);
  CoreFeatureInitialization::initialize(loadscentrifugal);
  CoreFeatureInitialization::initialize(loadstrajectory);
  CoreFeatureInitialization::initialize(loadsfilm);
  CoreFeatureInitialization::initialize(loadsradiation);
  CoreFeatureInitialization::initialize(loadssurfacetraction);
  CoreFeatureInitialization::initialize(loadsbodyheatflux);
  CoreFeatureInitialization::initialize(bcsdisplacements);
  CoreFeatureInitialization::initialize(bcstemperatures);
  CoreFeatureInitialization::initialize(historyoutputs);
  CoreFeatureInitialization::initialize(fieldoutputs);
  CoreFeatureInitialization::initialize(initialconditions);
  CoreFeatureInitialization::initialize(hbcs);
  CoreFeatureInitialization::initialize(steps);
  CoreFeatureInitialization::initialize(jobs);
  CoreFeatureInitialization::initialize(topology);
  CoreFeatureInitialization::initialize(results);
  CoreFeatureInitialization::initialize(customlines);
  CoreFeatureInitialization::initialize(draw);

  if (!timer)
  {
    timer = new CoreTimer;
  }

  if (!topologyoptimization)
  {
    topologyoptimization = new CoreTopologyOptimization;
  }

  if (use_ccx_logfile)
  {
    print_to_log("CalculiXCore Initialization!");
  }

  bool_init = true;
  bool_init_pythoninterface = false;
  return true;
}

