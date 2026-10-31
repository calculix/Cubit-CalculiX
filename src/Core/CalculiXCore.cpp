#include "CalculiXCore.hpp"

#include "Core/mesh/CoreBlocks.hpp"
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
#include "Core/application/CoreTimer.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/drawing/CoreDraw.hpp"

CalculiXCore::CalculiXCore() = default;

CalculiXCore::~CalculiXCore()
{
  if (draw)
  {
    draw->shutdown_topology_animation();
  }

  if (timer)
  {
    timer->stop();
  }

  cancel_topology_runs();

  delete topologyoptimization;
  topologyoptimization = nullptr;

  delete cb;
  delete mat;
  delete mat_library;
  delete sections;
  delete constraints;
  delete surfaceinteractions;
  delete contactpairs;
  delete amplitudes;
  delete orientations;
  delete damping;
  delete physicalconstants;
  delete loadsforces;
  delete loadspressures;
  delete loadsheatfluxes;
  delete loadsgravity;
  delete loadscentrifugal;
  delete loadstrajectory;
  delete loadsfilm;
  delete loadsradiation;
  delete loadssurfacetraction;
  delete loadsbodyheatflux;
  delete bcsdisplacements;
  delete bcstemperatures;
  delete historyoutputs;
  delete fieldoutputs;
  delete initialconditions;
  delete hbcs;
  delete steps;
  delete jobs;
  delete topology;
  delete results;
  delete timer;
  delete customlines;
  delete draw;
}

