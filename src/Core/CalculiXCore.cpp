#include "CalculiXCore.hpp"
#include "Core/steps/CoreStepsStructs.hpp"
#include <string>
#include <vector>
#include <map>
#include <array>
#include <QString>
#include "Core/topology/CoreTopologyStructs.hpp"
#include "ScientificNumber.hpp"
#include "ElementIntegration.hpp"
#include "ElementType.h"
#ifdef WIN32
 #include <io.h>
#else
 #include <unistd.h>
#endif
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cmath>
#include <set>
#include <limits>
#include <iomanip>
#include <exception>

//#include "cubitguicommondll.hpp"
//#include "CubitGuiUtil.hpp"
#include "CubitInterface.hpp"
#include "CubitPythonInterpreter.hpp"
#include "CubitMessage.hpp"
#include "MeshExportInterface.hpp"
#include "MaterialInterface.hpp"
#include "ProgressTool.hpp"
#include "AppUtil.hpp"

#include "Core/mesh/CoreBlocks.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/materials/CoreMaterialsLibrary.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/steps/CoreDamping.hpp"
#include "Core/steps/CorePhysicalConstants.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsTrajectory.hpp"
#include "Core/loads/CoreLoadsTrajectoryHeatflux.hpp"
#include "Core/loads/CoreLoadsTrajectoryBodyHeatfluxSphere.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/bcs/CoreHBCs.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/topology/CoreTopology.hpp"
#include "Core/topology/CoreTopologyOptimization.hpp"
#include "Core/results/CoreResults.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/application/CoreTimer.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "loadUserOptions.hpp"
#include "Core/drawing/CoreDraw.hpp"
#include "PlotChart.hpp"

#include <Utility/Eigen/Eigenvalues>
#include "HDF5Tool.hpp"
#include "TopologyHdf5.hpp"
#include "TopologyIterationIds.hpp"
#include <QCoreApplication>
#include <QEventLoop>
#include <QScopedValueRollback>
#include <QThread>

CalculiXCore::CalculiXCore():
  cb(NULL),mat(NULL),mat_library(NULL),sections(NULL),constraints(NULL),surfaceinteractions(NULL),
  contactpairs(NULL),amplitudes(NULL),orientations(NULL),damping(NULL),physicalconstants(NULL),
  loadsforces(NULL),loadspressures(NULL),loadsheatfluxes(NULL),
  loadsgravity(NULL),loadscentrifugal(NULL),loadstrajectory(NULL),loadsfilm(NULL),loadsradiation(NULL),
  loadssurfacetraction(NULL),loadsbodyheatflux(NULL),
  bcsdisplacements(NULL),bcstemperatures(NULL), historyoutputs(NULL), fieldoutputs(NULL),
  initialconditions(NULL), hbcs(NULL), steps(NULL),jobs(NULL),results(NULL),timer(NULL),customlines(NULL),
  draw(NULL)
{
  //init();
}

CalculiXCore::~CalculiXCore()
{
  if (timer)
  {
    timer->stop();
  }
  cancel_topology_runs();
  delete topologyoptimization;
  topologyoptimization = nullptr;
  if(cb)
  {
    delete cb;
  }
  if(mat)
  {
    delete mat;
  }
  if(mat_library)
  {
    delete mat_library;
  }
  if(sections)
  {
    delete sections;
  }
  if(constraints)
  {
    delete constraints;
  }
  if(surfaceinteractions)
  {
    delete surfaceinteractions;
  }
  if(contactpairs)
  {
    delete contactpairs;
  }
  if(amplitudes)
  {
    delete amplitudes;
  }
  if(orientations)
  {
    delete orientations;
  }
  if(damping)
  {
    delete damping;
  }
  if(physicalconstants)
  {
    delete physicalconstants;
  }
  if(loadsforces)
  {
    delete loadsforces;
  }
  if(loadspressures)
  {
    delete loadspressures;
  }
  if(loadsheatfluxes)
  {
    delete loadsheatfluxes;
  }
  if(loadsgravity)
  {
    delete loadsgravity;
  }
  if(loadscentrifugal)
  {
    delete loadscentrifugal;
  }
  if(loadstrajectory)
  {
    delete loadstrajectory;
  }
  if(loadsfilm)
  {
    delete loadsfilm;
  }
  if(loadsradiation)
  {
    delete loadsradiation;
  }
  if(loadssurfacetraction)
  {
    delete loadssurfacetraction;
  }
  if(loadsbodyheatflux)
  {
    delete loadsbodyheatflux;
  }
  if(bcsdisplacements)
  {
    delete bcsdisplacements;
  }
  if(bcstemperatures)
  {
    delete bcstemperatures;
  }
  if(historyoutputs)
  {
    delete historyoutputs;
  }
  if(fieldoutputs)
  {
    delete fieldoutputs;
  }
  if(initialconditions)
  {
    delete initialconditions;
  }
  if(hbcs)
  {
    delete hbcs;
  }
  if(steps)
  {
    delete steps;
  }
  if(jobs)
  {
    delete jobs;
  }
  if(topology)
  {
    delete topology;
  }
  if(results)
  {
    delete results;
  }
  if(timer)
  {
    delete timer;
  }
  if(customlines)
  {
    delete customlines;
  }
  if(draw)
  {
    delete draw;
  }
}

bool CalculiXCore::cmd(std::string cmd)
{
  /*
  bool status = false;
  // all commands send with CubitGuiUtil will get listed in the history
  CubitGuiUtil::send_cubit_command(cmd.c_str());
  status = !CubitInterface::was_last_cmd_undoable();
  */

  bool status = CubitInterface::cmd_single(cmd.c_str());

  return status;
}

bool CalculiXCore::silent_cmd(std::string cmd)
{
  const QScopedValueRollback<unsigned int> guard(silent_cmd_depth, silent_cmd_depth + 1);
  return CubitInterface::silent_cmd(cmd.c_str());
}

bool CalculiXCore::is_silent_cmd() const
{
  return silent_cmd_depth != 0;
}

std::string CalculiXCore::get_version()
{ 
  return this->version;
}

bool CalculiXCore::print_to_log(std::string str_log)
{
  std::ofstream output_file;
  output_file.open("CalculiXCore_logfile", std::ios_base::app);
  auto timestamp = std::chrono::system_clock::now();
  std::time_t time_t_timestamp = std::chrono::system_clock::to_time_t(timestamp);
  output_file << std::ctime(&time_t_timestamp) + str_log + "\n";
  output_file.close();
  return true;
}

bool CalculiXCore::print_to_cl(std::string str_log)
{
  PRINT_INFO("%s", str_log.c_str());
  return true;
}

bool CalculiXCore::init()
{
/*
  me_iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
  me_iface->initialize_export();
  mat_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));
*/
  if(!cb)
  {
    cb = new CoreBlocks;
  }
  
  cb->init();
/*
  if(!mat)
    mat = new CoreMaterials;
  
  mat->init();
*/
  if(!sections)
  {
    sections = new CoreSections;
  }
  
  sections->init();

  if(!constraints)
  {
    constraints = new CoreConstraints;
  }
  
  constraints->init();

  if(!surfaceinteractions)
  {
    surfaceinteractions = new CoreSurfaceInteractions;
  }
  
  surfaceinteractions->init();

  if(!contactpairs)
  {
    contactpairs = new CoreContactPairs;
  }
  
  contactpairs->init();

  if(!amplitudes)
  {
    amplitudes = new CoreAmplitudes;
  }
  
  amplitudes->init();

  if(!orientations)
  {
    orientations = new CoreOrientations;
  }
  
  orientations->init();

  if(!damping)
  {
    damping = new CoreDamping;
  }
  
  damping->init();

  if(!physicalconstants)
  {
    physicalconstants = new CorePhysicalConstants;
  }
  
  physicalconstants->init();

  if(!loadsforces)
  {
    loadsforces = new CoreLoadsForces;
  }
  
  loadsforces->init();

  if(!loadspressures)
  {
    loadspressures = new CoreLoadsPressures;
  }
  
  loadspressures->init();

  if(!loadsheatfluxes)
  {
    loadsheatfluxes = new CoreLoadsHeatfluxes;
  }
  
  loadsheatfluxes->init();

  if(!loadsgravity)
  {
    loadsgravity = new CoreLoadsGravity;
  }
  
  loadsgravity->init();

  if(!loadscentrifugal)
  {
    loadscentrifugal = new CoreLoadsCentrifugal;
  }
  
  loadscentrifugal->init();

  if(!loadstrajectory)
  {
    loadstrajectory = new CoreLoadsTrajectory;
  }
  
  loadstrajectory->init();

  if(!loadsfilm)
  {
    loadsfilm = new CoreLoadsFilm;
  }
  
  loadsfilm->init();

  if(!loadsradiation)
  {
    loadsradiation = new CoreLoadsRadiation;
  }
  
  loadsradiation->init();

  if(!loadssurfacetraction)
  {
    loadssurfacetraction = new CoreLoadsSurfaceTraction;
  }
  
  loadssurfacetraction->init();

  if(!loadsbodyheatflux)
  {
    loadsbodyheatflux = new CoreLoadsBodyHeatflux;
  }
  
  loadsbodyheatflux->init();

  if(!bcsdisplacements)
  {
    bcsdisplacements = new CoreBCsDisplacements;
  }
  
  bcsdisplacements->init();

  if(!bcstemperatures)
  {
    bcstemperatures = new CoreBCsTemperatures;
  }
  
  bcstemperatures->init();

  if(!historyoutputs)
  {
    historyoutputs = new CoreHistoryOutputs;
  }
  
  historyoutputs->init();

  if(!fieldoutputs)
  {
    fieldoutputs = new CoreFieldOutputs;
  }
  
  fieldoutputs->init();

  if(!initialconditions)
  {
    initialconditions = new CoreInitialConditions;
  }
  
  initialconditions->init();

  if(!hbcs)
  {
    hbcs = new CoreHBCs;
  }
  
  hbcs->init();

  if(!steps)
  {
    steps = new CoreSteps;
  }
  
  steps->init();

  if(!jobs)
  {
    jobs = new CoreJobs;
  }
  
  jobs->init();

  if (!topology)
  {
    topology = new CoreTopology;
  }

  topology->init();

  if(!results)
  {
    results = new CoreResults;
  }
  
  results->init();

  if(!customlines)
  {
    customlines = new CoreCustomLines;
  }
  
  customlines->init();

  if(!draw)
  {
    draw = new CoreDraw;
  }
  
  draw->init();

  if(!timer)
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

  this->bool_init = true;
  this->bool_init_pythoninterface = false;

  return true;
}

bool CalculiXCore::init2() // will be done when loading the ccm!
{ 
  me_iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
  if (me_iface==NULL)
  {
    return false;
  }
  me_iface->initialize_export();
    
  mat_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));
  if (mat_iface==NULL)
  {
    return false;
  }

  if(!mat)
  {
    mat = new CoreMaterials;
  }
  
  mat->init();

  if(!mat_library)
  {
    mat_library = new CoreMaterialsLibrary;
  }

  this->bool_init2 = true;
  
  return true;
}

bool CalculiXCore::init_pythoninterface()
{
  // init_pythoninterface(); needs to be initialized after the cubit is fully loaded; 
  // can be initialized by the user with ccx init pythoninterface

  if (!this->bool_init_pythoninterface)
  {
    CubitPythonInterpreter* interpreter = CubitInterface::python_interpreter();
    if (!interpreter || !interpreter->IsInitialized())
    {
      PRINT_ERROR("Cubit Python interpreter is not ready.\n");
      return false;
    }

    const std::string path = ccx_uo.mPathPythonInterface.toStdString();
    if (path.empty())
    {
      PRINT_ERROR("Path to Python Interface is empty.\n");
      return false;
    }

    interpreter->add_sys_path(path, true);
    if (interpreter->Run_SimpleString(
          "from calculix_pythoninterface import CalculiXPythonInterface\n"
          "ccx = CalculiXPythonInterface()") != 0)
    {
      PRINT_ERROR("Init CalculiX Python Interface Failed!\n");
      return false;
    }
    
    std::string log = "CalculiX Python Interface is ready!\n";
    PRINT_INFO("%s", log.c_str());

    this->bool_init_pythoninterface = true;
    return true;
  }
  
  return false;
}

bool CalculiXCore::init_materiallibrary()
{
  // init_materiallibrary(); needs to be initialized after the cubit is fully loaded; 
  // will be initialized with the gui timer on its first evocation

  if (!this->bool_init_materiallibrary)
  {
    this->bool_init_materiallibrary = mat_library->init();
    return this->bool_init_materiallibrary;
  }

  return false;
}

bool CalculiXCore::init_completed()
{
  if (bool_init && bool_init2 && bool_init_pythoninterface && bool_init_materiallibrary)
  {
    return true;
  }

  return false;
}

bool CalculiXCore::gui_updated()
{
  return bool_gui_updated;
}

bool CalculiXCore::set_gui_updated(bool status)
{
  bool_gui_updated = status;
  return true;
}

bool CalculiXCore::block_core_update()
{
  return bool_block_core_update;
}

bool CalculiXCore::set_block_core_update(bool status)
{
  bool_block_core_update = status;
  return true;
}

bool CalculiXCore::block_gui_update()
{
  return bool_block_gui_update;
}

bool CalculiXCore::set_block_gui_update(bool status)
{
  bool_block_gui_update = status;
  return true;
}

bool CalculiXCore::update()
{
  std::string temp_str = "";

  cb->update();
  //mat->update();
  loadsforces->update();
  loadspressures->update();
  loadsheatfluxes->update();
  bcsdisplacements->update();
  bcstemperatures->update();
  //if (topology && !topology->update())
  //return false;
  
  if (use_ccx_autocleanup)
  {
    temp_str = autocleanup();
    if (temp_str != "")
    {
      print_to_log(temp_str);
    }
  }

  if (use_ccx_logfile)
  {
    print_to_log("UPDATE");
    print_to_log(print_data());
  }

  //std::string log = "Update Cubit-CalculiX\n";
  //PRINT_INFO("%s", log.c_str());

  return true;
}

bool CalculiXCore::reset()
{
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
  mat->reset(); // to reset topo data
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
  //draw->reset();

  sideset_face_data.clear();
  //print_to_log("RESET");
  //print_to_log(print_data());

  std::string log = "Reset Cubit-CalculiX\n";
  PRINT_INFO("%s", log.c_str());

  if (timer)
  {
    timer->start(1000);
  }
  return true;
}

bool CalculiXCore::read_cub(std::string filename)
{
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
  ProgressTool* progressbar;
  progressbar = CubitInterface::app_util().get()->progress_tool();;

  log = "Reading Cubit-CalculiX data from \"" + filename + "\"\n";
  PRINT_INFO("%s", log.c_str());
  
  if (filename.substr(filename.size() - 4) == ".cub")
  {
    log = "Reading from a .cub not supported. Please save your data in a .cub5 file.\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  HDF5Tool cubTool(filename);
 
  if (!cubTool.nameExists("Cubit-CalculiX"))
  {
    log = "No Cubit-CalculiX data in \"" + filename + "\"\n";
    PRINT_INFO("%s", log.c_str());
    return true;
  }else{
    progressbar->start(0,29,"Reading Cubit-CalculiX data");
    progressbar->check_interrupt();
    //General
    std::vector<std::string> general;
    cubTool.read_dataset_string_rank_1("general","Cubit-CalculiX", general);
    if (general.size()>0)
    {
      log = "Save file was created with Cubit-CalculiX " + general[0] + "\n";
      PRINT_INFO("%s", log.c_str());
    }
    //Blocks
    cubTool.read_dataset_int_rank_2("blocks_data","Cubit-CalculiX/Blocks", cb->blocks_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Sections
    cubTool.read_dataset_int_rank_2("sections_data","Cubit-CalculiX/Sections", sections->sections_data);
    cubTool.read_dataset_string_rank_2("solid_section_data","Cubit-CalculiX/Sections", sections->solid_section_data);
    cubTool.read_dataset_string_rank_2("shell_section_data","Cubit-CalculiX/Sections", sections->shell_section_data);
    cubTool.read_dataset_string_rank_2("beam_section_data","Cubit-CalculiX/Sections", sections->beam_section_data);
    cubTool.read_dataset_string_rank_2("membrane_section_data","Cubit-CalculiX/Sections", sections->membrane_section_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Constraints
    cubTool.read_dataset_int_rank_2("constraints_data","Cubit-CalculiX/Constraints", constraints->constraints_data);
    cubTool.read_dataset_string_rank_2("rigidbody_constraint_data","Cubit-CalculiX/Constraints", constraints->rigidbody_constraint_data);
    cubTool.read_dataset_string_rank_2("tie_constraint_data","Cubit-CalculiX/Constraints", constraints->tie_constraint_data);
    cubTool.read_dataset_string_rank_2("equation_constraint_data","Cubit-CalculiX/Constraints", constraints->equation_constraint_data);
    cubTool.read_dataset_double_rank_2("equation_data","Cubit-CalculiX/Constraints", constraints->equation_data);
    cubTool.read_dataset_string_rank_2("equation_group_constraint_data","Cubit-CalculiX/Constraints", constraints->equation_group_constraint_data);
    cubTool.read_dataset_double_rank_2("equation_group_data","Cubit-CalculiX/Constraints", constraints->equation_group_data);
    progressbar->step();
    progressbar->check_interrupt();
    //SurfaceInteractions
    cubTool.read_dataset_int_rank_2("surfaceinteractions_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->surfaceinteractions_data);
    cubTool.read_dataset_string_rank_2("surfaceinteraction_name_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->surfaceinteraction_name_data);
    cubTool.read_dataset_string_rank_2("exponential_surfacebehavior_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->exponential_surfacebehavior_data);
    cubTool.read_dataset_string_rank_2("linear_surfacebehavior_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->linear_surfacebehavior_data);
    cubTool.read_dataset_string_rank_2("tabular_surfacebehavior_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->tabular_surfacebehavior_data);
    cubTool.read_dataset_string_rank_2("tied_surfacebehavior_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->tied_surfacebehavior_data);
    cubTool.read_dataset_string_rank_2("gap_conductance_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->gap_conductance_data);
    cubTool.read_dataset_string_rank_2("gap_heat_generation_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->gap_heat_generation_data);
    cubTool.read_dataset_string_rank_2("friction_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->friction_data);
    progressbar->step();
    progressbar->check_interrupt();
    //ContactPairs
    cubTool.read_dataset_int_rank_2("contactpairs_data","Cubit-CalculiX/ContactPairs", contactpairs->contactpairs_data);
    cubTool.read_dataset_string_rank_2("adjust_contactpair_data","Cubit-CalculiX/ContactPairs", contactpairs->adjust_contactpair_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Amplitudes
    cubTool.read_dataset_int_rank_2("amplitudes_data","Cubit-CalculiX/Amplitudes", amplitudes->amplitudes_data);
    cubTool.read_dataset_string_rank_2("name_amplitude_data","Cubit-CalculiX/Amplitudes", amplitudes->name_amplitude_data);
    cubTool.read_dataset_string_rank_2("shiftx_amplitude_data","Cubit-CalculiX/Amplitudes", amplitudes->shiftx_amplitude_data);
    cubTool.read_dataset_string_rank_2("shifty_amplitude_data","Cubit-CalculiX/Amplitudes", amplitudes->shifty_amplitude_data);
    cubTool.read_dataset_double_rank_2("amplitudevalues_amplitude_data","Cubit-CalculiX/Amplitudes", amplitudes->amplitudevalues_amplitude_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Orientations
    cubTool.read_dataset_int_rank_2("orientations_data","Cubit-CalculiX/Orientations", orientations->orientations_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Orientations", orientations->name_data);
    cubTool.read_dataset_string_rank_2("a_data","Cubit-CalculiX/Orientations", orientations->a_data);
    cubTool.read_dataset_string_rank_2("b_data","Cubit-CalculiX/Orientations", orientations->b_data);
    cubTool.read_dataset_string_rank_2("rotation_data","Cubit-CalculiX/Orientations", orientations->rotation_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Damping
    damping->damping_data.clear();
    cubTool.read_dataset_string_rank_1("damping_data","Cubit-CalculiX/Damping", damping->damping_data);
    if (damping->damping_data.size()==0)
    {
      damping->damping_data.push_back("");
      damping->damping_data.push_back("");
    }
    progressbar->step();
    progressbar->check_interrupt();
    //Physical Constants
    physicalconstants->physicalconstants_data.clear();
    cubTool.read_dataset_string_rank_1("physicalconstants_data","Cubit-CalculiX/PhysicalConstants", physicalconstants->physicalconstants_data);
    if (physicalconstants->physicalconstants_data.size()==0)
    {
      physicalconstants->physicalconstants_data.push_back("");
      physicalconstants->physicalconstants_data.push_back("");
      physicalconstants->physicalconstants_data.push_back("");
    }
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsForces
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Forces", loadsforces->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Forces", loadsforces->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsPressures
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Pressures", loadspressures->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Pressures", loadspressures->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsHeatFluxes
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/HeatFluxes", loadsheatfluxes->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/HeatFluxes", loadsheatfluxes->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsGravity
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->time_delay_data);
    cubTool.read_dataset_string_rank_2("direction_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->direction_data);
    cubTool.read_dataset_string_rank_2("magnitude_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->magnitude_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsCentrifugal
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->time_delay_data);
    cubTool.read_dataset_string_rank_2("direction_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->direction_data);
    cubTool.read_dataset_string_rank_2("magnitude_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->magnitude_data);
    cubTool.read_dataset_string_rank_2("coordinate_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->coordinate_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsTrajectory
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Trajectory", loadstrajectory->loads_data);
    //LoadsTrajectoryHeatflux
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->loads_data);
    cubTool.read_dataset_int_rank_2("fire_ray_surface_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->fire_ray_surface_data);
    cubTool.read_dataset_string_rank_2("direction_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->direction_data);
    cubTool.read_dataset_double_rank_2("magnitude_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->magnitude_data);
    cubTool.read_dataset_double_rank_2("radius_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->radius_data);
    cubTool.read_dataset_string_rank_2("time_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->time_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->name_data);
    //LoadsTrajectoryBodyHeatfluxSphere
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->loads_data);
    cubTool.read_dataset_int_rank_2("fire_ray_surface_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->fire_ray_surface_data);
    cubTool.read_dataset_string_rank_2("direction_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->direction_data);
    cubTool.read_dataset_double_rank_2("magnitude_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->magnitude_data);
    cubTool.read_dataset_double_rank_2("radius_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->radius_data);
    cubTool.read_dataset_double_rank_2("depth_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->depth_data);
    cubTool.read_dataset_string_rank_2("time_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->time_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsFilm
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Film", loadsfilm->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Film", loadsfilm->time_delay_data);
    cubTool.read_dataset_string_rank_2("temperature_data","Cubit-CalculiX/Loads/Film", loadsfilm->temperature_data);
    cubTool.read_dataset_string_rank_2("coefficient_data","Cubit-CalculiX/Loads/Film", loadsfilm->coefficient_data);
    cubTool.read_dataset_string_rank_2("film_time_delay_data","Cubit-CalculiX/Loads/Film", loadsfilm->film_time_delay_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Film", loadsfilm->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsRadiation
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->time_delay_data);
    cubTool.read_dataset_string_rank_2("temperature_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->temperature_data);
    cubTool.read_dataset_string_rank_2("emissivity_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->emissivity_data);
    cubTool.read_dataset_string_rank_2("radiation_time_delay_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->radiation_time_delay_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsSurfaceTraction
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/SurfaceTraction", loadssurfacetraction->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/SurfaceTraction", loadssurfacetraction->time_delay_data);
    cubTool.read_dataset_double_rank_2("force_data","Cubit-CalculiX/Loads/SurfaceTraction", loadssurfacetraction->force_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/SurfaceTraction", loadssurfacetraction->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsBodyHeatflux
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->loads_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->time_delay_data);
    cubTool.read_dataset_int_rank_2("element_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->element_data);
    cubTool.read_dataset_double_rank_2("magnitude_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->magnitude_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //BCs
    //BCsDisplacements
    cubTool.read_dataset_int_rank_2("bcs_data","Cubit-CalculiX/BCs/Displacements", bcsdisplacements->bcs_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/BCs/Displacements", bcsdisplacements->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //BCsTemperatures
    cubTool.read_dataset_int_rank_2("bcs_data","Cubit-CalculiX/BCs/Temperatures", bcstemperatures->bcs_data);
    cubTool.read_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/BCs/Temperatures", bcstemperatures->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //HistoryOutputs
    cubTool.read_dataset_int_rank_2("outputs_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->outputs_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->name_data);
    cubTool.read_dataset_string_rank_2("node_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->node_data);
    cubTool.read_dataset_string_rank_2("element_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->element_data);
    cubTool.read_dataset_string_rank_2("contact_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->contact_data);
    cubTool.read_dataset_string_rank_2("section_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->section_data);
    progressbar->step();
    progressbar->check_interrupt();
    //FieldOutputs
    cubTool.read_dataset_int_rank_2("outputs_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->outputs_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->name_data);
    cubTool.read_dataset_string_rank_2("node_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->node_data);
    cubTool.read_dataset_string_rank_2("element_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->element_data);
    cubTool.read_dataset_string_rank_2("contact_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->contact_data);
    progressbar->step();
    progressbar->check_interrupt();
    //InitialConditions
    cubTool.read_dataset_int_rank_2("initialconditions_data","Cubit-CalculiX/InitialConditions", initialconditions->initialconditions_data);
    cubTool.read_dataset_string_rank_2("displacement_data","Cubit-CalculiX/InitialConditions", initialconditions->displacement_data);
    cubTool.read_dataset_string_rank_2("temperature_data","Cubit-CalculiX/InitialConditions", initialconditions->temperature_data);
    cubTool.read_dataset_int_rank_1("stress_data","Cubit-CalculiX/InitialConditions", initialconditions->stress_data);
    cubTool.read_dataset_double_rank_2("stress_block_data","Cubit-CalculiX/InitialConditions", initialconditions->stress_block_data);
    cubTool.read_dataset_double_rank_2("stress_element_data","Cubit-CalculiX/InitialConditions", initialconditions->stress_element_data);
    progressbar->step();
    progressbar->check_interrupt();
    //HBCs
    cubTool.read_dataset_int_rank_2("bcs_data","Cubit-CalculiX/HBCs", hbcs->bcs_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Steps
    cubTool.read_dataset_int_rank_2("steps_data","Cubit-CalculiX/Steps", steps->steps_data);
    cubTool.read_dataset_string_rank_2("name_data","Cubit-CalculiX/Steps", steps->name_data);
    cubTool.read_dataset_string_rank_2("parameter_data","Cubit-CalculiX/Steps", steps->parameter_data);
    cubTool.read_dataset_string_rank_2("static_data","Cubit-CalculiX/Steps", steps->static_data);
    cubTool.read_dataset_string_rank_2("frequency_data","Cubit-CalculiX/Steps", steps->frequency_data);
    cubTool.read_dataset_string_rank_2("buckle_data","Cubit-CalculiX/Steps", steps->buckle_data);
    cubTool.read_dataset_string_rank_2("heattransfer_data","Cubit-CalculiX/Steps", steps->heattransfer_data);
    cubTool.read_dataset_string_rank_2("coupledtd_data","Cubit-CalculiX/Steps", steps->coupledtd_data);
    cubTool.read_dataset_string_rank_2("uncoupledtd_data","Cubit-CalculiX/Steps", steps->uncoupledtd_data);
    cubTool.read_dataset_string_rank_2("dynamic_data","Cubit-CalculiX/Steps", steps->dynamic_data);
    cubTool.read_dataset_string_rank_2("modal_dynamic_data","Cubit-CalculiX/Steps", steps->modal_dynamic_data);
    cubTool.read_dataset_string_rank_2("steady_state_dynamics_data","Cubit-CalculiX/Steps", steps->steady_state_dynamics_data);
    cubTool.read_dataset_string_rank_2("complex_frequency_data","Cubit-CalculiX/Steps", steps->complex_frequency_data);
    cubTool.read_dataset_int_rank_2("loads_data","Cubit-CalculiX/Steps", steps->loads_data);
    cubTool.read_dataset_int_rank_2("bcs_data","Cubit-CalculiX/Steps", steps->bcs_data);
    cubTool.read_dataset_int_rank_2("historyoutputs_data","Cubit-CalculiX/Steps", steps->historyoutputs_data);
    cubTool.read_dataset_int_rank_2("fieldoutputs_data","Cubit-CalculiX/Steps", steps->fieldoutputs_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Jobs
    if (cubTool.read_dataset_string_rank_2("jobs_data","Cubit-CalculiX/Jobs", jobs->jobs_data))
    {
      for (size_t i = 0; i < jobs->jobs_data.size(); i++)
      {
        std::vector<std::string> tmp_output_console;
        jobs->output_console.push_back(tmp_output_console);
        std::vector<std::string> tmp_cvg;
        jobs->cvg.push_back(tmp_cvg);
        std::vector<std::string> tmp_sta;
        jobs->sta.push_back(tmp_sta);
        std::string dataset = std::to_string(i);
        cubTool.read_dataset_string_rank_1(dataset,"Cubit-CalculiX/Jobs/output_console", jobs->output_console[i]);
        cubTool.read_dataset_string_rank_1(dataset,"Cubit-CalculiX/Jobs/cvg", jobs->cvg[i]);
        cubTool.read_dataset_string_rank_1(dataset,"Cubit-CalculiX/Jobs/sta", jobs->sta[i]);
      }
    }
    progressbar->step();
    progressbar->check_interrupt();
    //Results
    cubTool.read_dataset_int_rank_2("results_data","Cubit-CalculiX/Results", results->results_data);
    progressbar->step();
    progressbar->check_interrupt();
    //create empty frd and dat
    for (size_t i = 0; i < results->results_data.size(); i++)
    {
      results->create_frd_dat(results->results_data[i][1]);
    }
    for (size_t i = 0; i < results->frd_data.size(); i++)
    {
      int job_id = results->frd_data[i].job_id;
      std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(job_id) + "/";
      std::string subgroup = group;
        
      if (cubTool.nameExists(group.c_str()))
      {
        cubTool.read_dataset_string_rank_2("header",group.c_str(), results->frd_data[i].header);
        cubTool.read_dataset_string_rank_2("materials",group.c_str(), results->frd_data[i].materials);
        cubTool.read_dataset_int_rank_2("nodes",group.c_str(), results->frd_data[i].nodes);
        cubTool.read_dataset_double_rank_2("nodes_coords",group.c_str(), results->frd_data[i].nodes_coords);
        cubTool.read_dataset_int_rank_2("elements",group.c_str(), results->frd_data[i].elements);
        cubTool.read_dataset_int_rank_2("elements_connectivity",group.c_str(), results->frd_data[i].elements_connectivity);
        cubTool.read_dataset_int_rank_2("result_blocks",group.c_str(), results->frd_data[i].result_blocks);
        cubTool.read_dataset_double_rank_1("total_times",group.c_str(), results->frd_data[i].total_times);
        
        subgroup = group + "result_block_components/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "result_block_components/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "result_block_components/";
            std::vector<std::string> tmp;
            results->frd_data[i].result_block_components.push_back(tmp);
            cubTool.read_dataset_string_rank_1(dataset.c_str(),subgroup.c_str(), results->frd_data[i].result_block_components[ii]);
            ++ii;
            subgroup = group + "result_block_components/" + std::to_string(ii) +"/";
          }
        }
        cubTool.read_dataset_string_rank_1("result_block_type",group.c_str(), results->frd_data[i].result_block_type);
        subgroup = group + "result_block_data/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "result_block_data/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "result_block_data/";
            std::vector<std::vector<double>> tmp;
            results->frd_data[i].result_block_data.push_back(tmp);
            cubTool.read_dataset_double_rank_2(dataset.c_str(),subgroup.c_str(), results->frd_data[i].result_block_data[ii]);
            ++ii;
            subgroup = group + "result_block_data/" + std::to_string(ii) +"/";
          }
        }
        subgroup = group + "result_block_node_data/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "result_block_node_data/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "result_block_node_data/";
            std::vector<std::vector<int>> tmp;
            results->frd_data[i].result_block_node_data.push_back(tmp);
            cubTool.read_dataset_int_rank_2(dataset.c_str(),subgroup.c_str(), results->frd_data[i].result_block_node_data[ii]);
            ++ii;
            subgroup = group + "result_block_node_data/" + std::to_string(ii) +"/";
          }
        }
        cubTool.read_dataset_int_rank_1("sorted_node_ids",group.c_str(), results->frd_data[i].sorted_node_ids);
        cubTool.read_dataset_int_rank_1("sorted_node_data_ids",group.c_str(), results->frd_data[i].sorted_node_data_ids);
        subgroup = group + "sorted_result_node_ids/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "sorted_result_node_ids/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "sorted_result_node_ids/";
            std::vector<int> tmp;
            results->frd_data[i].sorted_result_node_ids.push_back(tmp);
            cubTool.read_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->frd_data[i].sorted_result_node_ids[ii]);
            ++ii;
            subgroup = group + "sorted_result_node_ids/" + std::to_string(ii) +"/";
          }
        }
        subgroup = group + "sorted_result_node_ids/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "sorted_result_node_data_ids/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "sorted_result_node_data_ids/";
            std::vector<int> tmp;
            results->frd_data[i].sorted_result_node_data_ids.push_back(tmp);
            cubTool.read_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->frd_data[i].sorted_result_node_data_ids[ii]);
            ++ii;
            subgroup = group + "sorted_result_node_data_ids/" + std::to_string(ii) +"/";
          }
        }        
      }
    }

    for (size_t i = 0; i < results->dat_data.size(); i++)
    {
      int job_id = results->dat_data[i].job_id;
      std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(job_id) + "/";
      std::string subgroup = group;
      if (cubTool.nameExists(group.c_str()))
      {
        cubTool.read_dataset_int_rank_2("result_blocks",group.c_str(), results->dat_data[i].result_blocks);
        cubTool.read_dataset_double_rank_1("total_times",group.c_str(), results->dat_data[i].total_times);
        subgroup = group + "result_block_components/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "result_block_components/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "result_block_components/";
            std::vector<std::string> tmp;
            results->dat_data[i].result_block_components.push_back(tmp);
            cubTool.read_dataset_string_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_block_components[ii]);
            ++ii;
            subgroup = group + "result_block_components/" + std::to_string(ii) +"/";
          }
        }
        cubTool.read_dataset_string_rank_1("result_block_type",group.c_str(), results->dat_data[i].result_block_type);
        cubTool.read_dataset_string_rank_1("result_block_set",group.c_str(), results->dat_data[i].result_block_set);
        subgroup = group + "result_block_data/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "result_block_data/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "result_block_data/";
            std::vector<std::vector<double>> tmp;
            results->dat_data[i].result_block_data.push_back(tmp);
            cubTool.read_dataset_double_rank_2(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_block_data[ii]);
            ++ii;
            subgroup = group + "result_block_data/" + std::to_string(ii) +"/";
          }
        }
        subgroup = group + "result_block_c1_data/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "result_block_c1_data/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "result_block_c1_data/";
            std::vector<std::vector<int>> tmp;
            results->dat_data[i].result_block_c1_data.push_back(tmp);
            cubTool.read_dataset_int_rank_2(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_block_c1_data[ii]);
            ++ii;
            subgroup = group + "result_block_c1_data/" + std::to_string(ii) +"/";
          }
        }
        subgroup = group + "buckle_data/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "buckle_data/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "buckle_data/";
            std::vector<std::vector<double>> tmp;
            results->dat_data[i].buckle_data.push_back(tmp);
            cubTool.read_dataset_double_rank_2(dataset.c_str(),subgroup.c_str(), results->dat_data[i].buckle_data[ii]);
            ++ii;
            subgroup = group + "buckle_data/" + std::to_string(ii) +"/";
          }
        }
        subgroup = group + "result_section/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "result_section/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "result_section/";
            std::vector<int> tmp;
            results->dat_data[i].result_section.push_back(tmp);
            cubTool.read_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_section[ii]);
            ++ii;
            subgroup = group + "result_section/" + std::to_string(ii) +"/";
          }
        }
        cubTool.read_dataset_string_rank_1("result_section_set",group.c_str(), results->dat_data[i].result_section_set);
        // init() supplies default labels. Read into an empty buffer and
        // replace them only after the saved labels have been read successfully.
        if (cubTool.nameExists(group + "result_section_label"))
        {
          std::vector<std::string> section_labels;
          if (!cubTool.read_dataset_string_rank_1(
                  "result_section_label", group, section_labels))
          {
            progressbar->end();
            PRINT_ERROR("Unable to read DAT section labels for job %d.\n", job_id);
            return false;
          }
          results->dat_data[i].result_section_label.swap(section_labels);
        }
        subgroup = group + "result_section_data/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "result_section_data/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "result_section_data/";
            std::vector<double> tmp;
            results->dat_data[i].result_section_data.push_back(tmp);
            cubTool.read_dataset_double_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_section_data[ii]);
            ++ii;
            subgroup = group + "result_section_data/" + std::to_string(ii) +"/";
          }
        }
        subgroup = group + "sorted_c1/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "sorted_c1/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "sorted_c1/";
            std::vector<int> tmp;
            results->dat_data[i].sorted_c1.push_back(tmp);
            cubTool.read_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].sorted_c1[ii]);
            ++ii;
            subgroup = group + "sorted_c1/" + std::to_string(ii) +"/";
          }
        }
        subgroup = group + "sorted_result_block_c1_data_id/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "sorted_result_block_c1_data_id/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "sorted_result_block_c1_data_id/";
            std::vector<int> tmp;
            results->dat_data[i].sorted_result_block_c1_data_id.push_back(tmp);
            cubTool.read_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].sorted_result_block_c1_data_id[ii]);
            ++ii;
            subgroup = group + "sorted_result_block_c1_data_id/" + std::to_string(ii) +"/";
          }
        }
        subgroup = group + "sorted_result_block_c1_data_type/";
        if (cubTool.nameExists(subgroup.c_str()))
        {
          int ii = 0;
          subgroup = group + "sorted_result_block_c1_data_type/" + std::to_string(ii) +"/";
          while (cubTool.nameExists(subgroup.c_str()))
          {
            std::string dataset = std::to_string(ii);
            subgroup = group + "sorted_result_block_c1_data_type/";
            std::vector<int> tmp;
            results->dat_data[i].sorted_result_block_c1_data_type.push_back(tmp);
            cubTool.read_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].sorted_result_block_c1_data_type[ii]);
            ++ii;
            subgroup = group + "sorted_result_block_c1_data_type/" + std::to_string(ii) +"/";
          }
        }
      }
    }
    progressbar->step();
    progressbar->check_interrupt();
    //CustomLines
    cubTool.read_dataset_string_rank_2("customlines_data","Cubit-CalculiX/CustomLines", customlines->customlines_data);
    progressbar->step();
    progressbar->check_interrupt();
    if (!topology->read_configuration(cubTool))
    {
      progressbar->end();
      PRINT_ERROR("Unable to read topology configurations.\n");
      return false;
    }

    try
    {
      decltype(mat->topology_material_data) materials;
      decltype(sections->topology_solid_sections) solid_sections;
      const std::string root = "Cubit-CalculiX/Topology";
      std::vector<int> topology_version;

      if (cubTool.nameExists(root))
      {
        if (!cubTool.read_dataset_int_rank_1("version", root, topology_version) ||
            topology_version.size() != 1)
        {
          progressbar->end();
          PRINT_ERROR("Unable to read topology save version.\n");
          return false;
        }
      }

      if (!topology_version.empty() && topology_version[0] >= 2)
      {
        TopologyHdf5<true> archive(cubTool, "Cubit-CalculiX");
        if (!archive.value("TopologyMaterials", materials) ||
            !archive.value("TopologySections", solid_sections))
        {
          progressbar->end();
          PRINT_ERROR("Unable to read topology material/section data.\n");
          return false;
        }
      }

      if (!topology_version.empty() && topology_version[0] == 2)
      {
        for (auto& collection : materials)
        {
          if (!increment_topology_iteration_keys(collection.second.iteration_factors) || !increment_topology_iteration_keys(collection.second.iteration_material_caps))
          {
            progressbar->end();
            PRINT_ERROR("Unable to migrate topology material factors.\n");
            return false;
          }
        }
      }

      for (const auto& collection : materials)
      {
        if (!valid_topology_iteration_keys(collection.second.iteration_factors) || !valid_topology_iteration_keys(collection.second.iteration_material_caps))
        {
          progressbar->end();
          PRINT_ERROR("Invalid saved topology material iteration ID.\n");
          return false;
        }
      }
      // A saved PID does not belong to a solver launched by this session.
      for (const auto& configuration : topology->topology_cases)
      {
        for (const auto* associations : {&configuration.second.iteration_jobs, &configuration.second.iteration_adjoint_jobs})
        {
          for (const auto& iteration : *associations)
          {
            const int index = jobs->get_jobs_data_id_from_job_id(iteration.second);
            if (index >= 0 && (jobs->jobs_data[index][3] == "1" || jobs->jobs_data[index][3] == "5"))
            {
              jobs->jobs_data[index][3] = "4";
              jobs->jobs_data[index][4] = "-1";
            }
          }
        }
      }
      mat->topology_material_data.swap(materials);
      sections->topology_solid_sections.swap(solid_sections);

      // Dependencies and job results are loaded before checking selections.
      for (const auto& entry : topology->topology_cases)
      {
        const int iteration = entry.second.selected_iteration;
        if (iteration >= 0)
        {
          if (get_topology_iteration_element_data(entry.first, iteration).empty())
          {
            progressbar->end();
            PRINT_ERROR("Unable to restore selected iteration for topology %d.\n",
                        entry.first);
            return false;
          }
        }
      }
    }
    catch (const H5::Exception& error)
    {
      progressbar->end();
      PRINT_ERROR("Unable to read topology material/section data: %s\n",
                  error.getDetailMsg().c_str());
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
  }

  return true;
}

bool CalculiXCore::save_cub(std::string filename)
{
  std::string log = "Saving Cubit-CaluliX data to \"" + filename + "\"\n";
  PRINT_INFO("%s", log.c_str());
  ProgressTool* progressbar;
  progressbar = CubitInterface::app_util().get()->progress_tool();;

  if (filename.substr(filename.size() - 4) == ".cub")
  {
    log = "Writing to a .cub not supported. Please save your data in a .cub5 file.\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  HDF5Tool cubTool(filename);

  if (!cubTool.nameExists("Cubit-CalculiX"))
  {
    progressbar->start(0,29,"Writing Cubit-CalculiX data");
    progressbar->check_interrupt();
    //General
    cubTool.createGroup("Cubit-CalculiX");
    std::vector<std::string> general;
    general.push_back(this->version); //version info
    cubTool.write_dataset_string_rank_1("general","Cubit-CalculiX", general);
    //Core
    cubTool.createGroup("Cubit-CalculiX/Core");
    //Blocks
    cubTool.createGroup("Cubit-CalculiX/Blocks");
    cubTool.write_dataset_int_rank_2("blocks_data","Cubit-CalculiX/Blocks", cb->blocks_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Materials
    cubTool.createGroup("Cubit-CalculiX/Materials");
    progressbar->step();
    progressbar->check_interrupt();
    //Sections
    cubTool.createGroup("Cubit-CalculiX/Sections");
    cubTool.write_dataset_int_rank_2("sections_data","Cubit-CalculiX/Sections", sections->sections_data);
    cubTool.write_dataset_string_rank_2("solid_section_data","Cubit-CalculiX/Sections", sections->solid_section_data);
    cubTool.write_dataset_string_rank_2("shell_section_data","Cubit-CalculiX/Sections", sections->shell_section_data);
    cubTool.write_dataset_string_rank_2("beam_section_data","Cubit-CalculiX/Sections", sections->beam_section_data);
    cubTool.write_dataset_string_rank_2("membrane_section_data","Cubit-CalculiX/Sections", sections->membrane_section_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Constraints
    cubTool.createGroup("Cubit-CalculiX/Constraints");
    cubTool.write_dataset_int_rank_2("constraints_data","Cubit-CalculiX/Constraints", constraints->constraints_data);
    cubTool.write_dataset_string_rank_2("rigidbody_constraint_data","Cubit-CalculiX/Constraints", constraints->rigidbody_constraint_data);
    cubTool.write_dataset_string_rank_2("tie_constraint_data","Cubit-CalculiX/Constraints", constraints->tie_constraint_data);
    cubTool.write_dataset_string_rank_2("equation_constraint_data","Cubit-CalculiX/Constraints", constraints->equation_constraint_data);
    cubTool.write_dataset_double_rank_2("equation_data","Cubit-CalculiX/Constraints", constraints->equation_data);
    cubTool.write_dataset_string_rank_2("equation_group_constraint_data","Cubit-CalculiX/Constraints", constraints->equation_group_constraint_data);
    cubTool.write_dataset_double_rank_2("equation_group_data","Cubit-CalculiX/Constraints", constraints->equation_group_data);
    progressbar->step();
    progressbar->check_interrupt();
    //SurfaceInteractions
    cubTool.createGroup("Cubit-CalculiX/SurfaceInteractions");
    cubTool.write_dataset_int_rank_2("surfaceinteractions_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->surfaceinteractions_data);
    cubTool.write_dataset_string_rank_2("surfaceinteraction_name_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->surfaceinteraction_name_data);
    cubTool.write_dataset_string_rank_2("exponential_surfacebehavior_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->exponential_surfacebehavior_data);
    cubTool.write_dataset_string_rank_2("linear_surfacebehavior_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->linear_surfacebehavior_data);
    cubTool.write_dataset_string_rank_2("tabular_surfacebehavior_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->tabular_surfacebehavior_data);
    cubTool.write_dataset_string_rank_2("tied_surfacebehavior_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->tied_surfacebehavior_data);
    cubTool.write_dataset_string_rank_2("gap_conductance_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->gap_conductance_data);
    cubTool.write_dataset_string_rank_2("gap_heat_generation_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->gap_heat_generation_data);
    cubTool.write_dataset_string_rank_2("friction_data","Cubit-CalculiX/SurfaceInteractions", surfaceinteractions->friction_data);
    progressbar->step();
    progressbar->check_interrupt();
    //ContactPairs
    cubTool.createGroup("Cubit-CalculiX/ContactPairs");
    cubTool.write_dataset_int_rank_2("contactpairs_data","Cubit-CalculiX/ContactPairs", contactpairs->contactpairs_data);
    cubTool.write_dataset_string_rank_2("adjust_contactpair_data","Cubit-CalculiX/ContactPairs", contactpairs->adjust_contactpair_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Amplitudes
    cubTool.createGroup("Cubit-CalculiX/Amplitudes");
    cubTool.write_dataset_int_rank_2("amplitudes_data","Cubit-CalculiX/Amplitudes", amplitudes->amplitudes_data);
    cubTool.write_dataset_string_rank_2("name_amplitude_data","Cubit-CalculiX/Amplitudes", amplitudes->name_amplitude_data);
    cubTool.write_dataset_string_rank_2("shiftx_amplitude_data","Cubit-CalculiX/Amplitudes", amplitudes->shiftx_amplitude_data);
    cubTool.write_dataset_string_rank_2("shifty_amplitude_data","Cubit-CalculiX/Amplitudes", amplitudes->shifty_amplitude_data);
    cubTool.write_dataset_double_rank_2("amplitudevalues_amplitude_data","Cubit-CalculiX/Amplitudes", amplitudes->amplitudevalues_amplitude_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Orientations
    cubTool.createGroup("Cubit-CalculiX/Orientations");
    cubTool.write_dataset_int_rank_2("orientations_data","Cubit-CalculiX/Orientations", orientations->orientations_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Orientations", orientations->name_data);
    cubTool.write_dataset_string_rank_2("a_data","Cubit-CalculiX/Orientations", orientations->a_data);
    cubTool.write_dataset_string_rank_2("b_data","Cubit-CalculiX/Orientations", orientations->b_data);
    cubTool.write_dataset_string_rank_2("rotation_data","Cubit-CalculiX/Orientations", orientations->rotation_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Damping
    cubTool.createGroup("Cubit-CalculiX/Damping");
    cubTool.write_dataset_string_rank_1("damping_data","Cubit-CalculiX/Damping", damping->damping_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Physical Constants
    cubTool.createGroup("Cubit-CalculiX/PhysicalConstants");
    cubTool.write_dataset_string_rank_1("physicalconstants_data","Cubit-CalculiX/PhysicalConstants", physicalconstants->physicalconstants_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Loads
    cubTool.createGroup("Cubit-CalculiX/Loads");
    //LoadsForces
    cubTool.createGroup("Cubit-CalculiX/Loads/Forces");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Forces", loadsforces->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Forces", loadsforces->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsPressures
    cubTool.createGroup("Cubit-CalculiX/Loads/Pressures");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Pressures", loadspressures->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Pressures", loadspressures->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsHeatFluxes
    cubTool.createGroup("Cubit-CalculiX/Loads/HeatFluxes");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/HeatFluxes", loadsheatfluxes->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/HeatFluxes", loadsheatfluxes->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsGravity
    cubTool.createGroup("Cubit-CalculiX/Loads/Gravity");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->time_delay_data);
    cubTool.write_dataset_string_rank_2("direction_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->direction_data);
    cubTool.write_dataset_string_rank_2("magnitude_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->magnitude_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Gravity", loadsgravity->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsCentrifugal
    cubTool.createGroup("Cubit-CalculiX/Loads/Centrifugal");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->time_delay_data);
    cubTool.write_dataset_string_rank_2("direction_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->direction_data);
    cubTool.write_dataset_string_rank_2("magnitude_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->magnitude_data);
    cubTool.write_dataset_string_rank_2("coordinate_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->coordinate_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Centrifugal", loadscentrifugal->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsTrajectory
    cubTool.createGroup("Cubit-CalculiX/Loads/Trajectory");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Trajectory", loadstrajectory->loads_data);
    //LoadsTrajectoryHeatflux
    cubTool.createGroup("Cubit-CalculiX/Loads/Trajectory/Heatflux");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->loads_data);
    cubTool.write_dataset_int_rank_2("fire_ray_surface_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->fire_ray_surface_data);
    cubTool.write_dataset_string_rank_2("direction_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->direction_data);
    cubTool.write_dataset_double_rank_2("magnitude_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->magnitude_data);
    cubTool.write_dataset_double_rank_2("radius_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->radius_data);
    cubTool.write_dataset_string_rank_2("time_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->time_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Trajectory/Heatflux", loadstrajectory->heatflux->name_data);
    //LoadsTrajectoryBodyHeatfluxSphere
    cubTool.createGroup("Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->loads_data);
    cubTool.write_dataset_int_rank_2("fire_ray_surface_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->fire_ray_surface_data);
    cubTool.write_dataset_string_rank_2("direction_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->direction_data);
    cubTool.write_dataset_double_rank_2("magnitude_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->magnitude_data);
    cubTool.write_dataset_double_rank_2("radius_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->radius_data);
    cubTool.write_dataset_double_rank_2("depth_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->depth_data);
    cubTool.write_dataset_string_rank_2("time_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->time_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Trajectory/BodyHeatfluxSphere", loadstrajectory->bodyheatfluxsphere->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsFilm
    cubTool.createGroup("Cubit-CalculiX/Loads/Film");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Film", loadsfilm->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Film", loadsfilm->time_delay_data);
    cubTool.write_dataset_string_rank_2("temperature_data","Cubit-CalculiX/Loads/Film", loadsfilm->temperature_data);
    cubTool.write_dataset_string_rank_2("coefficient_data","Cubit-CalculiX/Loads/Film", loadsfilm->coefficient_data);
    cubTool.write_dataset_string_rank_2("film_time_delay_data","Cubit-CalculiX/Loads/Film", loadsfilm->film_time_delay_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Film", loadsfilm->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsRadiation
    cubTool.createGroup("Cubit-CalculiX/Loads/Radiation");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->time_delay_data);
    cubTool.write_dataset_string_rank_2("temperature_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->temperature_data);
    cubTool.write_dataset_string_rank_2("emissivity_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->emissivity_data);
    cubTool.write_dataset_string_rank_2("radiation_time_delay_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->radiation_time_delay_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/Radiation", loadsradiation->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsSurfaceTraction
    cubTool.createGroup("Cubit-CalculiX/Loads/SurfaceTraction");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/SurfaceTraction", loadssurfacetraction->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/SurfaceTraction", loadssurfacetraction->time_delay_data);
    cubTool.write_dataset_double_rank_2("force_data","Cubit-CalculiX/Loads/SurfaceTraction", loadssurfacetraction->force_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/SurfaceTraction", loadssurfacetraction->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //LoadsBodyHeatflux
    cubTool.createGroup("Cubit-CalculiX/Loads/BodyHeatflux");
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->loads_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->time_delay_data);
    cubTool.write_dataset_int_rank_2("element_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->element_data);
    cubTool.write_dataset_double_rank_2("magnitude_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->magnitude_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Loads/BodyHeatflux", loadsbodyheatflux->name_data);
    progressbar->step();
    progressbar->check_interrupt();
    //BCs
    cubTool.createGroup("Cubit-CalculiX/BCs");
    //BCsDisplacements
    cubTool.createGroup("Cubit-CalculiX/BCs/Displacements");
    cubTool.write_dataset_int_rank_2("bcs_data","Cubit-CalculiX/BCs/Displacements", bcsdisplacements->bcs_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/BCs/Displacements", bcsdisplacements->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //BCsTemperatures
    cubTool.createGroup("Cubit-CalculiX/BCs/Temperatures");
    cubTool.write_dataset_int_rank_2("bcs_data","Cubit-CalculiX/BCs/Temperatures", bcstemperatures->bcs_data);
    cubTool.write_dataset_string_rank_2("time_delay_data","Cubit-CalculiX/BCs/Temperatures", bcstemperatures->time_delay_data);
    progressbar->step();
    progressbar->check_interrupt();
    //HistoryOutputs
    cubTool.createGroup("Cubit-CalculiX/HistoryOutputs");
    cubTool.write_dataset_int_rank_2("outputs_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->outputs_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->name_data);
    cubTool.write_dataset_string_rank_2("node_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->node_data);
    cubTool.write_dataset_string_rank_2("element_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->element_data);
    cubTool.write_dataset_string_rank_2("contact_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->contact_data);
    cubTool.write_dataset_string_rank_2("section_data","Cubit-CalculiX/HistoryOutputs", historyoutputs->section_data);
    progressbar->step();
    progressbar->check_interrupt();
    //FieldOutputs
    cubTool.createGroup("Cubit-CalculiX/FieldOutputs");
    cubTool.write_dataset_int_rank_2("outputs_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->outputs_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->name_data);
    cubTool.write_dataset_string_rank_2("node_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->node_data);
    cubTool.write_dataset_string_rank_2("element_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->element_data);
    cubTool.write_dataset_string_rank_2("contact_data","Cubit-CalculiX/FieldOutputs", fieldoutputs->contact_data);
    progressbar->step();
    progressbar->check_interrupt();
    //InitialConditions
    cubTool.createGroup("Cubit-CalculiX/InitialConditions");
    cubTool.write_dataset_int_rank_2("initialconditions_data","Cubit-CalculiX/InitialConditions", initialconditions->initialconditions_data);
    cubTool.write_dataset_string_rank_2("displacement_data","Cubit-CalculiX/InitialConditions", initialconditions->displacement_data);
    cubTool.write_dataset_string_rank_2("temperature_data","Cubit-CalculiX/InitialConditions", initialconditions->temperature_data);
    cubTool.write_dataset_int_rank_1("stress_data","Cubit-CalculiX/InitialConditions", initialconditions->stress_data);
    cubTool.write_dataset_double_rank_2("stress_block_data","Cubit-CalculiX/InitialConditions", initialconditions->stress_block_data);
    cubTool.write_dataset_double_rank_2("stress_element_data","Cubit-CalculiX/InitialConditions", initialconditions->stress_element_data);
    progressbar->step();
    progressbar->check_interrupt();
    //HBCs
    cubTool.createGroup("Cubit-CalculiX/HBCs");
    cubTool.write_dataset_int_rank_2("bcs_data","Cubit-CalculiX/HBCs", hbcs->bcs_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Steps
    cubTool.createGroup("Cubit-CalculiX/Steps");
    cubTool.write_dataset_int_rank_2("steps_data","Cubit-CalculiX/Steps", steps->steps_data);
    cubTool.write_dataset_string_rank_2("name_data","Cubit-CalculiX/Steps", steps->name_data);
    cubTool.write_dataset_string_rank_2("parameter_data","Cubit-CalculiX/Steps", steps->parameter_data);
    cubTool.write_dataset_string_rank_2("static_data","Cubit-CalculiX/Steps", steps->static_data);
    cubTool.write_dataset_string_rank_2("frequency_data","Cubit-CalculiX/Steps", steps->frequency_data);
    cubTool.write_dataset_string_rank_2("buckle_data","Cubit-CalculiX/Steps", steps->buckle_data);
    cubTool.write_dataset_string_rank_2("heattransfer_data","Cubit-CalculiX/Steps", steps->heattransfer_data);
    cubTool.write_dataset_string_rank_2("coupledtd_data","Cubit-CalculiX/Steps", steps->coupledtd_data);
    cubTool.write_dataset_string_rank_2("uncoupledtd_data","Cubit-CalculiX/Steps", steps->uncoupledtd_data);
    cubTool.write_dataset_string_rank_2("dynamic_data","Cubit-CalculiX/Steps", steps->dynamic_data);
    cubTool.write_dataset_string_rank_2("modal_dynamic_data","Cubit-CalculiX/Steps", steps->modal_dynamic_data);
    cubTool.write_dataset_string_rank_2("steady_state_dynamics_data","Cubit-CalculiX/Steps", steps->steady_state_dynamics_data);
    cubTool.write_dataset_string_rank_2("complex_frequency_data","Cubit-CalculiX/Steps", steps->complex_frequency_data);
    cubTool.write_dataset_int_rank_2("loads_data","Cubit-CalculiX/Steps", steps->loads_data);
    cubTool.write_dataset_int_rank_2("bcs_data","Cubit-CalculiX/Steps", steps->bcs_data);
    cubTool.write_dataset_int_rank_2("historyoutputs_data","Cubit-CalculiX/Steps", steps->historyoutputs_data);
    cubTool.write_dataset_int_rank_2("fieldoutputs_data","Cubit-CalculiX/Steps", steps->fieldoutputs_data);
    progressbar->step();
    progressbar->check_interrupt();
    //Jobs
    cubTool.createGroup("Cubit-CalculiX/Jobs");
    cubTool.createGroup("Cubit-CalculiX/Jobs/output_console");
    cubTool.createGroup("Cubit-CalculiX/Jobs/cvg");
    cubTool.createGroup("Cubit-CalculiX/Jobs/sta");
    cubTool.write_dataset_string_rank_2("jobs_data","Cubit-CalculiX/Jobs", jobs->jobs_data);
    for (size_t i = 0; i < jobs->jobs_data.size(); i++)
    {
      std::string dataset = std::to_string(i);
      cubTool.write_dataset_string_rank_1(dataset.c_str(),"Cubit-CalculiX/Jobs/output_console", jobs->output_console[i]);
      cubTool.write_dataset_string_rank_1(dataset.c_str(),"Cubit-CalculiX/Jobs/cvg", jobs->cvg[i]);
      cubTool.write_dataset_string_rank_1(dataset.c_str(),"Cubit-CalculiX/Jobs/sta", jobs->sta[i]);
    }
    progressbar->step();
    progressbar->check_interrupt();
    //Results
    cubTool.createGroup("Cubit-CalculiX/Results");
    cubTool.write_dataset_int_rank_2("results_data","Cubit-CalculiX/Results", results->results_data);
    cubTool.createGroup("Cubit-CalculiX/Results/Frd");
    cubTool.createGroup("Cubit-CalculiX/Results/Dat");
    if (ccx_uo.mSaveLoadedResults)
    {
      for (size_t i = 0; i < results->frd_data.size(); i++)
      {
        int job_id = results->frd_data[i].job_id;
        std::string group = "Cubit-CalculiX/Results/Frd/" + std::to_string(job_id) + "/";
        cubTool.createGroup(group.c_str());
        cubTool.write_dataset_string_rank_2("header",group.c_str(), results->frd_data[i].header);
        cubTool.write_dataset_string_rank_2("materials",group.c_str(), results->frd_data[i].materials);
        cubTool.write_dataset_int_rank_2("nodes",group.c_str(), results->frd_data[i].nodes);
        cubTool.write_dataset_double_rank_2("nodes_coords",group.c_str(), results->frd_data[i].nodes_coords);
        cubTool.write_dataset_int_rank_2("elements",group.c_str(), results->frd_data[i].elements);
        cubTool.write_dataset_int_rank_2("elements_connectivity",group.c_str(), results->frd_data[i].elements_connectivity);
        cubTool.write_dataset_int_rank_2("result_blocks",group.c_str(), results->frd_data[i].result_blocks);
        cubTool.write_dataset_double_rank_1("total_times",group.c_str(), results->frd_data[i].total_times);
        if (results->frd_data[i].result_block_components.size()>0)
        {
          std::string subgroup = group + "result_block_components/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->frd_data[i].result_block_components.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_string_rank_1(dataset.c_str(),subgroup.c_str(), results->frd_data[i].result_block_components[ii]);
          }
        }
        cubTool.write_dataset_string_rank_1("result_block_type",group.c_str(), results->frd_data[i].result_block_type);
        if (results->frd_data[i].result_block_data.size()>0)
        {
          std::string subgroup = group + "result_block_data/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->frd_data[i].result_block_data.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_double_rank_2(dataset.c_str(),subgroup.c_str(), results->frd_data[i].result_block_data[ii]);
          }
        }
        if (results->frd_data[i].result_block_node_data.size()>0)
        {
          std::string subgroup = group + "result_block_node_data/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->frd_data[i].result_block_node_data.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_int_rank_2(dataset.c_str(),subgroup.c_str(), results->frd_data[i].result_block_node_data[ii]);
          }
        }
        cubTool.write_dataset_int_rank_1("sorted_node_ids",group.c_str(), results->frd_data[i].sorted_node_ids);
        cubTool.write_dataset_int_rank_1("sorted_node_data_ids",group.c_str(), results->frd_data[i].sorted_node_data_ids);
        if (results->frd_data[i].sorted_result_node_ids.size()>0)
        {
          std::string subgroup = group + "sorted_result_node_ids/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->frd_data[i].sorted_result_node_ids.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->frd_data[i].sorted_result_node_ids[ii]);
          }
        }
        if (results->frd_data[i].sorted_result_node_data_ids.size()>0)
        {
          std::string subgroup = group + "sorted_result_node_data_ids/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->frd_data[i].sorted_result_node_data_ids.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->frd_data[i].sorted_result_node_data_ids[ii]);
          }
        }
      }
      for (size_t i = 0; i < results->dat_data.size(); i++)
      {
        int job_id = results->dat_data[i].job_id;
        std::string group = "Cubit-CalculiX/Results/Dat/" + std::to_string(job_id) + "/";
        cubTool.createGroup(group.c_str());
        cubTool.write_dataset_int_rank_2("result_blocks",group.c_str(), results->dat_data[i].result_blocks);
        cubTool.write_dataset_double_rank_1("total_times",group.c_str(), results->dat_data[i].total_times);
        if (results->dat_data[i].result_block_components.size()>0)
        {
          std::string subgroup = group + "result_block_components/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].result_block_components.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_string_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_block_components[ii]);
          }
        }
        cubTool.write_dataset_string_rank_1("result_block_type",group.c_str(), results->dat_data[i].result_block_type);
        cubTool.write_dataset_string_rank_1("result_block_set",group.c_str(), results->dat_data[i].result_block_set);
        if (results->dat_data[i].result_block_data.size()>0)
        {
          std::string subgroup = group + "result_block_data/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].result_block_data.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_double_rank_2(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_block_data[ii]);
          }
        }
        if (results->dat_data[i].result_block_c1_data.size()>0)
        {
          std::string subgroup = group + "result_block_c1_data/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].result_block_c1_data.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_int_rank_2(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_block_c1_data[ii]);
          }
        }
        if (results->dat_data[i].buckle_data.size()>0)
        {
          std::string subgroup = group + "buckle_data/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].buckle_data.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_double_rank_2(dataset.c_str(),subgroup.c_str(), results->dat_data[i].buckle_data[ii]);
          }
        }
        if (results->dat_data[i].result_section.size()>0)
        {
          std::string subgroup = group + "result_section/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].result_section.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_section[ii]);
          }
        }
        cubTool.write_dataset_string_rank_1("result_section_set",group.c_str(), results->dat_data[i].result_section_set);
        cubTool.write_dataset_string_rank_1("result_section_label",group.c_str(), results->dat_data[i].result_section_label);
        if (results->dat_data[i].result_section_data.size()>0)
        {
          std::string subgroup = group + "result_section_data/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].result_section_data.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_double_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].result_section_data[ii]);
          }
        }
        if (results->dat_data[i].sorted_c1.size()>0)
        {
          std::string subgroup = group + "sorted_c1/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].sorted_c1.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].sorted_c1[ii]);
          }
        }
        if (results->dat_data[i].sorted_result_block_c1_data_id.size()>0)
        {
          std::string subgroup = group + "sorted_result_block_c1_data_id/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].sorted_result_block_c1_data_id.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].sorted_result_block_c1_data_id[ii]);
          }
        }
        if (results->dat_data[i].sorted_result_block_c1_data_type.size()>0)
        {
          std::string subgroup = group + "sorted_result_block_c1_data_type/";
          cubTool.createGroup(subgroup.c_str());
          for (size_t ii = 0; ii < results->dat_data[i].sorted_result_block_c1_data_type.size(); ii++)
          {
            std::string dataset = std::to_string(ii);
            cubTool.write_dataset_int_rank_1(dataset.c_str(),subgroup.c_str(), results->dat_data[i].sorted_result_block_c1_data_type[ii]);
          }
        }
      }
    }
    progressbar->step();
    progressbar->check_interrupt();
    //CustomLines
    cubTool.createGroup("Cubit-CalculiX/CustomLines");
    cubTool.write_dataset_string_rank_2("customlines_data","Cubit-CalculiX/CustomLines", customlines->customlines_data);
    progressbar->step();
    progressbar->check_interrupt();
    if (!topology->save_configuration(cubTool))
    {
      progressbar->end();
      PRINT_ERROR("Unable to save topology configurations.\n");
      return false;
    }

    try
    {
      TopologyHdf5<false> archive(cubTool, "Cubit-CalculiX");
      if (!archive.value("TopologyMaterials", mat->topology_material_data) ||
          !archive.value("TopologySections", sections->topology_solid_sections))
      {
        progressbar->end();
        PRINT_ERROR("Unable to save topology material/section data.\n");
        return false;
      }
    }
    catch (const H5::Exception& error)
    {
      progressbar->end();
      PRINT_ERROR("Unable to save topology material/section data: %s\n",
                  error.getDetailMsg().c_str());
      return false;
    }
    progressbar->end();

    log = "Finished saving Cubit-CalculiX data.\n";
    PRINT_INFO("%s", log.c_str());
  }else{
    log = "group already here. Can't write Cubit-CalculiX data into \"" + filename + "\"\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  return true;
}

std::string CalculiXCore::autocleanup()
{
  std::string log = "";
  bool print_log = false;
  int sub_data_id;
  std::vector<int> sub_data_ids;
  bool sub_bool;
  bool sub_bool_ref;

  std::string output;

  log = "CCX Autocleanup\n";

  // SECTIONS
  //loop over all sections
  for (size_t i = sections->sections_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    sub_bool_ref = false;
    // SOLID
    if (sections->sections_data[i-1][1] == 1)
    {
      sub_data_id = sections->get_solid_section_data_id_from_solid_section_id(sections->sections_data[i-1][2]);
      if (!mat->check_material_exists(std::stoi(sections->solid_section_data[sub_data_id][2])))
      {
        log.append("Material " + sections->solid_section_data[sub_data_id][2] + " doesn't exist.\n");
        log.append("Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (cb->get_blocks_data_id_from_block_id(std::stoi(sections->solid_section_data[sub_data_id][1]))==-1)
      {
        log.append("Block ID " + sections->solid_section_data[sub_data_id][1] + " doesn't exist.\n");
        log.append("Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (sections->solid_section_data[sub_data_id][3]!="-1")
      {
        if (!check_orientation_exists(std::stoi(sections->solid_section_data[sub_data_id][3])))
        {
          log.append("Orientation ID " + sections->solid_section_data[sub_data_id][3] + " doesn't exist.\n");
          log.append("Orientation Reference from Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
          sections->solid_section_data[sub_data_id][3]="-1";
          sub_bool_ref = true;
        }
      }
    }
    // SHELL
    if (sections->sections_data[i-1][1] == 2) 
    {
      sub_data_id = sections->get_shell_section_data_id_from_shell_section_id(sections->sections_data[i-1][2]);
      if (!mat->check_material_exists(std::stoi(sections->shell_section_data[sub_data_id][2])))
      {
        log.append("Material " + sections->shell_section_data[sub_data_id][2] + " doesn't exist.\n");
        log.append("Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (cb->get_blocks_data_id_from_block_id(std::stoi(sections->shell_section_data[sub_data_id][1]))==-1)
      {
        log.append("Block ID " + sections->shell_section_data[sub_data_id][1] + " doesn't exist.\n");
        log.append("Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (sections->shell_section_data[sub_data_id][3]!="-1")
      {
        if (!check_orientation_exists(std::stoi(sections->shell_section_data[sub_data_id][3])))
        {
          log.append("Orientation ID " + sections->shell_section_data[sub_data_id][3] + " doesn't exist.\n");
          log.append("Orientation Reference from Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
          sections->shell_section_data[sub_data_id][3]="-1";
          sub_bool_ref = true;
        }
      }
    }
    // BEAM
    if (sections->sections_data[i-1][1] == 3) 
    {
      sub_data_id = sections->get_beam_section_data_id_from_beam_section_id(sections->sections_data[i-1][2]);
      if (!mat->check_material_exists(std::stoi(sections->beam_section_data[sub_data_id][2])))
      {
        log.append("Material " + sections->beam_section_data[sub_data_id][2] + " doesn't exist.\n");
        log.append("Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (cb->get_blocks_data_id_from_block_id(std::stoi(sections->beam_section_data[sub_data_id][1]))==-1)
      {
        log.append("Block ID " + sections->beam_section_data[sub_data_id][1] + " doesn't exist.\n");
        log.append("Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (sections->beam_section_data[sub_data_id][13]!="-1")
      {
        if (!check_orientation_exists(std::stoi(sections->beam_section_data[sub_data_id][13])))
        {
          log.append("Orientation ID " + sections->beam_section_data[sub_data_id][13] + " doesn't exist.\n");
          log.append("Orientation Reference from Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
          sections->beam_section_data[sub_data_id][13]="-1";
          sub_bool_ref = true;
        }
      }
    }
    // MEMBRANE
    if (sections->sections_data[i-1][1] == 4) 
    {
      sub_data_id = sections->get_membrane_section_data_id_from_membrane_section_id(sections->sections_data[i-1][2]);
      if (!mat->check_material_exists(std::stoi(sections->membrane_section_data[sub_data_id][2])))
      {
        log.append("Material " + sections->membrane_section_data[sub_data_id][2] + " doesn't exist.\n");
        log.append("Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (cb->get_blocks_data_id_from_block_id(std::stoi(sections->membrane_section_data[sub_data_id][1]))==-1)
      {
        log.append("Block ID " + sections->membrane_section_data[sub_data_id][1] + " doesn't exist.\n");
        log.append("Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (sections->membrane_section_data[sub_data_id][3]!="-1")
      {
        if (!check_orientation_exists(std::stoi(sections->membrane_section_data[sub_data_id][3])))
        {
          log.append("Orientation ID " + sections->membrane_section_data[sub_data_id][3] + " doesn't exist.\n");
          log.append("Orientation Reference from Section ID " + std::to_string(sections->sections_data[i-1][0]) + " will be deleted.\n");
          sections->membrane_section_data[sub_data_id][3]="-1";
          sub_bool_ref = true;
        }
      }
    }

    if (sub_bool || sub_bool_ref)
    {
      print_log = true;
      if (sub_bool)
      {
        sections->delete_section(sections->sections_data[i-1][0]);
      }
    }
  }

  // CONSTRAINTS  
  for (size_t i = constraints->constraints_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    // RIGID BODY
    if (constraints->constraints_data[i-1][1] == 1)
    {
      sub_data_id = constraints->get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(constraints->constraints_data[i-1][2]);
      
      if (constraints->rigidbody_constraint_data[sub_data_id][1]=="1")
      {
        if (!check_nodeset_exists(std::stoi(constraints->rigidbody_constraint_data[sub_data_id][2])))
        {
          log.append("Nodeset ID " + constraints->rigidbody_constraint_data[sub_data_id][2] + " doesn't exist.\n");
          log.append("Constraint ID " + std::to_string(constraints->constraints_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
        }
      } else if (constraints->rigidbody_constraint_data[sub_data_id][1]=="2")
      {
        if (cb->get_blocks_data_id_from_block_id(std::stoi(constraints->rigidbody_constraint_data[sub_data_id][2]))==-1)
        {
          log.append("Block ID " + constraints->rigidbody_constraint_data[sub_data_id][2] + " doesn't exist.\n");
          log.append("Constraint ID " + std::to_string(constraints->constraints_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
        }
      }
      if (!check_vertex_exists(std::stoi(constraints->rigidbody_constraint_data[sub_data_id][3])))
      {
        log.append("Vertex ID " + constraints->rigidbody_constraint_data[sub_data_id][3] + " doesn't exist.\n");
        log.append("Constraint ID " + std::to_string(constraints->constraints_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (!check_vertex_exists(std::stoi(constraints->rigidbody_constraint_data[sub_data_id][4])))
      {
        log.append("Vertex ID " + constraints->rigidbody_constraint_data[sub_data_id][4] + " doesn't exist.\n");
        log.append("Constraint ID " + std::to_string(constraints->constraints_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    // TIE
    if (constraints->constraints_data[i-1][1] == 2) 
    {
      sub_data_id = constraints->get_tie_constraint_data_id_from_tie_constraint_id(constraints->constraints_data[i-1][2]);

      if (!check_sideset_exists(std::stoi(constraints->tie_constraint_data[sub_data_id][2])))
      {
        log.append("Master: Sideset ID " + constraints->tie_constraint_data[sub_data_id][2] + " doesn't exist.\n");
        log.append("Constraint ID " + std::to_string(constraints->constraints_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (!check_sideset_exists(std::stoi(constraints->tie_constraint_data[sub_data_id][3])))
      {
        log.append("Slave: Sideset ID " + constraints->tie_constraint_data[sub_data_id][3] + " doesn't exist.\n");
        log.append("Constraint ID " + std::to_string(constraints->constraints_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      constraints->delete_constraint(constraints->constraints_data[i-1][0]);
    }
  }

  // CONTACT PAIRS
  for (size_t i = contactpairs->contactpairs_data.size(); i > 0; i--)
  { 
    sub_bool = false;

    if (!check_surfaceinteraction_exists(contactpairs->contactpairs_data[i-1][1]))
    {
      log.append("Surfaceinteraction ID " + std::to_string(contactpairs->contactpairs_data[i-1][1]) + " doesn't exist.\n");
      log.append("Contact Pair ID " + std::to_string(contactpairs->contactpairs_data[i-1][0]) + " will be deleted.\n");
      sub_bool = true;
    }
    sub_data_id = contactpairs->get_adjust_contactpair_data_id_from_adjust_contactpair_id(contactpairs->contactpairs_data[i-1][5]);
    if (contactpairs->adjust_contactpair_data[sub_data_id][2]!="")
    {
      if (!check_nodeset_exists(std::stoi(contactpairs->adjust_contactpair_data[sub_data_id][2])))
      {
        log.append("Nodeset ID " + contactpairs->adjust_contactpair_data[sub_data_id][2] + " doesn't exist.\n");
        log.append("Contact Pair ID " + std::to_string(contactpairs->contactpairs_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (!check_sideset_exists(contactpairs->contactpairs_data[i-1][3]))
    {
      log.append("Master: Sideset ID " + std::to_string(contactpairs->contactpairs_data[i-1][3]) + " doesn't exist.\n");
      log.append("Contact Pair ID " + std::to_string(contactpairs->contactpairs_data[i-1][0]) + " will be deleted.\n");
      sub_bool = true;
    }
    if (!check_sideset_exists(contactpairs->contactpairs_data[i-1][4]))
    {
      log.append("Slave: Sideset ID " + std::to_string(contactpairs->contactpairs_data[i-1][4]) + " doesn't exist.\n");
      log.append("Contact Pair ID " + std::to_string(contactpairs->contactpairs_data[i-1][0]) + " will be deleted.\n");
      sub_bool = true;
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      contactpairs->delete_contactpair(contactpairs->contactpairs_data[i-1][0]);
    }
  }

  // LOADS FORCES
  for (size_t i = loadsforces->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadsforces->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadsforces->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadsforces->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load Force ID " + std::to_string(loadsforces->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsforces->loads_data[i-1][2]=-1;
    }
  }
  // LOADS PRESSURES
  for (size_t i = loadspressures->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadspressures->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadspressures->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadspressures->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load Pressure ID " + std::to_string(loadspressures->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadspressures->loads_data[i-1][2]=-1;
    }
  }
  // LOADS HEATFLUXES
  for (size_t i = loadsheatfluxes->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadsheatfluxes->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadsheatfluxes->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadsheatfluxes->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load Heatflux ID " + std::to_string(loadsheatfluxes->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsheatfluxes->loads_data[i-1][2]=-1;
    }
  }
  // LOADS GRAVITY
  for (size_t i = loadsgravity->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadsgravity->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadsgravity->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadsgravity->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load Gravity ID " + std::to_string(loadsgravity->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsgravity->loads_data[i-1][2]=-1;
    }
    sub_bool = false;
    if (!check_block_exists(loadsgravity->loads_data[i-1][4]))
      {
        log.append("Block ID " + std::to_string(loadsgravity->loads_data[i-1][4]) + " doesn't exist.\n");
        log.append("Gravity ID " + std::to_string(loadsgravity->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsgravity->delete_load(loadsgravity->loads_data[i-1][0]);
    }
  }
  // LOADS CENTRIFUGAL
  for (size_t i = loadscentrifugal->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadscentrifugal->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadscentrifugal->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadscentrifugal->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load Centrigual ID " + std::to_string(loadscentrifugal->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadscentrifugal->loads_data[i-1][2]=-1;
    }
    sub_bool = false;
    if (!check_block_exists(loadscentrifugal->loads_data[i-1][4]))
    {
      log.append("Block ID " + std::to_string(loadscentrifugal->loads_data[i-1][4]) + " doesn't exist.\n");
      log.append("Centrifugal ID " + std::to_string(loadscentrifugal->loads_data[i-1][0]) + " will be deleted.\n");
      sub_bool = true;
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadscentrifugal->delete_load(loadscentrifugal->loads_data[i-1][0]);
    }
  }
  // LOADS TRAJECTORY
  for (size_t i = loadstrajectory->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    // LOADS TRAJECTORY HEATFLUX
    if (loadstrajectory->get_load_type(loadstrajectory->loads_data[i-1][0])=="HEATFLUX")
    {
      int sub_data_id = loadstrajectory->heatflux->get_loads_data_id_from_load_id(loadstrajectory->loads_data[i-1][2]);
      if (!check_curve_exists(loadstrajectory->heatflux->loads_data[sub_data_id][2]))
      {
        log.append("Curve ID " + std::to_string(loadstrajectory->heatflux->loads_data[sub_data_id][2]) + " doesn't exist.\n");
        log.append("Trajectory ID " + std::to_string(loadstrajectory->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      if (!check_vertex_exists(loadstrajectory->heatflux->loads_data[sub_data_id][3]))
      {
        log.append("Vertex ID " + std::to_string(loadstrajectory->heatflux->loads_data[sub_data_id][3]) + " doesn't exist.\n");
        log.append("Trajectory ID " + std::to_string(loadstrajectory->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
      std::vector<int> ids = CubitInterface::parse_cubit_list("vertex", std::to_string(loadstrajectory->heatflux->loads_data[sub_data_id][3]) + " in curve " + std::to_string(loadstrajectory->heatflux->loads_data[sub_data_id][2])); 
      if (ids.size()==0)
      {
        log.append("Vertex ID " + std::to_string(loadstrajectory->heatflux->loads_data[sub_data_id][3]) + " doesn't exist in Curve.\n");
        log.append("Trajectory ID " + std::to_string(loadstrajectory->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }

    if (sub_bool)
    {
      print_log = sub_bool;
      loadstrajectory->delete_load(loadstrajectory->loads_data[i-1][0]);
    }
  }
  // LOADS FILM
  for (size_t i = loadsfilm->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadsfilm->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadsfilm->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadsfilm->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load Film ID " + std::to_string(loadsfilm->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsfilm->loads_data[i-1][2]=-1;
    }
    sub_bool = false;
    if (loadsfilm->loads_data[i-1][7]!=-1)
    {
      if (!check_amplitude_exists(loadsfilm->loads_data[i-1][7]))
      {
        log.append("Film Amplitude ID " + std::to_string(loadsfilm->loads_data[i-1][7]) + " doesn't exist.\n");
        log.append("Film Amplitude Reference from Load Film ID " + std::to_string(loadsfilm->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsfilm->loads_data[i-1][7]=-1;
    }
    sub_bool = false;
    if (!check_sideset_exists(loadsfilm->loads_data[i-1][4]))
      {
        log.append("Sideset ID " + std::to_string(loadsfilm->loads_data[i-1][4]) + " doesn't exist.\n");
        log.append("Film ID " + std::to_string(loadsfilm->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsfilm->delete_load(loadsfilm->loads_data[i-1][0]);
    }
  }
  // RADIATION
  for (size_t i = loadsradiation->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadsradiation->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadsradiation->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadsradiation->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load Radiation ID " + std::to_string(loadsradiation->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsradiation->loads_data[i-1][2]=-1;
    }
    sub_bool = false;
    if (loadsradiation->loads_data[i-1][7]!=-1)
    {
      if (!check_amplitude_exists(loadsradiation->loads_data[i-1][7]))
      {
        log.append("Radiation Amplitude ID " + std::to_string(loadsradiation->loads_data[i-1][7]) + " doesn't exist.\n");
        log.append("Radiation Amplitude Reference from Load Radiation ID " + std::to_string(loadsradiation->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsradiation->loads_data[i-1][7]=-1;
    }
    sub_bool = false;
    if (!check_sideset_exists(loadsradiation->loads_data[i-1][4]))
      {
        log.append("Sideset ID " + std::to_string(loadsradiation->loads_data[i-1][4]) + " doesn't exist.\n");
        log.append("Radiation ID " + std::to_string(loadsradiation->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsradiation->delete_load(loadsradiation->loads_data[i-1][0]);
    }
  }
  // SURFACE TRACTION
  for (size_t i = loadssurfacetraction->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadssurfacetraction->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadssurfacetraction->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadssurfacetraction->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load Surface Traction ID " + std::to_string(loadssurfacetraction->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadssurfacetraction->loads_data[i-1][2]=-1;
    }
    sub_bool = false;
    if (!check_sideset_exists(loadssurfacetraction->loads_data[i-1][4]))
      {
        log.append("Sideset ID " + std::to_string(loadssurfacetraction->loads_data[i-1][4]) + " doesn't exist.\n");
        log.append("Surface Traction ID " + std::to_string(loadssurfacetraction->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadssurfacetraction->delete_load(loadssurfacetraction->loads_data[i-1][0]);
    }
  }
  // BODYHEATFLUX
  for (size_t i = loadsbodyheatflux->loads_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    if (loadsbodyheatflux->loads_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(loadsbodyheatflux->loads_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(loadsbodyheatflux->loads_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from Load BodyHeatflux ID " + std::to_string(loadsbodyheatflux->loads_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsbodyheatflux->loads_data[i-1][2]=-1;
    }
    sub_bool = false;

    if (loadsbodyheatflux->loads_data[i-1][5]==1)
    {
      std::vector<int> elements = loadsbodyheatflux->get_elements_from_element_id(loadsbodyheatflux->loads_data[i-1][4]);
      for (size_t ii = 1; ii < elements.size(); ii++)
      {
        if (!check_block_exists(elements[ii]))
        {
          log.append("Block ID " + std::to_string(elements[ii]) + " doesn't exist.\n");
          log.append("Bodyheatflux ID " + std::to_string(loadsbodyheatflux->loads_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          break;
        }
      }  
    }else if (loadsbodyheatflux->loads_data[i-1][5]==2)
    {
      std::vector<int> elements = loadsbodyheatflux->get_elements_from_element_id(loadsbodyheatflux->loads_data[i-1][4]);
      for (size_t ii = 1; ii < elements.size(); ii++)
      {
        if (!check_global_element_exists(elements[ii]))
        {
          log.append("Global Element ID " + std::to_string(elements[ii]) + " doesn't exist.\n");
          log.append("Check if assigned Elements are part of an existing Block.\n");
          log.append("Bodyheatflux ID " + std::to_string(loadsbodyheatflux->loads_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          break;
        }
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      loadsbodyheatflux->delete_load(loadsbodyheatflux->loads_data[i-1][0]);
    }
  }
  // BCS DISPLACEMENTS
  for (size_t i = bcsdisplacements->bcs_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    
    if (bcsdisplacements->bcs_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(bcsdisplacements->bcs_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(bcsdisplacements->bcs_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from BC Displament ID " + std::to_string(bcsdisplacements->bcs_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      bcsdisplacements->bcs_data[i-1][2]=-1;
    }
  }
  // BCS TEMPERATURES
  for (size_t i = bcstemperatures->bcs_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    
    if (bcstemperatures->bcs_data[i-1][2]!=-1)
    {
      if (!check_amplitude_exists(bcstemperatures->bcs_data[i-1][2]))
      {
        log.append("Amplitude ID " + std::to_string(bcstemperatures->bcs_data[i-1][2]) + " doesn't exist.\n");
        log.append("Amplitude Reference from BC Temperature ID " + std::to_string(bcstemperatures->bcs_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      bcstemperatures->bcs_data[i-1][2]=-1;
    }
  }

  // INITIAL CONDITIONS
  for (size_t i = initialconditions->initialconditions_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    // Displacement
    if (initialconditions->initialconditions_data[i-1][1] == 1)
    {
      sub_data_id = initialconditions->get_displacement_data_id_from_displacement_id(initialconditions->initialconditions_data[i-1][2]);
      if (initialconditions->displacement_data[sub_data_id][1]!="")
      {
        if (!check_bc_exists(std::stoi(initialconditions->displacement_data[sub_data_id][1]),4))
        {
          log.append("BC Displacement ID " + initialconditions->displacement_data[sub_data_id][1] + " doesn't exist.\n");
          log.append("BC Displacement Reference from Initial Condition ID " + std::to_string(initialconditions->initialconditions_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          initialconditions->displacement_data[sub_data_id][1]="";
        }
      }
    }
    // Temperature
    if (initialconditions->initialconditions_data[i-1][1] == 2)
    {
      sub_data_id = initialconditions->get_temperature_data_id_from_temperature_id(initialconditions->initialconditions_data[i-1][2]);
      if (initialconditions->temperature_data[sub_data_id][1]!="")
      {
        if (!check_bc_exists(std::stoi(initialconditions->temperature_data[sub_data_id][1]),5))
        {
          log.append("BC Temperature ID " + initialconditions->temperature_data[sub_data_id][1] + " doesn't exist.\n");
          log.append("BC Temperature Reference from Initial Condition ID " + std::to_string(initialconditions->initialconditions_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          initialconditions->temperature_data[sub_data_id][1]="";
        }
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
    }
  }
  // HISTORY OUTPUTS
  for (size_t i = historyoutputs->outputs_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    // Node
    if (historyoutputs->outputs_data[i-1][2] == 1)
    {
      sub_data_id = historyoutputs->get_node_data_id_from_node_id(historyoutputs->outputs_data[i-1][3]);
      if (historyoutputs->node_data[sub_data_id][1]!="")
      {
        if (!check_nodeset_exists(std::stoi(historyoutputs->node_data[sub_data_id][1])))
        {
          log.append("Nodeset ID " + historyoutputs->node_data[sub_data_id][1] + " doesn't exist.\n");
          log.append("Nodeset Reference from History Output ID " + std::to_string(historyoutputs->outputs_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          historyoutputs->node_data[sub_data_id][1] = "";
        }
      }
    }
    // Element
    if (historyoutputs->outputs_data[i-1][2] == 2)
    {
      sub_data_id = historyoutputs->get_element_data_id_from_element_id(historyoutputs->outputs_data[i-1][3]);
      if (historyoutputs->element_data[sub_data_id][1]!="")
      {
        if (!check_block_exists(std::stoi(historyoutputs->element_data[sub_data_id][1])))
        {
          log.append("Block ID " + historyoutputs->element_data[sub_data_id][1] + " doesn't exist.\n");
          log.append("Block Reference from History Output ID " + std::to_string(historyoutputs->outputs_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          historyoutputs->element_data[sub_data_id][1] = "";
        }
      }
    }
    // Contact
    if (historyoutputs->outputs_data[i-1][2] == 3)
    {
      sub_data_id = historyoutputs->get_contact_data_id_from_contact_id(historyoutputs->outputs_data[i-1][3]);
      if (historyoutputs->contact_data[sub_data_id][1]!="")
      {
        if (!check_contactpair_exists(std::stoi(historyoutputs->contact_data[sub_data_id][1])))
        {
          log.append("Contact Pair ID " + historyoutputs->contact_data[sub_data_id][1] + " doesn't exist.\n");
          log.append("Contact Pair Reference from History Output ID " + std::to_string(historyoutputs->outputs_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          historyoutputs->contact_data[sub_data_id][1] = "";
        }
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
    }
  }
  // FIELD OUTPUTS
  for (size_t i = fieldoutputs->outputs_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    // Node
    if (fieldoutputs->outputs_data[i-1][2] == 1)
    {
      sub_data_id = fieldoutputs->get_node_data_id_from_node_id(fieldoutputs->outputs_data[i-1][3]);
      if (fieldoutputs->node_data[sub_data_id][1]!="")
      {
        if (!check_nodeset_exists(std::stoi(fieldoutputs->node_data[sub_data_id][1])))
        {
          log.append("Nodeset ID " + fieldoutputs->node_data[sub_data_id][1] + " doesn't exist.\n");
          log.append("Nodeset Reference from Field Output ID " + std::to_string(fieldoutputs->outputs_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          fieldoutputs->node_data[sub_data_id][1] = "";
        }
      }
    }
    // Element
    if (fieldoutputs->outputs_data[i-1][2] == 2)
    {
      sub_data_id = fieldoutputs->get_element_data_id_from_element_id(fieldoutputs->outputs_data[i-1][3]);
      if (fieldoutputs->element_data[sub_data_id][1]!="")
      {
        if (!check_block_exists(std::stoi(fieldoutputs->element_data[sub_data_id][1])))
        {
          log.append("Block ID " + fieldoutputs->element_data[sub_data_id][1] + " doesn't exist.\n");
          log.append("Block Reference from Field Output ID " + std::to_string(fieldoutputs->outputs_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          fieldoutputs->element_data[sub_data_id][1] = "";
        }
      }
    }
    if (sub_bool)
    {
      print_log = sub_bool;
    }
  }
  // HBCS
  // Displacement 
  sub_data_ids = hbcs->get_bc_data_ids_from_bcs_id(0);
  for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
  {
    if (hbcs->bcs_data[sub_data_ids[ii-1]][1]==1)
    {
      if (!check_bc_exists(hbcs->bcs_data[sub_data_ids[ii-1]][2],4))
      {
        log.append("BC Displacement ID " + std::to_string(hbcs->bcs_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
        log.append("BC Displacement Reference from HBC will be deleted.\n");
        sub_bool = true;
        hbcs->remove_bcs(0, 1, {hbcs->bcs_data[sub_data_ids[ii-1]][2]});
      }
    }
  }
  // Temperature
  sub_data_ids = hbcs->get_bc_data_ids_from_bcs_id(0);
  for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
  {
    if (hbcs->bcs_data[sub_data_ids[ii-1]][1]==2)
    {
      if (!check_bc_exists(hbcs->bcs_data[sub_data_ids[ii-1]][2],5))
      {
        log.append("BC Temperature ID " + std::to_string(hbcs->bcs_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
        log.append("BC Temperature Reference from HBC will be deleted.\n");
        sub_bool = true;
        hbcs->remove_bcs(0, 2, {hbcs->bcs_data[sub_data_ids[ii-1]][2]});
      }
    }
  }
  // STEPS
  for (size_t i = steps->steps_data.size(); i > 0; i--)
  { 
    sub_bool = false;
    // STEP LOADS
    // Force 
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==1)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],6))
        {
          log.append("Load Force ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Force Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 1, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Pressure
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==2)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],7))
        {
          log.append("Load Pressure ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Pressure Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 2, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Heatflux
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==3)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],8))
        {
          log.append("Load Heatflux ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Heatflux Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 3, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Gravity
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==4)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],9))
        {
          log.append("Load Gravity ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Gravity Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 4, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Centrifugal
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==5)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],10))
        {
          log.append("Load Centrifugal ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Centrifugal Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 5, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Trajectory
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==6)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],11))
        {
          log.append("Load Trajectory ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Trajectory Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 6, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Film
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==7)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],12))
        {
          log.append("Load Film ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Film Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 7, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Radiation
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==8)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],13))
        {
          log.append("Load Radiation ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Radiation Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 8, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Surface Traction
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==9)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],14))
        {
          log.append("Load Surface Traction ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load Surface Traction Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 9, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // BodyHeatflux
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->loads_data[sub_data_ids[ii-1]][1]==10)
      {
        if (!check_bc_exists(steps->loads_data[sub_data_ids[ii-1]][2],15))
        {
          log.append("Load BodyHeatflux ID " + std::to_string(steps->loads_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("Load BodyHeatflux Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_loads(steps->steps_data[i-1][0], 10, {steps->loads_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // STEP BCS
    // Displacement 
    sub_data_ids = steps->get_bc_data_ids_from_bcs_id(steps->steps_data[i-1][6]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    { 
      if (steps->bcs_data[sub_data_ids[ii-1]][1]==1)
      {
        if (!check_bc_exists(steps->bcs_data[sub_data_ids[ii-1]][2],4))
        {
          log.append("BC Displacement ID " + std::to_string(steps->bcs_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("BC Displacement Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_bcs(steps->steps_data[i-1][0], 1, {steps->bcs_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // Temperature
    sub_data_ids = steps->get_bc_data_ids_from_bcs_id(steps->steps_data[i-1][6]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (steps->bcs_data[sub_data_ids[ii-1]][1]==2)
      {
        if (!check_bc_exists(steps->bcs_data[sub_data_ids[ii-1]][2],5))
        {
          log.append("BC Temperature ID " + std::to_string(steps->bcs_data[sub_data_ids[ii-1]][2]) + " doesn't exist.\n");
          log.append("BC Temperature Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
          sub_bool = true;
          steps->remove_bcs(steps->steps_data[i-1][0], 2, {steps->bcs_data[sub_data_ids[ii-1]][2]});
        }
      }
    }
    // STEPS HISTORY OUTPUTS
    sub_data_ids = steps->get_historyoutput_data_ids_from_historyoutputs_id(steps->steps_data[i-1][7]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (!check_historyoutput_exists(steps->historyoutputs_data[sub_data_ids[ii-1]][1]))
      {
        log.append("History Output ID " + std::to_string(steps->historyoutputs_data[sub_data_ids[ii-1]][1]) + " doesn't exist.\n");
        log.append("History Output Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
        steps->remove_historyoutputs(steps->steps_data[i-1][0], {steps->historyoutputs_data[sub_data_ids[ii-1]][1]});
      }
    }
    // STEPS FIELD OUTPUTS
    sub_data_ids = steps->get_fieldoutput_data_ids_from_fieldoutputs_id(steps->steps_data[i-1][8]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      if (!check_fieldoutput_exists(steps->fieldoutputs_data[sub_data_ids[ii-1]][1]))
      {
        log.append("Field Output ID " + std::to_string(steps->fieldoutputs_data[sub_data_ids[ii-1]][1]) + " doesn't exist.\n");
        log.append("Field Output Reference from Step ID " + std::to_string(steps->steps_data[i-1][0]) + " will be deleted.\n");
        sub_bool = true;
        steps->remove_fieldoutputs(steps->steps_data[i-1][0], {steps->fieldoutputs_data[sub_data_ids[ii-1]][1]});
      }
    }

    if (sub_bool)
    {
      print_log = sub_bool;
    }
  }
  
  if (print_log)
  {
    PRINT_INFO("%s", log.c_str());
    return log;
  }
  return "";
}

std::string CalculiXCore::print_data()
{
  std::string str_return = "";
  str_return.append(cb->print_data());
  str_return.append(sections->print_data());
  str_return.append(constraints->print_data());
  str_return.append(surfaceinteractions->print_data());
  str_return.append(contactpairs->print_data());
  str_return.append(amplitudes->print_data());
  str_return.append(orientations->print_data());
  str_return.append(damping->print_data());
  str_return.append(physicalconstants->print_data());
  str_return.append(loadsforces->print_data());
  str_return.append(loadspressures->print_data());
  str_return.append(loadsheatfluxes->print_data());
  str_return.append(loadsgravity->print_data());
  str_return.append(loadscentrifugal->print_data());
  str_return.append(loadstrajectory->print_data());
  str_return.append(loadsfilm->print_data());
  str_return.append(loadsradiation->print_data());
  str_return.append(loadssurfacetraction->print_data());
  str_return.append(loadsbodyheatflux->print_data());
  str_return.append(bcsdisplacements->print_data());
  str_return.append(bcstemperatures->print_data());
  str_return.append(historyoutputs->print_data());
  str_return.append(fieldoutputs->print_data());
  str_return.append(initialconditions->print_data());
  str_return.append(hbcs->print_data());
  str_return.append(steps->print_data());
  str_return.append(jobs->print_data());
  str_return.append(results->print_data());
  str_return.append(customlines->print_data());

  return str_return;
}

bool CalculiXCore::export_to_csv(std::string path, std::vector<std::string> header, std::vector<std::vector<double>> data,bool overwrite)
{
  std::string filename = path;
  
  // check if file already exists
  if (!overwrite)
  {
    #ifdef WIN32
      if (_access(filename.c_str(), 0) == 0)
      {
        PRINT_ERROR("Output File already exists!\n");
        return false;
      }
    #else
      if (access(filename.c_str(), F_OK) == 0)
      {
        PRINT_ERROR("Output File already exists!\n");
        return false;
      }
    #endif
  }

  std::ofstream file(filename);
  if(file.is_open())
  {
    //header
    for (size_t i = 0; i < header.size(); ++i)
    {
      file << header[i];
      if (i < header.size() - 1)
      {
        file << ","; // add a period between header elements
      }
    }
    file << "\n";

    //data
    for (size_t i = 0; i < data.size(); ++i)
    {
      for (size_t ii = 0; ii < data[i].size(); ++ii)
      {
        file << data[i][ii];
        if (ii < data[i].size() - 1)
        {
          file << ","; // add a period between elements in the row
        }
      }
      file << "\n";
    }
    file.close();
  }

  return true;
}

std::string  CalculiXCore::to_string_scientific(double value, int precision)
{
  return ScientificNumber::format(value, precision);
}

double  CalculiXCore::string_scientific_to_double(std::string value, int precision)
{
  double output;
  if (!value.empty() && value[0] == '-') // check if negative
  {
    value.replace(0,1, "");
    //check if is INF
    if (value=="INF")
    {
      output = -1.255070e+29;
    }else{
      output = ScientificNumber::parse(value);
      output = -1*output;
    }
  } else {
    //check if is INF
    if (value=="INF")
    {
      output = 1.255070e+29;
    }else{
      output = ScientificNumber::parse(value);
    }
  }
  //apply precision
  output = ScientificNumber::parse(this->to_string_scientific(output,precision));

  return output;
}

std::vector<std::string> CalculiXCore::get_ccx_element_types()
{ 
  return cb->get_ccx_element_types();
}

bool CalculiXCore::set_ccx_element_type(int block_id, std::string ccx_element_type)
{ 
  int new_element_type_ccx;
  new_element_type_ccx = cb->get_ccx_element_type_id(ccx_element_type);

  if (cb->modify_block_element_type_ccx(block_id, new_element_type_ccx))
  {
    return true;
  } else {
    return false;
  }
}

std::string CalculiXCore::get_ccx_element_type(int block_id)
{
  return cb->get_ccx_element_type_name(block_id);
}

int CalculiXCore::get_ccx_element_type_integration_points(int block_id)
{
  return cb->get_ccx_element_type_integration_points(block_id);
}

std::string CalculiXCore::get_cubit_element_type_entity(std::string cubit_element_type)
{
  return cb->get_cubit_element_type_entity_name(cubit_element_type);
}

std::vector<std::vector<int>> CalculiXCore::get_element_id_type_connectivity()
{
  std::vector<std::vector<int>> id_type_connectivity;
  std::vector<int> tmp_id_type_connectivity;
  // Get Block id's
  std::vector<int> blocks;
  blocks = this->get_blocks();

  // Get a batch of elements in an initialized block
  int buf_size = 100;
  std::vector<ElementType>   element_type(buf_size);
  std::vector<ElementHandle> handles(buf_size);

  // define name variable
  std::string element_type_name;
  std::string cubit_element_type_entity;
  std::string block_name;

  // Elements in a buffer set will be of the same element type and in the same block
  for (size_t i = 0; i < blocks.size(); i++)
  {
    int num_elems;
    int start_index = 0;
    //BlockHandle block = blocks[i];
    BlockHandle block;
    me_iface->get_block_handle(blocks[i], block);

    // write only once per block
    int block_id = blocks[i];
  
    while( (num_elems = me_iface->get_block_elements(start_index, buf_size, block, element_type, handles)) > 0)
    {
      // Get ids for the element handles
      std::vector<int> ids(num_elems);
      me_iface->get_element_ids(num_elems, handles, ids);

      // convert ids from element_id to global_element_id
      cubit_element_type_entity = me_iface->get_element_type_name(element_type[0]);
      cubit_element_type_entity = this->get_cubit_element_type_entity(cubit_element_type_entity);

      //rewrite to global element ids
      for (size_t ii = 0; ii < ids.size(); ii++)
      {
        ids[ii] = CubitInterface::get_global_element_id(cubit_element_type_entity,ids[ii]);
        
        tmp_id_type_connectivity.clear();
        tmp_id_type_connectivity.push_back(ids[ii]);
        tmp_id_type_connectivity.push_back(element_type[ii]);
        int num_nodes;
        me_iface->get_nodes_per_element(element_type[ii],num_nodes);
        std::vector<int> conn(num_nodes);
        me_iface->get_connectivity(handles[ii],conn);
        for (size_t iii = 0; iii < conn.size(); iii++)
        {
          tmp_id_type_connectivity.push_back(conn[iii]);
        }
        id_type_connectivity.push_back(tmp_id_type_connectivity);
      }
      start_index += num_elems;
    }
  }
  return id_type_connectivity;
}

std::string CalculiXCore::get_block_name(int block_id)
{
  std::string block_name;

  BlockHandle block;
  if (!me_iface->get_block_handle(block_id, block))
  {
    block_name = "BLOCK " + std::to_string(block_id) + " doesn't exist!";
    return block_name;
  }
  block_name = me_iface->name_from_handle(block);
    
  if (block_name == "")
  {
    block_name = "Block_" + std::to_string(block_id);
  }
  return block_name;
}

std::vector<int> CalculiXCore::get_block_node_ids(int block_id)
{
  
  std::vector< int > returned_node_list;
  std::vector< int > returned_sphere_list;
  std::vector< int > returned_edge_list;
  std::vector< int > returned_tri_list;
  std::vector< int > returned_face_list;
  std::vector< int > returned_pyramid_list;
  std::vector< int > returned_wedge_list;
  std::vector< int > returned_tet_list;
  std::vector< int > returned_hex_list;

  CubitInterface::get_block_elements_and_nodes(block_id,
		returned_node_list,
		returned_sphere_list,
		returned_edge_list,
		returned_tri_list,
		returned_face_list,
		returned_pyramid_list,
		returned_wedge_list,
		returned_tet_list,
		returned_hex_list 
	); 	
  
/*
  std::vector< int > returned_group_list;
  std::vector< int > returned_volume_list;
  std::vector< int > returned_surface_list;
  std::vector< int > returned_curve_list; 
  std::vector< int > returned_vertex_list;

  CubitInterface::get_block_children(block_id,
   returned_group_list,
   returned_node_list,
   returned_sphere_list,
   returned_edge_list,
   returned_tri_list,
   returned_face_list,
   returned_pyramid_list,
   returned_tet_list,
   returned_hex_list,
   returned_wedge_list,
   returned_volume_list,
   returned_surface_list,
   returned_curve_list,
   returned_vertex_list
  );
*/
  return returned_node_list;
}

std::vector<int> CalculiXCore::get_block_element_ids(int block_id)
{
  std::string current_element_type = ""; // to detect changes of element type in blocks. especially hex and wedge
  std::vector<int> return_ids;

  // Get a batch of elements in an initialized block
  int buf_size = 100;
  std::vector<ElementType>   element_type(buf_size);
  std::vector<ElementHandle> handles(buf_size);

  // define name variable
  std::string element_type_name;
  std::string cubit_element_type_entity;

  // Elements in a buffer set will be of the same element type and in the same block
  
  int num_elems;
  int start_index = 0;
  BlockHandle block;
  me_iface->get_block_handle(block_id, block);
  current_element_type = CubitInterface::get_block_element_type(block_id);

  if (current_element_type != "SPHERE") // check if cubit element type from block is sphere
  {         
    // write only once per block
    element_type_name = this->get_ccx_element_type(block_id);

    while( (num_elems = me_iface->get_block_elements(start_index, buf_size, block, element_type, handles)) > 0)
    {
      // Get ids for the element handles
      std::vector<int> ids(num_elems);
      me_iface->get_element_ids(num_elems, handles, ids);

      // skip if element type is 0 (SPHERE), that element type doesn´t exist in CalculiX
      // skip if block element type is different from requested elements, hex and wedge mix
      if ((element_type[0] != 0) && (current_element_type==me_iface->get_element_type_name(element_type[0])))
      {
        // convert ids from element_id to global_element_id
        cubit_element_type_entity = me_iface->get_element_type_name(element_type[0]);
        cubit_element_type_entity = this->get_cubit_element_type_entity(cubit_element_type_entity);
        
        //rewrite to global element ids
        for (size_t i = 0; i < ids.size(); i++)
        {
          ids[i] = CubitInterface::get_global_element_id(cubit_element_type_entity,ids[i]);
          return_ids.push_back(ids[i]);
        }

      }
      start_index += num_elems;
    }
  }
  return return_ids;
}

std::vector<double> CalculiXCore::get_block_bounding_box_center(int block_id)
{
  std::vector< int > returned_node_list;
  std::vector< int > returned_sphere_list;
  std::vector< int > returned_edge_list;
  std::vector< int > returned_tri_list;
  std::vector< int > returned_face_list;
  std::vector< int > returned_pyramid_list;
  std::vector< int > returned_wedge_list;
  std::vector< int > returned_tet_list;
  std::vector< int > returned_hex_list;
  std::vector< int > returned_group_list;
  std::vector< int > returned_volume_list;
  std::vector< int > returned_surface_list;
  std::vector< int > returned_curve_list; 
  std::vector< int > returned_vertex_list;
  std::vector<double> coord;

  CubitInterface::get_block_children(block_id,
   returned_group_list,
   returned_node_list,
   returned_sphere_list,
   returned_edge_list,
   returned_tri_list,
   returned_face_list,
   returned_pyramid_list,
   returned_tet_list,
   returned_hex_list,
   returned_wedge_list,
   returned_volume_list,
   returned_surface_list,
   returned_curve_list,
   returned_vertex_list
  );

  std::array<double,10> vector_list;
  std::vector<int> ids = CubitInterface::parse_cubit_list("vertex", "all");
  if (returned_volume_list.size()!=0)
  {
    vector_list = CubitInterface::get_total_bounding_box("volume", returned_volume_list);
    coord.push_back((vector_list[0]+vector_list[1])/2);
    coord.push_back((vector_list[3]+vector_list[4])/2);
    coord.push_back((vector_list[6]+vector_list[7])/2);
  }else if (returned_surface_list.size()!=0)
  {
    vector_list = CubitInterface::get_total_bounding_box("surface", returned_surface_list);
    coord.push_back((vector_list[0]+vector_list[1])/2);
    coord.push_back((vector_list[3]+vector_list[4])/2);
    coord.push_back((vector_list[6]+vector_list[7])/2);
  }else if (returned_curve_list.size()!=0)
  {
    vector_list = CubitInterface::get_total_bounding_box("curve", returned_curve_list);
    coord.push_back((vector_list[0]+vector_list[1])/2);
    coord.push_back((vector_list[3]+vector_list[4])/2);
    coord.push_back((vector_list[6]+vector_list[7])/2);
  }else if (returned_vertex_list.size()!=0)
  {
    vector_list = CubitInterface::get_total_bounding_box("vertex", returned_vertex_list);
    coord.push_back((vector_list[0]+vector_list[1])/2);
    coord.push_back((vector_list[3]+vector_list[4])/2);
    coord.push_back((vector_list[6]+vector_list[7])/2);
  }
  
  return coord;
}

std::string CalculiXCore::get_material_name(int material_id)
{
  std::string material_name;
  MaterialInterface::Material material;
  material = mat_iface->get_material(material_id);
  material_name = mat_iface->get_material_name(material);
  
  return material_name;
}

std::string CalculiXCore::get_material_prefix(std::string material_name)
{
  return mat->get_material_prefix(material_name);
}

int CalculiXCore::get_group_property_size(std::string group_property)
{
  return mat->get_group_property_size(group_property);
}

std::string CalculiXCore::get_nodeset_name(int nodeset_id)
{
  std::string nodeset_name;

  NodesetHandle nodeset;
  if (!me_iface->get_nodeset_handle(nodeset_id, nodeset))
  {
    nodeset_name = "NODESET " + std::to_string(nodeset_id) + " doesn't exist!";
    return nodeset_name;
  }
  nodeset_name = me_iface->get_nodeset_name(nodeset);

  // nodesetname or id
  if (nodeset_name == "")
  {
    nodeset_name = "Nodeset_" + std::to_string(nodeset_id);
  }
  return nodeset_name;
}

std::vector<std::vector<double>> CalculiXCore::get_nodeset_entities_coords(int nodeset_id)
{
  std::vector<std::vector<double>> tmp;

  std::vector< int > returned_node_list;
	std::vector< int > returned_volume_list;
	std::vector< int > returned_surface_list;
	std::vector< int > returned_curve_list;
	std::vector< int > returned_vertex_list;

  CubitInterface::get_nodeset_children(nodeset_id,returned_node_list,returned_volume_list,returned_surface_list,returned_curve_list,returned_vertex_list);

  for (size_t i = 0; i < returned_node_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_nodal_coordinates(returned_node_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }
  
  for (size_t i = 0; i < returned_volume_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("volume",returned_volume_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  for (size_t i = 0; i < returned_surface_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("surface",returned_surface_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  for (size_t i = 0; i < returned_curve_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("curve",returned_curve_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  for (size_t i = 0; i < returned_vertex_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("vertex",returned_vertex_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  
  return tmp;
}

std::vector<std::vector<double>> CalculiXCore::get_sideset_entities_coords(int sideset_id)
{
  std::vector<std::vector<double>> tmp;

  std::vector< int > returned_face_list;
	std::vector< int > returned_surface_list;
	std::vector< int > returned_curve_list;

  CubitInterface::get_sideset_children(sideset_id,returned_face_list,returned_surface_list,returned_curve_list);

  for (size_t i = 0; i < returned_face_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("face",returned_face_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }
  
  for (size_t i = 0; i < returned_surface_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("surface",returned_surface_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  for (size_t i = 0; i < returned_curve_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("curve",returned_curve_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2]});
  }

  return tmp;
}

std::vector<std::vector<double>> CalculiXCore::get_sideset_entities_coords_normals(int sideset_id)
{
  std::vector<std::vector<double>> tmp;

  std::vector< int > returned_face_list;
	std::vector< int > returned_surface_list;
	std::vector< int > returned_curve_list;

  CubitInterface::get_sideset_children(sideset_id,returned_face_list,returned_surface_list,returned_curve_list);

  /*
  for (size_t i = 0; i < returned_face_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("face",returned_face_list[i]);
    std::array<double,3> normal = CubitInterface::get_surface_normal_at_coord(-1,coord);
    if (!((normal[0]==0)&&(normal[1]==0)&&(normal[2]==0)))
    {
      tmp.push_back({coord[0],coord[1],coord[2],normal[0],normal[1],normal[2]});
    }
  }
  */
  
  for (size_t i = 0; i < returned_surface_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("surface",returned_surface_list[i]);
    std::array<double,3> normal = CubitInterface::get_surface_normal(returned_surface_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2],normal[0],normal[1],normal[2]});
  }

  /*
  for (size_t i = 0; i < returned_curve_list.size(); i++)
  {
    std::array<double,3> coord = CubitInterface::get_center_point("curve",returned_curve_list[i]);
    std::array<double,3> normal = CubitInterface::get_surface_normal(returned_surface_list[i]);
    tmp.push_back({coord[0],coord[1],coord[2],normal[0],normal[1],normal[2]});
  }
  */
  
  return tmp;
}

std::string CalculiXCore::get_sideset_name(int sideset_id)
{
  std::string sideset_name;

  SidesetHandle sideset;
  if (!me_iface->get_sideset_handle(sideset_id, sideset))
  {
    sideset_name = "SIDESET " + std::to_string(sideset_id) + " doesn't exist!";
    return sideset_name;
  }
  sideset_name = me_iface->get_sideset_name(sideset);

  // sidesetname or id
  if (sideset_name == "")
  {
    sideset_name = "Surface_" + std::to_string(sideset_id);
  }
  return sideset_name;
}

std::string CalculiXCore::get_surfaceinteraction_name(int surfaceinteraction_id)
{
  std::string surfaceinteraction_name;
  int surfaceinteraction_data_id;
  int surfaceinteraction_name_id;
  int surfaceinteraction_name_data_id;

  surfaceinteraction_data_id = surfaceinteractions->get_surfaceinteractions_data_id_from_surfaceinteraction_id(surfaceinteraction_id);

  if (surfaceinteraction_data_id != -1)
  {
    surfaceinteraction_name_id = surfaceinteractions->surfaceinteractions_data[surfaceinteraction_data_id][1];
    surfaceinteraction_name_data_id = surfaceinteractions->get_surfaceinteraction_name_data_id_from_surfaceinteraction_name_id(surfaceinteraction_name_id);
    surfaceinteraction_name = surfaceinteractions->surfaceinteraction_name_data[surfaceinteraction_name_data_id][1];
  }else{
    surfaceinteraction_name = "SURFACEINTERACTION " + std::to_string(surfaceinteraction_id) + " doesn't exist!";
  }

  return surfaceinteraction_name;
}

std::vector<std::string> CalculiXCore::get_contactpair_master_slave(int contactpair_id)
{
  std::vector<std::string> contactpair(2);
  int contactpair_data_id;
  
  contactpair_data_id = contactpairs->get_contactpairs_data_id_from_contactpair_id(contactpair_id);
  if (contactpair_data_id!=-1)
  {    
    contactpair[0] = this->get_sideset_name(contactpairs->contactpairs_data[contactpair_data_id][3]);
    contactpair[1] = this->get_sideset_name(contactpairs->contactpairs_data[contactpair_data_id][4]);
  } else {
    contactpair[0] = "CONTACTPAIR " + std::to_string(contactpair_id) + "DOESN'T EXIST!";
    contactpair[1] = "CONTACTPAIR " + std::to_string(contactpair_id) + "DOESN'T EXIST!";
  }
  return contactpair;
}

std::string CalculiXCore::get_amplitude_name(int amplitude_id)
{
  std::string amplitude_name="";
  int amplitude_data_id;
  int amplitude_name_id;
  int amplitude_name_data_id;

  amplitude_data_id = amplitudes->get_amplitudes_data_id_from_amplitude_id(amplitude_id);

  if (amplitude_id != -1)
  {
    amplitude_name_id = amplitudes->amplitudes_data[amplitude_data_id][1];
    amplitude_name_data_id = amplitudes->get_name_amplitude_data_id_from_name_amplitude_id(amplitude_name_id);
    amplitude_name = amplitudes->name_amplitude_data[amplitude_name_data_id][1];
  }

  return amplitude_name;
}

std::string CalculiXCore::get_orientation_name(int orientation_id)
{
  std::string name="";
  int orientation_data_id;
  int name_id;
  int name_data_id;

  orientation_data_id = orientations->get_orientations_data_id_from_orientation_id(orientation_id);

  if (orientation_id != -1)
  {
    name_id = orientations->orientations_data[orientation_data_id][1];
    name_data_id = orientations->get_name_data_id_from_name_id(name_id);
    name = orientations->name_data[name_data_id][1];
  }

  return name;
}

std::vector<int> CalculiXCore::get_loadsforces_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadsforces->loads_data.size(); i++)
  {
    tmp.push_back(loadsforces->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadspressures_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadspressures->loads_data.size(); i++)
  {
    tmp.push_back(loadspressures->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadsheatfluxes_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadsheatfluxes->loads_data.size(); i++)
  {
    tmp.push_back(loadsheatfluxes->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadsgravity_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadsgravity->loads_data.size(); i++)
  {
    tmp.push_back(loadsgravity->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadscentrifugal_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadscentrifugal->loads_data.size(); i++)
  {
    tmp.push_back(loadscentrifugal->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadstrajectory_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadstrajectory->loads_data.size(); i++)
  {
    tmp.push_back(loadstrajectory->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadsfilm_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadsfilm->loads_data.size(); i++)
  {
    tmp.push_back(loadsfilm->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadsradiation_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadsradiation->loads_data.size(); i++)
  {
    tmp.push_back(loadsradiation->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadssurfacetraction_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadssurfacetraction->loads_data.size(); i++)
  {
    tmp.push_back(loadssurfacetraction->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_loadsbodyheatflux_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < loadsbodyheatflux->loads_data.size(); i++)
  {
    tmp.push_back(loadsbodyheatflux->loads_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_bcsdisplacements_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < bcsdisplacements->bcs_data.size(); i++)
  {
    tmp.push_back(bcsdisplacements->bcs_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_orientations_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < orientations->orientations_data.size(); i++)
  {
    tmp.push_back(orientations->orientations_data[i][0]);
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_equation_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < constraints->constraints_data.size(); i++)
  {
    if (constraints->constraints_data[i][1]==3)
    {
      tmp.push_back(constraints->constraints_data[i][0]);
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_equationgroup_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < constraints->constraints_data.size(); i++)
  {
    if (constraints->constraints_data[i][1]==4)
    {
      tmp.push_back(constraints->constraints_data[i][0]);
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_customline_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < customlines->customlines_data.size(); i++)
  {
    tmp.push_back(std::stoi(customlines->customlines_data[i][0]));
  }
  return tmp;
}

std::vector<int> CalculiXCore::get_bcstemperatures_ids()
{
  std::vector<int> tmp;
  for (size_t i = 0; i < bcstemperatures->bcs_data.size(); i++)
  {
    tmp.push_back(bcstemperatures->bcs_data[i][0]);
  }
  return tmp;
}

bool CalculiXCore::check_block_exists(int block_id)
{
  int block_data_id;
  block_data_id = cb->get_blocks_data_id_from_block_id(block_id);
  if (block_data_id == -1)
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_global_element_exists(int element_id)
{
  return CubitInterface::get_element_exists(element_id);
}

bool CalculiXCore::check_material_exists(int material_id)
{
  return mat->check_material_exists(material_id);
}

bool CalculiXCore::check_bc_exists(int bc_id,int BCType)
{
  std::vector<int> ids;

  if (BCType == 4)
  {
    ids = CubitInterface::get_bc_id_list(CI_BCTYPE_DISPLACEMENT);
  }else if (BCType == 5)
  {
    ids = CubitInterface::get_bc_id_list(CI_BCTYPE_TEMPERATURE);
  }else if (BCType == 6)
  {
    ids = CubitInterface::get_bc_id_list(CI_BCTYPE_FORCE);
  }else if (BCType == 7)
  {
    ids = CubitInterface::get_bc_id_list(CI_BCTYPE_PRESSURE);
  }else if (BCType == 8) // heaftlux
  {
    ids = CubitInterface::get_bc_id_list(CI_BCTYPE_HEATFLUX);
  }else if (BCType == 9) // gravity
  {
    for (size_t i = 0; i < loadsgravity->loads_data.size(); i++)
    {
      ids.push_back(loadsgravity->loads_data[i][0]);
    }
  }else if (BCType == 10) // centrifugal
  {
    for (size_t i = 0; i < loadscentrifugal->loads_data.size(); i++)
    {
      ids.push_back(loadscentrifugal->loads_data[i][0]);
    }
  }else if (BCType == 11) // Trajectory
  {
    for (size_t i = 0; i < loadstrajectory->loads_data.size(); i++)
    {
      ids.push_back(loadstrajectory->loads_data[i][0]);
    }
  }else if (BCType == 12) // Film
  {
    for (size_t i = 0; i < loadsfilm->loads_data.size(); i++)
    {
      ids.push_back(loadsfilm->loads_data[i][0]);
    }
  }else if (BCType == 13) // Radiation
  {
    for (size_t i = 0; i < loadsradiation->loads_data.size(); i++)
    {
      ids.push_back(loadsradiation->loads_data[i][0]);
    }
  }else if (BCType == 14) // Surface Traction
  {
    for (size_t i = 0; i < loadssurfacetraction->loads_data.size(); i++)
    {
      ids.push_back(loadssurfacetraction->loads_data[i][0]);
    }
  }else if (BCType == 15) // BodyHeatflux
  {
    for (size_t i = 0; i < loadsbodyheatflux->loads_data.size(); i++)
    {
      ids.push_back(loadsbodyheatflux->loads_data[i][0]);
    }
  }
  
  for (size_t i = 0; i < ids.size(); i++)
  {
    if (ids[i]==bc_id)
    {
      return true;
    } 
  }
  return false;
}

bool CalculiXCore::check_nodeset_exists(int nodeset_id)
{
  NodesetHandle nodeset;
  if (!me_iface->get_nodeset_handle(nodeset_id, nodeset))
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_sideset_exists(int sideset_id)
{
  SidesetHandle sideset;
  if (!me_iface->get_sideset_handle(sideset_id, sideset))
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_vertex_exists(int vertex_id)
{
  std::vector<int> vertex_list = CubitInterface::parse_cubit_list("vertex",std::to_string(vertex_id));
  
  if (vertex_list.size()==1)
  {
    return true;
  }

  return false;
}

bool CalculiXCore::check_curve_exists(int curve_id)
{
  std::vector<int> curve_list = CubitInterface::parse_cubit_list("curve",std::to_string(curve_id));
  
  if (curve_list.size()==1)
  {
    return true;
  }

  return false;
}

bool CalculiXCore::check_surface_exists(int surface_id)
{
  std::vector<int> surface_list = CubitInterface::parse_cubit_list("surface",std::to_string(surface_id));
  
  if (surface_list.size()==1)
  {
    return true;
  }

  return false;
}

bool CalculiXCore::check_surfaceinteraction_exists(int surfaceinteraction_id)
{
  int surfaceinteraction_data_id;
  surfaceinteraction_data_id = surfaceinteractions->get_surfaceinteractions_data_id_from_surfaceinteraction_id(surfaceinteraction_id);
  if (surfaceinteraction_data_id == -1)
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_contactpair_exists(int contactpair_id)
{
  int contactpair_data_id;
  contactpair_data_id = contactpairs->get_contactpairs_data_id_from_contactpair_id(contactpair_id);
  if (contactpair_data_id == -1)
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_amplitude_exists(int amplitude_id)
{
  int amplitude_data_id;
  amplitude_data_id = amplitudes->get_amplitudes_data_id_from_amplitude_id(amplitude_id);
  if (amplitude_data_id == -1)
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_orientation_exists(int orientation_id)
{
  int orientation_data_id;
  orientation_data_id = orientations->get_orientations_data_id_from_orientation_id(orientation_id);
  if (orientation_data_id == -1)
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_step_exists(int step_id)
{
  int step_data_id;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id == -1)
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_historyoutput_exists(int historyoutput_id)
{
  int historyoutput_data_id;
  historyoutput_data_id = historyoutputs->get_outputs_data_id_from_output_id(historyoutput_id);
  if (historyoutput_data_id == -1)
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_fieldoutput_exists(int fieldoutput_id)
{
  int field_data_id;
  field_data_id = fieldoutputs->get_outputs_data_id_from_output_id(fieldoutput_id);
  if (field_data_id == -1)
  {
    return false;
  }
  return true;
}

bool CalculiXCore::check_vertex_in_nodeset_exists(int vertex_id,int nodeset_id)
{
  std::vector<int> nodeset_vertices;
  nodeset_vertices = CubitInterface::get_nodeset_vertices(nodeset_id);
  for (size_t i = 0; i < nodeset_vertices.size(); i++)
  {
    if (nodeset_vertices[i] == vertex_id)
    {
      return true;
    }
  }
  return false;
}

int CalculiXCore::get_bc_fea_type(std::vector<std::pair<int, double>> bc_attribs)
{
  for (size_t i = 0; i < bc_attribs.size(); i++)
  {
    if ((bc_attribs[i].first > -1) && (bc_attribs[i].first < 6))
    {
      //displacement
      return 1;
    }
    if ((bc_attribs[i].first > 5) && (bc_attribs[i].first < 12))
    {
      //Velocity
      return 2;
    }
    if ((bc_attribs[i].first > 11) && (bc_attribs[i].first < 18))
    {
      //Acceleration
      return 3;
    }
    if ((bc_attribs[i].first > 17) && (bc_attribs[i].first < 23))
    //if (bc_attribs[i].first == 22)
    {
      //Temperature
      return 4;
    }
    if ((bc_attribs[i].first > 22) && (bc_attribs[i].first < 31))
    {
      //Forces
      return 5;
    }
    if (bc_attribs[i].first == 31)
    {
      //Pressure
      return 6;
    }
    if ((bc_attribs[i].first > 37) && (bc_attribs[i].first < 41))
    {
      //Heatflux
      return 7;
    }
  }
  return -1;
}

std::vector<int> CalculiXCore::get_blocks()
{ 
  std::vector<int> blocks;
  
  for (size_t i = 0; i < cb->blocks_data.size(); i++)
  {
    blocks.push_back(cb->blocks_data[i][0]);
  }
  return blocks;
}

int CalculiXCore::get_ccx_element_side(int element_type,int side)
{
  int s_return = -1;
  if ((element_type>=39) && (element_type<=43)) {
    // HEX
    if (side==1) {
      s_return=3;
    } else if (side==2) {
      s_return=4;
    } else if (side==3){
      s_return=5;
    } else if (side==4){
      s_return=6;
    } else if (side==5){
      s_return=1;
    } else if (side==6){
      s_return=2;
    }
  } else if ((element_type>=28) && (element_type<=33)) {
    // TETRA
    if (side==1) {
      s_return=2;
    } else if (side==2) {
      s_return=3;
    } else if (side==3){
      s_return=4;
    } else if (side==4){
      s_return=1;
    }
  } else if ((element_type>=11) && (element_type<=14)) {
    // TRI
    if (side==1) {
      s_return=1;
    } else if (side==2) {
      s_return=2;
    } else if (side==3){
      s_return=3;
    } else if (side==4){
      s_return=5;
    } else if (side==5){
      s_return=4;
    }
  } else if ((element_type>=15) && (element_type<=18)) {
    // TRISHELL
    if (side==1) {
      s_return=2;
    } else if (side==2) {
      s_return=1;
    } else if (side==3){
      s_return=3;
    } else if (side==4){
      s_return=4;
    } else if (side==5){
      s_return=5;
    }
  } else if ((element_type>=19) && (element_type<=22)) {
    // SHELL
    if (side==1) {
      s_return=2;
    } else if (side==2) {
      s_return=1;
    } else if (side==3){
      s_return=3;
    } else if (side==4){
      s_return=4;
    } else if (side==5){
      s_return=5;
    } else if (side==6){
      s_return=6;
    }
  } else if ((element_type>=23) && (element_type<=27)) {
    // QUAD
    if (side==1) {
      s_return=1;
    } else if (side==2) {
      s_return=2;
    } else if (side==3){
      s_return=3;
    } else if (side==4){
      s_return=4;
    } else if (side==5){
      s_return=6;
    } else if (side==6){
      s_return=5;
    }
  } else if (element_type == WEDGE || element_type == WEDGE6 || element_type == WEDGE15) {
    // WEDGE
    if (side==1) {
      s_return=3;
    } else if (side==2) {
      s_return=4;
    } else if (side==3){
      s_return=5;
    } else if (side==4){
      s_return=1;
    } else if (side==5){
      s_return=2;
    }
  } else {
    s_return = side;
  }

  return s_return;
}

bool CalculiXCore::add_sideset_face(std::string sideset_id, std::string sideset_name, std::string face, std::string element_type)
{
  std::vector<std::string> v = {sideset_id, sideset_name, face, element_type};
  
  sideset_face_data.push_back(v);
  
  return true;
}

std::vector<std::vector<std::string>> CalculiXCore::get_sideset_face(int sideset_id)
{
  std::vector<std::vector<std::string>> return_data;
  for (size_t i = 0; i < sideset_face_data.size(); i++)
  {
    if (sideset_face_data[i][0]==std::to_string(sideset_id))
    {
        return_data.push_back({sideset_face_data[i][0],sideset_face_data[i][1],sideset_face_data[i][2],sideset_face_data[i][3]});
    }  
  }
  return return_data;
}

bool CalculiXCore::create_section(std::string section_type,int block_id, int material_id, std::vector<std::string> options)
{
  return sections->create_section(section_type, block_id, material_id, options);
}

bool CalculiXCore::modify_section(std::string section_type,int section_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return sections->modify_section(section_type, section_id, options, options_marker);
}

bool CalculiXCore::delete_section(int section_id)
{
  return sections->delete_section(section_id);
}

bool CalculiXCore::create_constraint(std::string constraint_type, std::vector<std::string> options,std::vector<std::vector<double>> options2)
{
  return constraints->create_constraint(constraint_type, options,options2);
}

bool CalculiXCore::modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker,std::vector<std::vector<double>> options2)
{
  return constraints->modify_constraint(constraint_type, constraint_id, options, options_marker,options2);
}

bool CalculiXCore::delete_constraint(int constraint_id)
{
  return constraints->delete_constraint(constraint_id);
}

bool CalculiXCore::create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance) // create constraint tie from cubit contact pairs
{
  std::vector<std::string> options;
  std::vector<std::vector<double>> options2;

  std::vector<int> contact_ids;
  contact_ids = CubitInterface::get_bc_id_list(CI_BCTYPE_CONTACT_PAIR);

  BCContactHandle contact;
  
  for (size_t i = 0; i < contact_ids.size(); i++)
  {
    me_iface->get_bc_contact_handle(contact_ids[i], contact);
    BCEntityHandle master_region;
		BCEntityHandle slave_region; 
    SidesetHandle master_sideset;
    SidesetHandle slave_sideset;
    int master_id;
    int slave_id;

    me_iface->get_contact_regions(contact,master_region,slave_region);
    if (!me_iface->get_bc_sideset(master_region,master_sideset))
    {
      return false;
    }

    if (!me_iface->get_bc_sideset(slave_region,slave_sideset))
    {
      return false;
    }

    master_id = me_iface->id_from_handle(master_sideset);
    slave_id = me_iface->id_from_handle(slave_sideset);

    options.push_back(name + std::to_string(i));
    options.push_back(std::to_string(master_id));
    options.push_back(std::to_string(slave_id));
    options.push_back(position_tolerance);

    this->create_constraint("TIE", options,options2);
    options.clear();
  } 
  
  return true;
}

bool CalculiXCore::create_surfaceinteraction(std::string surfacebehavior_type, std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return surfaceinteractions->create_surfaceinteraction(surfacebehavior_type, options, options2);
}

bool CalculiXCore::modify_surfaceinteraction(std::string modify_type, int surfaceinteraction_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return surfaceinteractions->modify_surfaceinteraction(modify_type,surfaceinteraction_id,options,options_marker,options2);
}

bool CalculiXCore::delete_surfaceinteraction(int surfaceinteraction_id)
{
  return surfaceinteractions->delete_surfaceinteraction(surfaceinteraction_id);
}

bool CalculiXCore::create_contactpair(std::vector<std::string> options)
{
  return contactpairs->create_contactpair(options);
}

bool CalculiXCore::modify_contactpair(int contactpair_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return contactpairs->modify_contactpair(contactpair_id,options,options_marker);
}

bool CalculiXCore::delete_contactpair(int contactpair_id)
{
  return contactpairs->delete_contactpair(contactpair_id);
}

bool CalculiXCore::create_contactpair_from_cubitcontactpair(int surfaceinteraction_id, std::string contactpairtype, std::string adjust, std::string adjust_nodeset)
{
  std::vector<std::string> options;

  std::vector<int> contact_ids;
  contact_ids = CubitInterface::get_bc_id_list(CI_BCTYPE_CONTACT_PAIR);	

  if (surfaceinteractions->get_surfaceinteractions_data_id_from_surfaceinteraction_id(surfaceinteraction_id)==-1)
  {
    return false;
  }

  BCContactHandle contact;
  
  for (size_t i = 0; i < contact_ids.size(); i++)
  {
    me_iface->get_bc_contact_handle(contact_ids[i], contact);
    BCEntityHandle master_region;
		BCEntityHandle slave_region; 
    SidesetHandle master_sideset;
    SidesetHandle slave_sideset;
    int master_id;
    int slave_id;

    me_iface->get_contact_regions(contact,master_region,slave_region);
    if (!me_iface->get_bc_sideset(master_region,master_sideset))
    {
      return false;
    }

    if (!me_iface->get_bc_sideset(slave_region,slave_sideset))
    {
      return false;
    }

    master_id = me_iface->id_from_handle(master_sideset);
    slave_id = me_iface->id_from_handle(slave_sideset);

    options.push_back(std::to_string(surfaceinteraction_id));
    options.push_back(contactpairtype);
    options.push_back(std::to_string(master_id));
    options.push_back(std::to_string(slave_id));
    options.push_back(adjust);
    options.push_back(adjust_nodeset);

    this->create_contactpair(options);
    options.clear();
  } 
  
  return true;
}

bool CalculiXCore::create_constraint_equation_from_coincident_nodes(std::string name, int group_id, double tolerance, bool dof_1, bool dof_2, bool dof_3)
{
  return constraints->create_constraint_equation_from_coincident_nodes(name,group_id,tolerance,dof_1,dof_2,dof_3);
}

bool CalculiXCore::create_amplitude(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return amplitudes->create_amplitude(options,options2);
}

bool CalculiXCore::modify_amplitude(int amplitude_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return amplitudes->modify_amplitude(amplitude_id,options,options_marker,options2);
}

bool CalculiXCore::delete_amplitude(int amplitude_id)
{
  return amplitudes->delete_amplitude(amplitude_id);
}

bool CalculiXCore::delete_amplitudes(std::vector<int> amplitude_ids)
{
  return amplitudes->delete_amplitudes(amplitude_ids);
}

bool CalculiXCore::create_orientation(std::vector<std::string> options, std::vector<std::vector<std::string>> options2)
{
  return orientations->create_orientation(options, options2);
}

bool CalculiXCore::modify_orientation(int orientation_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<std::vector<std::string>> options2)
{
  return orientations->modify_orientation(orientation_id, options, options_marker, options2);
}

bool CalculiXCore::delete_orientation(int orientation_id)
{
  return orientations->delete_orientation(orientation_id);
}

bool CalculiXCore::modify_damping(std::vector<std::string> options, std::vector<int> options_marker)
{
  return damping->modify_damping(options, options_marker);
}

bool CalculiXCore::delete_damping(bool delete_alpha, bool delete_beta)
{
  return damping->delete_damping(delete_alpha, delete_beta);
}

bool CalculiXCore::modify_physicalconstants(std::vector<std::string> options, std::vector<int> options_marker)
{
  return physicalconstants->modify_physicalconstants(options, options_marker);
}

bool CalculiXCore::delete_physicalconstants(bool delete_absolute_zero, bool delete_stefan_boltzmann, bool delete_newton_gravity)
{
  return physicalconstants->delete_physicalconstants(delete_absolute_zero, delete_stefan_boltzmann, delete_newton_gravity);
} 

bool CalculiXCore::modify_loadsforces(int force_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadsforces->modify_load(force_id,options,options_marker);
}

bool CalculiXCore::modify_loadspressures(int pressure_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadspressures->modify_load(pressure_id,options,options_marker);
}

bool CalculiXCore::modify_loadsheatfluxes(int heatflux_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadsheatfluxes->modify_load(heatflux_id,options,options_marker);
}

bool CalculiXCore::create_loadsgravity(std::vector<std::string> options)
{
  return loadsgravity->create_load(options);
}

bool CalculiXCore::modify_loadsgravity(int gravity_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadsgravity->modify_load(gravity_id,options,options_marker);
}

bool CalculiXCore::delete_loadsgravity(int gravity_id)
{
  return loadsgravity->delete_load(gravity_id);
}

bool CalculiXCore::create_loadscentrifugal(std::vector<std::string> options)
{
  return loadscentrifugal->create_load(options);
}

bool CalculiXCore::modify_loadscentrifugal(int centrifugal_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadscentrifugal->modify_load(centrifugal_id,options,options_marker);
}

bool CalculiXCore::delete_loadscentrifugal(int centrifugal_id)
{
  return loadscentrifugal->delete_load(centrifugal_id);
}

bool CalculiXCore::create_loadstrajectory(std::string load_type, std::vector<std::string> options, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  return loadstrajectory->create_load(load_type, options, options2, options3);
}

bool CalculiXCore::modify_loadstrajectory(int trajectory_id, std::vector<std::string> options, std::vector<int> options_marker, std::vector<int> options2, std::vector<std::vector<double>> options3)
{
  return loadstrajectory->modify_load(trajectory_id,options,options_marker, options2, options3);
}

bool CalculiXCore::delete_loadstrajectory(int trajectory_id)
{
  return loadstrajectory->delete_load(trajectory_id);
}

std::string CalculiXCore::loadstrajectory_get_load_type(int trajectory_id)
{
  return loadstrajectory->get_load_type(trajectory_id);
}

int CalculiXCore::loadstrajectory_get_subload_id(int trajectory_id)
{
  return loadstrajectory->get_subload_id(trajectory_id);
}

std::vector<int> CalculiXCore::loadstrajectory_heatflux_get_node_ids(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="HEATFLUX")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->heatflux->get_node_ids(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->heatflux->get_node_ids(-1);
}

std::vector<int> CalculiXCore::loadstrajectory_heatflux_get_edge_ids(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="HEATFLUX")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->heatflux->get_edge_ids(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->heatflux->get_edge_ids(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_heatflux_get_hit_coordinates(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="HEATFLUX")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->heatflux->get_hit_coordinates(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->heatflux->get_hit_coordinates(-1);
}

std::vector<std::vector<std::vector<int>>> CalculiXCore::loadstrajectory_heatflux_get_face_ids(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="HEATFLUX")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->heatflux->get_face_ids(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->heatflux->get_face_ids(-1);
}

std::vector<std::vector<std::vector<int>>> CalculiXCore::loadstrajectory_heatflux_get_draw_face_ids(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="HEATFLUX")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->heatflux->get_draw_face_ids(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->heatflux->get_draw_face_ids(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_heatflux_get_times(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="HEATFLUX")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->heatflux->get_times(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->heatflux->get_times(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_heatflux_get_radius(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="HEATFLUX")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->heatflux->get_radius(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->heatflux->get_radius(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_heatflux_get_magnitude(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="HEATFLUX")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->heatflux->get_magnitude(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->heatflux->get_magnitude(-1);
}

std::vector<int> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_node_ids(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_node_ids(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_node_ids(-1);
}

std::vector<int> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_edge_ids(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_edge_ids(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_edge_ids(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_hit_coordinates(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_hit_coordinates(-1);
}

std::vector<std::vector<std::vector<int>>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_element_ids(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_element_ids(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_element_ids(-1);
}

std::vector<std::vector<std::vector<int>>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_draw_element_ids(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_draw_element_ids(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_times(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_times(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_times(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_radius(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_radius(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_radius(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_depth(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_depth(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_depth(-1);
}

std::vector<std::vector<double>> CalculiXCore::loadstrajectory_bodyheatfluxsphere_get_magnitude(int trajectory_id)
{
  if (loadstrajectory->get_load_type(trajectory_id)=="BODYHEATFLUXSPHERE")
  {
    int loads_data_id = loadstrajectory->get_loads_data_id_from_load_id(trajectory_id);
    return loadstrajectory->bodyheatfluxsphere->get_magnitude(loadstrajectory->loads_data[loads_data_id][2]);
  }
  return loadstrajectory->bodyheatfluxsphere->get_magnitude(-1);
}

bool CalculiXCore::create_loadsfilm(std::vector<std::string> options)
{
  return loadsfilm->create_load(options);
}

bool CalculiXCore::modify_loadsfilm(int film_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadsfilm->modify_load(film_id, options, options_marker);
}

bool CalculiXCore::delete_loadsfilm(int film_id)
{
  return loadsfilm->delete_load(film_id);
}

bool CalculiXCore::create_loadsradiation(std::vector<std::string> options)
{
  return loadsradiation->create_load(options);
}

bool CalculiXCore::modify_loadsradiation(int radiation_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return loadsradiation->modify_load(radiation_id, options, options_marker);
}

bool CalculiXCore::delete_loadsradiation(int radiation_id)
{
  return loadsradiation->delete_load(radiation_id);
} 

bool CalculiXCore::create_loadssurfacetraction(std::vector<std::string> options, std::vector<double> options2)
{
  return loadssurfacetraction->create_load(options, options2);
}

bool CalculiXCore::modify_loadssurfacetraction(int surfacetraction_id, std::vector<std::string> options, std::vector<double> options2, std::vector<int> options_marker)
{
  return loadssurfacetraction->modify_load(surfacetraction_id, options, options2, options_marker);
}

bool CalculiXCore::delete_loadssurfacetraction(int surfacetraction_id)
{
  return loadssurfacetraction->delete_load(surfacetraction_id);
}  

bool CalculiXCore::create_loadsbodyheatflux(std::vector<std::string> options, std::vector<int> options2)
{
  return loadsbodyheatflux->create_load(options, options2);
}

bool CalculiXCore::modify_loadsbodyheatflux(int bodyheatflux_id, std::vector<std::string> options, std::vector<int> options2, std::vector<int> options_marker)
{
  return loadsbodyheatflux->modify_load(bodyheatflux_id, options, options2, options_marker);
}

bool CalculiXCore::delete_loadsbodyheatflux(int bodyheatflux_id)
{
  return loadsbodyheatflux->delete_load(bodyheatflux_id);
}

bool CalculiXCore::modify_bcsdisplacements(int displacement_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return bcsdisplacements->modify_bc(displacement_id,options,options_marker);
}

bool CalculiXCore::modify_bcstemperatures(int temperature_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return bcstemperatures->modify_bc(temperature_id,options,options_marker);
}

bool CalculiXCore::create_historyoutput(std::vector<std::string> options)
{
  return historyoutputs->create_output(options);
}

bool CalculiXCore::modify_historyoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return historyoutputs->modify_output(output_id, modify_type, options, options_marker);
}

bool CalculiXCore::delete_historyoutput(int output_id)
{
  return historyoutputs->delete_output(output_id);
}

std::vector<std::string> CalculiXCore::get_historyoutput_node_keys()
{
  return  historyoutputs->node_keys;
}

std::vector<std::string> CalculiXCore::get_historyoutput_element_keys()
{
  return  historyoutputs->element_keys;
}

std::vector<std::string> CalculiXCore::get_historyoutput_contact_keys()
{
  return  historyoutputs->contact_keys;
}

std::vector<std::string> CalculiXCore::get_historyoutput_section_keys()
{
  return  historyoutputs->section_keys;
}

bool CalculiXCore::create_fieldoutput(std::vector<std::string> options)
{
  return fieldoutputs->create_output(options);
}

bool CalculiXCore::modify_fieldoutput(int output_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return fieldoutputs->modify_output(output_id, modify_type, options, options_marker);
}

bool CalculiXCore::delete_fieldoutput(int output_id)
{
  return fieldoutputs->delete_output(output_id);
}

std::vector<std::string> CalculiXCore::get_fieldoutput_node_keys()
{
  return  fieldoutputs->node_keys;
}

std::vector<std::string> CalculiXCore::get_fieldoutput_element_keys()
{
  return  fieldoutputs->element_keys;
}

std::vector<std::string> CalculiXCore::get_fieldoutput_contact_keys()
{
  return  fieldoutputs->contact_keys;
}

bool CalculiXCore::create_initialcondition(std::vector<std::string> options)
{
  return initialconditions->create_initialcondition(options);
}

bool CalculiXCore::modify_initialcondition(int initialcondition_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return initialconditions->modify_initialcondition(initialcondition_id, modify_type, options, options_marker);
}

bool CalculiXCore::add_initialcondition_stress(int initialcondition_id, int modify_type, std::vector<double> options)
{
  return initialconditions->add_initialcondition_stress(initialcondition_id, modify_type, options);
}

bool CalculiXCore::delete_initialcondition(int initialcondition_id)
{
  return initialconditions->delete_initialcondition(initialcondition_id);
}

bool CalculiXCore::hbc_add_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return hbcs->add_bcs(bcs_id, bc_type, bc_ids);
}

bool CalculiXCore::hbc_remove_bcs(int bcs_id, int bc_type, std::vector<int> bc_ids)
{
  return hbcs->remove_bcs(bcs_id, bc_type, bc_ids);
}

bool CalculiXCore::create_step(std::vector<std::string> options)
{
  return steps->create_step(options);
}

bool CalculiXCore::modify_step(int step_id, int modify_type, std::vector<std::string> options, std::vector<int> options_marker)
{
  return steps->modify_step(step_id, modify_type, options, options_marker);
}

bool CalculiXCore::delete_step(int step_id)
{
  return steps->delete_step(step_id);
}

bool CalculiXCore::step_add_loads(int step_id, int load_type, std::vector<int> load_ids)
{
  return steps->add_loads(step_id,load_type,load_ids);
}

bool CalculiXCore::step_add_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return steps->add_bcs(step_id,bc_type,bc_ids);
}

bool CalculiXCore::step_add_historyoutputs(int step_id, std::vector<int> historyoutput_ids)
{
  return steps->add_historyoutputs(step_id,historyoutput_ids);
}

bool CalculiXCore::step_add_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return steps->add_fieldoutputs(step_id, fieldoutput_ids);
}

bool CalculiXCore::step_remove_loads(int step_id, int load_type, std::vector<int> load_ids)
{
  return steps->remove_loads(step_id, load_type, load_ids);
}

bool CalculiXCore::step_remove_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return steps->remove_bcs(step_id, bc_type, bc_ids);
}

bool CalculiXCore::step_remove_historyoutputs(int step_id, std::vector<int> historyoutput_ids)
{
  return steps->remove_historyoutputs(step_id,historyoutput_ids);
}

bool CalculiXCore::step_remove_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return steps->remove_fieldoutputs(step_id,fieldoutput_ids);
}

bool CalculiXCore::step_utility_modelchangeelement_dummystep(int step_id, std::vector<int> trajectory_ids)
{
  //create steps
  if (!steps->create_modelchangeelement_dummystep(step_id))
  {
    return false;
  }
  
  //insert custom lines
  // customlines_data[0][1] name
  // customlines_data[0][2] position
  // customlines_data[0][3] insert keyword
  // customlines_data[0][4] insert keyword id
  // customlines_data[0][5] customline
  std::vector<std::string> options;
  options.push_back("DUMMY STEP 2");
  options.push_back("AFTER");
  options.push_back("STEP_BEGIN");
  options.push_back("2");

  std::string cline;
  cline = "*MODEL CHANGE,TYPE=ELEMENT,REMOVE\n";
  std::vector<int> element_ids;
  
  for (size_t i = 0; i < trajectory_ids.size(); i++)
  {
    std::vector<std::vector<std::vector<int>>> tmp_ids = this->loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(trajectory_ids[i]);
    if (tmp_ids.size()==0)
    {
      return false;
    }
    
    for (size_t ii = 0; ii < tmp_ids.size(); ii++)
    {
      for (size_t iii = 0; iii < tmp_ids[ii].size(); iii++)
      {
        for (size_t iv = 0; iv < tmp_ids[ii][iii].size(); iv++)
        {
          element_ids.push_back(tmp_ids[ii][iii][iv]);
        }
      }
    }
  }
  sort(begin(element_ids), end(element_ids));
  // loop over all elements
  for (size_t i = 0; i < element_ids.size(); i++)
  { 
    std::vector<int>::iterator ip;

    // Using std::unique
    ip = std::unique(element_ids.begin(), element_ids.begin() + element_ids.size());
    // Now v becomes {1 3 10 1 3 7 8 * * * * *}
    // * means undefined

    // Resizing the vector so as to remove the undefined
    // terms
    element_ids.resize(std::distance(element_ids.begin(), ip));
  }

  int ic = 0;
  for (size_t i = 0; i < element_ids.size(); i++)
  {
    cline.append(std::to_string(element_ids[i]));
    ic = ic + 1;
    if (ic == 16)
    {
      cline.append(",\n");
      ic = 0;
    }else{
      cline.append(",");
    }
  }

  options.push_back(cline);

  customlines->create_customline(options);

  return true;
}

StepSplitResult CalculiXCore::step_utility_split_step(int step_id, std::vector<std::vector<double>> times)
{
  return steps->split_step(step_id, times);
}

bool CalculiXCore::step_utility_save_backup()
{
  return steps->save_backup();
}

bool CalculiXCore::step_utility_load_backup()
{
  return steps->load_backup();
}

bool CalculiXCore::create_job(std::vector<std::string> options)
{
  return jobs->create_job(options);
}

bool CalculiXCore::modify_job(int job_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return jobs->modify_job(job_id, options, options_marker);
}

bool CalculiXCore::delete_job(int job_id)
{
  return jobs->delete_job(job_id);
}

bool CalculiXCore::get_job_topology_context(int job_id, int& topology_id, int& iteration_id)
{
  topology_id = -1;
  iteration_id = -1;

  if (!topology || job_id <= 0)
  {
    PRINT_ERROR("Invalid job topology context query.\n");
    return false;
  }

  for (const auto& configuration : topology->topology_cases)
  {
    for (const auto* associations : {&configuration.second.iteration_jobs, &configuration.second.iteration_adjoint_jobs})
    {
      for (const auto& iteration : *associations)
      {
        if (iteration.second != job_id)
        {
          continue;
        }

        if (topology_id != -1 || configuration.first <= 0 || iteration.first < 1)
        {
          PRINT_ERROR("Invalid topology association for job %d.\n", job_id);
          return false;
        }

        topology_id = configuration.first;
        iteration_id = iteration.first;
      }
    }
  }

  // Ordinary jobs return true with both IDs set to -1.
  return true;
}

bool CalculiXCore::prepare_job_input(int job_id, const std::string& filename)
{
  if (job_id <= 0 || filename.empty() || filename.find_first_of("\"\r\n") != std::string::npos)
  {
    PRINT_ERROR("Invalid job input request.\n");
    return false;
  }

  if (job_export_context.job_id > 0)
  {
    PRINT_ERROR("A job input export is already active.\n");
    return false;
  }

  JobExportContext context;
  context.job_id = job_id;

  if (!get_job_topology_context(job_id, context.topology_id, context.iteration_id))
  {
    return false;
  }

  if (context.topology_id > 0)
  {
    const auto& configuration = topology->topology_cases.find(context.topology_id)->second;
    const auto adjoint = configuration.iteration_adjoint_jobs.find(context.iteration_id);

    context.adjoint = adjoint != configuration.iteration_adjoint_jobs.end() && adjoint->second == job_id;
  }

  const QScopedValueRollback<JobExportContext> guard(job_export_context, context);

  std::string command = "export ccx \"" + filename + "\"";

  if (context.topology_id > 0)
  {
    command += " topology " + std::to_string(context.topology_id);
    command += " iteration " + std::to_string(context.iteration_id);
  }

  command += " overwrite";

  return silent_cmd(command);
}

bool CalculiXCore::run_job(int job_id, int option)
{
  return jobs->run_job(job_id,option);
}

bool CalculiXCore::check_jobs()
{
  if (!jobs)
  {
    return false;
  }

  if (checking_jobs)
  {
    return true;
  }

  const QScopedValueRollback<bool> guard(checking_jobs, true);
  const bool result = jobs->check_jobs();

  if (topologyoptimization)
  {
    topologyoptimization->update();
  }

  return result;
}

bool CalculiXCore::wait_job(int job_id)
{
  return jobs->wait_job(job_id);
}

bool CalculiXCore::kill_job(int job_id)
{
  return jobs->kill_job(job_id);
}

bool CalculiXCore::create_topology(int topology_id, const TopologyOptions& options)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->create_topology(topology_id, options);
}

bool CalculiXCore::modify_topology(int topology_id, const TopologyOptions& options)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->modify_topology(topology_id, options);
}

bool CalculiXCore::reset_topology_run(int topology_id)
{
  if (!topology || !jobs)
  {
    PRINT_ERROR("Topology or job core is not initialized.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (entry->second.run_state == TopologyRunState::Running)
  {
    PRINT_ERROR("Topology %d is running.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;

  // Check every associated job before deleting any.
  for (const auto* associations : {&configuration.iteration_jobs, &configuration.iteration_adjoint_jobs})
  {
    for (const auto& iteration : *associations)
    {
      const int status = jobs->get_job_status(iteration.second);

      if (status == 1 || status == 5)
      {
        PRINT_ERROR("Topology %d has an active associated job.\n", topology_id);
        return false;
      }
    }
  }

  for (auto* associations : {&configuration.iteration_adjoint_jobs, &configuration.iteration_jobs})
  {
    for (auto iterator = associations->begin(); iterator != associations->end();)
    {
      const int job_id = iterator->second;

      if (jobs->get_jobs_data_id_from_job_id(job_id) >= 0 && !jobs->delete_job(job_id))
      {
        PRINT_ERROR("Failed to delete topology job %d.\n", job_id);
        return false;
      }

      iterator = associations->erase(iterator);
    }
  }

  configuration.selected_iteration = -1;
  configuration.element_preserved.clear();
  configuration.iteration_element_energies.clear();
  configuration.iteration_total_energies.clear();
  configuration.iteration_evaluations.clear();
  configuration.iteration_density_designs.clear();
  configuration.run_state = TopologyRunState::NotRun;

  if (mat)
  {
    mat->topology_material_data.erase(topology_id);
  }
  if (sections)
  {
    sections->topology_solid_sections.erase(topology_id);
  }
  if (topologyoptimization)
  {
    topologyoptimization->forget_topology_run(topology_id);
  }
  return true;
}

bool CalculiXCore::delete_topology(int topology_id)
{
  if (!reset_topology_run(topology_id))
  {
    return false;
  }

  return topology->delete_topology(topology_id);
}

bool CalculiXCore::get_topology_preserved_sets(
  int topology_id,
  std::vector<TopologyPreservedSetInfo>& sets,
  std::set<int>& direct_elements)
{
  sets.clear();
  direct_elements.clear();

  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  std::map<int, std::set<int>> defaults;
  std::set<int> collected_direct_elements;

  if (!collect_topology_default_sets(
        defaults, collected_direct_elements))
  {
    return false;
  }

  const auto& configuration = entry->second;
  auto all_sets = defaults;

  const auto merge = [&](const std::map<int, std::set<int>>& groups)
  {
    for (const auto& group : groups)
    {
      all_sets[group.first].insert(
        group.second.begin(), group.second.end());
    }
  };

  merge(configuration.added_preserved_sets);
  merge(configuration.removed_preserved_sets);

  const auto contains = [](
    const std::map<int, std::set<int>>& groups,
    int set_type,
    int set_id) -> bool
  {
    const auto group = groups.find(set_type);

    return group != groups.end() &&
      group->second.count(set_id) != 0;
  };

  std::vector<TopologyPreservedSetInfo> rows;

  for (const auto& group : all_sets)
  {
    if (group.first < 1 || group.first > 3)
    {
      PRINT_ERROR("Invalid topology preserved set type %d.\n",
        group.first);
      return false;
    }

    for (int set_id : group.second)
    {
      if (set_id <= 0)
      {
        PRINT_ERROR("Invalid topology preserved set ID %d.\n",
          set_id);
        return false;
      }

      TopologyPreservedSetInfo row;
      row.set_type = group.first;
      row.set_id = set_id;

      row.automatic = contains(
        defaults, row.set_type, set_id);

      row.user_added = contains(
        configuration.added_preserved_sets,
        row.set_type, set_id);

      row.excluded = contains(
        configuration.removed_preserved_sets,
        row.set_type, set_id);

      row.effective =
        (row.automatic || row.user_added) && !row.excluded;

      rows.push_back(row);
    }
  }

  sets.swap(rows);
  direct_elements.swap(collected_direct_elements);
  return true;
}

bool CalculiXCore::reset_preserved_sets(int topology_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->reset_preserved_sets(topology_id);
}

bool CalculiXCore::add_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->add_preserved_sets(topology_id, set_type, ids);
}

bool CalculiXCore::remove_preserved_sets(int topology_id, int set_type, const std::vector<int>& ids)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->remove_preserved_sets(topology_id, set_type, ids);
}

bool CalculiXCore::get_topology_set_element_ids(int topology_id,int set_type,int set_id, std::vector<int>& element_ids)
{
  element_ids.clear();

  if (!mat || !me_iface)
  {
    PRINT_ERROR("Topology mesh/material interfaces are not initialized.\n");
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end())
  {
    PRINT_ERROR("Material collection for topology %d not found.\n", topology_id);
    return false;
  }

  const auto& analysis_elements = collection->second.element_base_material;

  bool exists = false;

  if (set_id > 0)
  {
    if (set_type == 1)
    {
      exists = check_block_exists(set_id);
    }
    else if (set_type == 2)
    {
      exists = check_nodeset_exists(set_id);
    }
    else if (set_type == 3)
    {
      exists = check_sideset_exists(set_id);
    }
  }

  if (!exists)
  {
    PRINT_ERROR("Invalid topology set type %d, ID %d.\n", set_type, set_id);
    return false;
  }

  std::set<int> resolved;

  if (set_type == 1)
  {
    for (int id : get_block_element_ids(set_id))
    {
      if (analysis_elements.count(id) != 0)
      {
        resolved.insert(id);
      }
    }
  }
    else if (set_type == 2)
  {
    const auto node_ids = CubitInterface::get_nodeset_nodes_inclusive(set_id);

    const std::set<int> selected_nodes(
        node_ids.begin(), node_ids.end());

    for (const auto& assignment : analysis_elements)
    {
      const int global_id = assignment.first;

      if (!CubitInterface::get_element_exists(global_id))
      {
        PRINT_ERROR("Topology element %d does not exist.\n", global_id);
        return false;
      }

      std::string entity = CubitInterface::get_element_type(global_id);

      if (entity != "hex" &&
          entity != "tet" &&
          entity != "wedge")
      {
        entity = get_cubit_element_type_entity(entity);
      }

      const int local_id = CubitInterface::get_element_type_id(global_id);

      if ((entity != "hex" &&
           entity != "tet" &&
           entity != "wedge") ||
          local_id <= 0)
      {
        PRINT_ERROR("Invalid topology volume element %d.\n", global_id);
        return false;
      }

      // Include midside nodes for quadratic elements.
      const auto connectivity =
          CubitInterface::get_expanded_connectivity(
              entity, local_id);

      if (connectivity.empty())
      {
        PRINT_ERROR("Missing connectivity for topology element %d.\n", global_id);
        return false;
      }

      for (const int node_id : connectivity)
      {
        if (selected_nodes.count(node_id) != 0)
        {
          resolved.insert(global_id);
          break;
        }
      }
    }

    PRINT_INFO(
        "Topology %d: nodeset %d contains %zu nodes "
        "and selects %zu preserved elements.\n",
        topology_id, set_id,
        selected_nodes.size(), resolved.size());
  }
  else
  {
    SidesetHandle sideset;

    if (!me_iface->get_sideset_handle(set_id, sideset))
    {
      PRINT_ERROR("Unable to query sideset %d.\n", set_id);
      return false;
    }

    std::vector<ElementHandle> handles;
    std::vector<ElementType> types;
    std::vector<int> side_ids;

    const int batch_size = 100;
    int start = 0;
    int count = 0;

    while ((count = me_iface->get_sideset_sides(
                sideset, start, batch_size,
                handles, types, side_ids)) > 0)
    {
      for (int i = 0; i < count; ++i)
      {
        int local_id = -1;
        me_iface->get_element_id(handles[i], local_id);

        const std::string entity = get_cubit_element_type_entity(
            me_iface->get_element_type_name(types[i]));

        const int global_id =
            CubitInterface::get_global_element_id(entity, local_id);

        if (analysis_elements.count(global_id) != 0)
        {
          resolved.insert(global_id);
        }
      }

      start += count;
    }

    if (count < 0)
    {
      PRINT_ERROR("Failed to read sideset %d.\n", set_id);
      return false;
    }
  }

  element_ids.assign(resolved.begin(), resolved.end());
  return true;
}

bool CalculiXCore::collect_topology_default_sets(std::map<int, std::set<int>>& default_sets,std::set<int>& default_elements)
{
  default_sets.clear();
  default_elements.clear();

  if (!constraints || !contactpairs || !me_iface || !steps ||
    !loadsgravity || !loadscentrifugal ||
    !loadsfilm || !loadsradiation || !loadssurfacetraction || !loadsbodyheatflux)
  {
    PRINT_ERROR("Topology preservation cores are not initialized.\n");
    return false;
  }

  std::map<int, std::set<int>> collected;
  std::set<int> collected_elements;
  std::set<int> collected_nodes;

  const auto add_node_id = [&](double value) -> bool
  {
    if (!std::isfinite(value) ||
        value < 1.0 ||
        value > static_cast<double>(std::numeric_limits<int>::max()) ||
        std::floor(value) != value)
    {
      PRINT_ERROR("Invalid preserved node ID.\n");
      return false;
    }

    const int node_id = static_cast<int>(value);

    if (!CubitInterface::get_node_exists(node_id))
    {
      PRINT_ERROR("Preserved node %d does not exist.\n", node_id);
      return false;
    }

    collected_nodes.insert(node_id);
    return true;
  };

  // Read stored string IDs without throwing exceptions.
  const auto read_positive_id =
      [](const std::string& text, int& id) -> bool
  {
    std::istringstream input(text);

    if (!(input >> id) || id <= 0)
    {
      return false;
    }

    input >> std::ws;
    return input.eof();
  };

  const auto add_string_id =
      [&](int set_type, const std::string& text) -> bool
  {
    int id = -1;

    if (!read_positive_id(text, id))
    {
      PRINT_ERROR("Invalid preserved set ID '%s'.\n", text.c_str());
      return false;
    }

    collected[set_type].insert(id);
    return true;
  };

  // Load/BC type -> IDs assigned to existing steps.
  std::map<int, std::set<int>> used_loads;
  std::map<int, std::set<int>> used_bcs;

  for (const auto& step : steps->steps_data)
  {
    if (step.size() < 7)
    {
      PRINT_ERROR("Invalid step data.\n");
      return false;
    }

    const auto load_indices =
        steps->get_load_data_ids_from_loads_id(step[5]);

    for (int index : load_indices)
    {
      const auto& load = steps->loads_data[index];

      if (load.size() < 3 ||
          load[1] < 1 || load[1] > 10 || load[2] <= 0)
      {
        PRINT_ERROR("Invalid step load reference.\n");
        return false;
      }

      used_loads[load[1]].insert(load[2]);
    }

    const auto bc_indices =
        steps->get_bc_data_ids_from_bcs_id(step[6]);

    for (int index : bc_indices)
    {
      const auto& bc = steps->bcs_data[index];

      if (bc.size() < 3 ||
          (bc[1] != 1 && bc[1] != 2) || bc[2] <= 0)
      {
        PRINT_ERROR("Invalid step boundary condition reference.\n");
        return false;
      }

      used_bcs[bc[1]].insert(bc[2]);
    }
  }

  // Constraints: rigid bodies and ties.
  for (const auto& constraint : constraints->constraints_data)
  {
    if (constraint.size() < 3)
    {
      PRINT_ERROR("Invalid constraint data.\n");
      return false;
    }

    const int type = constraint[1];
    const int subtype_id = constraint[2];

    if (type == 1)
    {
      const int index =
          constraints->get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(
              subtype_id);

      if (index < 0 ||
          constraints->rigidbody_constraint_data[index].size() < 3)
      {
        PRINT_ERROR("Missing rigid body constraint data.\n");
        return false;
      }

      const auto& rigid =
          constraints->rigidbody_constraint_data[index];

      // Rigid body types: 1 = nodeset, 2 = block.
      // Preservation types: 1 = block, 2 = nodeset.
      int set_type = -1;

      if (rigid[1] == "1")
      {
        set_type = 2;
      }
      else if (rigid[1] == "2")
      {
        set_type = 1;
      }
      else
      {
        PRINT_ERROR("Invalid rigid body selection type.\n");
        return false;
      }

      if (!add_string_id(set_type, rigid[2]))
      {
        return false;
      }
    }
    else if (type == 2)
    {
      const int index =
          constraints->get_tie_constraint_data_id_from_tie_constraint_id(
              subtype_id);

      if (index < 0 ||
          constraints->tie_constraint_data[index].size() < 4)
      {
        PRINT_ERROR("Missing tie constraint data.\n");
        return false;
      }

      const auto& tie = constraints->tie_constraint_data[index];

      if (!add_string_id(3, tie[2]) ||
          !add_string_id(3, tie[3]))
      {
        return false;
      }
    }
    else if (type == 3)
    {
      const auto indices =
          constraints->get_equation_data_ids_from_equation_constraint_id(
              subtype_id);

      if (indices.empty())
      {
        PRINT_ERROR("Missing equation constraint data.\n");
        return false;
      }

      for (int index : indices)
      {
        const auto& equation = constraints->equation_data[index];

        if (equation.size() < 4)
        {
          PRINT_ERROR("Invalid equation constraint data.\n");
          return false;
        }

        if (!add_node_id(equation[1]))
        {
          return false;
        }
      }
    }
    else if (type == 4)
    {
      const auto indices =
          constraints->get_equation_group_data_ids_from_equation_group_constraint_id(
              subtype_id);

      if (indices.empty())
      {
        PRINT_ERROR("Missing equation group constraint data.\n");
        return false;
      }

      for (int index : indices)
      {
        const auto& equation = constraints->equation_group_data[index];

        if (equation.size() < 4)
        {
          PRINT_ERROR("Invalid equation group constraint data.\n");
          return false;
        }

        if (!add_node_id(equation[1]) ||
            !add_node_id(equation[2]))
        {
          return false;
        }
      }
    }
  }

  // Contact pairs: master/slave sidesets and optional adjust nodeset.
  for (const auto& contact : contactpairs->contactpairs_data)
  {
    if (contact.size() < 6 ||
        contact[3] <= 0 || contact[4] <= 0)
    {
      PRINT_ERROR("Invalid contact pair data.\n");
      return false;
    }

    collected[3].insert(contact[3]);
    collected[3].insert(contact[4]);

    const int index =
        contactpairs->get_adjust_contactpair_data_id_from_adjust_contactpair_id(
            contact[5]);

    if (index < 0 ||
        contactpairs->adjust_contactpair_data[index].size() < 3)
    {
      PRINT_ERROR("Missing contact pair adjustment data.\n");
      return false;
    }

    const std::string& nodeset =
        contactpairs->adjust_contactpair_data[index][2];

    if (!nodeset.empty() &&
        !add_string_id(2, nodeset))
    {
      return false;
    }
  }

  // These load tables store their block/sideset ID at column 4.
  const auto collect_load_sets =
      [&](const std::vector<std::vector<int>>& rows,
          int load_type,
          int set_type,
          const char* load_name) -> bool
  {
    for (int load_id : used_loads[load_type])
    {
      const auto row = std::find_if(
          rows.begin(), rows.end(),
          [&](const std::vector<int>& data)
          {
            return !data.empty() && data[0] == load_id;
          });

      if (row == rows.end() || row->size() < 5 || (*row)[4] <= 0)
      {
        PRINT_ERROR("Missing or invalid %s load %d.\n",
                    load_name, load_id);
        return false;
      }

      collected[set_type].insert((*row)[4]);
    }

    return true;
  };

  // Block references.
  if (!collect_load_sets(loadsgravity->loads_data,4,1, "gravity") ||
      !collect_load_sets(loadscentrifugal->loads_data,5,1, "centrifugal"))
  {
    return false;
  }

  // Sideset references.
  if (!collect_load_sets(loadsfilm->loads_data,7,3, "film") ||
      !collect_load_sets(loadsradiation->loads_data,8,3, "radiation") ||
      !collect_load_sets(loadssurfacetraction->loads_data,9,3, "surface traction"))
  {
    return false;
  }

  // Body heat flux: column 4 identifies the stored selection.
  // Column 5: 1 = blocks, 2 = global elements.
  for (int load_id : used_loads[10])
  {
    const int load_index =
        loadsbodyheatflux->get_loads_data_id_from_load_id(load_id);

    if (load_index < 0)
    {
      PRINT_ERROR("Body heat flux load %d does not exist.\n", load_id);
      return false;
    }

    const auto& load = loadsbodyheatflux->loads_data[load_index];

    if (load.size() < 6 ||
        (load[5] != 1 && load[5] != 2))
    {
      PRINT_ERROR("Invalid body heat flux selection.\n");
      return false;
    }

    const int index =
        loadsbodyheatflux->get_element_data_id_from_element_id(
            load[4]);

    if (index < 0 ||
        loadsbodyheatflux->element_data[index].size() < 2)
    {
      PRINT_ERROR("Missing body heat flux selection data.\n");
      return false;
    }

    const auto& selection = loadsbodyheatflux->element_data[index];

    // Entry 0 is the selection record ID.
    for (size_t i = 1; i < selection.size(); ++i)
    {
      const int id = selection[i];

      if (id <= 0)
      {
        PRINT_ERROR("Invalid body heat flux target ID %d.\n", id);
        return false;
      }

      if (load[5] == 1)
      {
        collected[1].insert(id);
      }
      else
        collected_elements.insert(id);
    }
  }

  for (int trajectory_id : used_loads[6])
  {
    if (!loadstrajectory)
    {
      PRINT_ERROR("Trajectory core is not initialized.\n");
      return false;
    }

    const int index =
        loadstrajectory->get_loads_data_id_from_load_id(
            trajectory_id);

    if (index < 0 ||
        loadstrajectory->loads_data[index].size() < 3)
    {
      PRINT_ERROR("Missing trajectory load %d.\n", trajectory_id);
      return false;
    }

    const auto& trajectory = loadstrajectory->loads_data[index];
    const int type = trajectory[1];
    const int subload_id = trajectory[2];

    if (type == 1)
    {
      if (!loadstrajectory->heatflux ||
          loadstrajectory->heatflux
              ->get_loads_data_id_from_load_id(subload_id) < 0)
      {
        PRINT_ERROR("Missing surface heat flux trajectory data.\n");
        return false;
      }

      // Sorted face corner nodes -> global volume element IDs.
      std::map<std::vector<int>, std::set<int>> face_owners;

      const auto connectivity = get_element_id_type_connectivity();

      for (const auto& element : connectivity)
      {
        if (element.size() < 3 || element[0] <= 0)
        {
          PRINT_ERROR("Invalid topology connectivity row.\n");
          return false;
        }

        const std::string entity = get_cubit_element_type_entity(
            me_iface->get_element_type_name(
                static_cast<ElementType>(element[1])));

        // Corner-node indices in Exodus connectivity order.
        std::vector<std::vector<int>> faces;
        size_t corner_count = 0;

        if (entity == "hex")
        {
          corner_count = 8;
          faces = {
              {0, 1, 2, 3},
              {4, 5, 6, 7},
              {0, 1, 5, 4},
              {1, 2, 6, 5},
              {2, 3, 7, 6},
              {3, 0, 4, 7}
          };
        }
        else if (entity == "tet")
        {
          corner_count = 4;
          faces = {
              {0, 1, 2},
              {0, 1, 3},
              {1, 2, 3},
              {2, 0, 3}
          };
        }
        else if (entity == "wedge")
        {
          corner_count = 6;
          faces = {
              {0, 1, 2},
              {3, 4, 5},
              {0, 1, 4, 3},
              {1, 2, 5, 4},
              {2, 0, 3, 5}
          };
        }
        else
        {
          continue;
        }

        if (element.size() < corner_count + 2)
        {
          PRINT_ERROR("Incomplete connectivity for element %d.\n",
                      element[0]);
          return false;
        }

        for (const auto& face : faces)
        {
          std::vector<int> key;

          for (int corner : face)
          {
            key.push_back(element[2 + corner]);
          }

          std::sort(key.begin(), key.end());
          face_owners[key].insert(element[0]);
        }
      }

      const auto targets =
          loadstrajectory->heatflux->get_face_ids(subload_id);

      std::set<int> selected_faces;

      for (const auto& position : targets)
      {
        for (const auto& selection : position)
        {
          selected_faces.insert(selection.begin(), selection.end());
        }
      }

      for (int face_id : selected_faces)
      {
        if (face_id <= 0)
        {
          PRINT_ERROR("Invalid trajectory face ID %d.\n", face_id);
          return false;
        }

        // Cubit get_connectivity() returns corner nodes.
        auto key = CubitInterface::get_connectivity("face", face_id);

        if (key.size() != 3 && key.size() != 4)
        {
          PRINT_ERROR("Invalid connectivity for trajectory face %d.\n",
                      face_id);
          return false;
        }

        std::sort(key.begin(), key.end());

        const auto owners = face_owners.find(key);

        if (owners == face_owners.end())
        {
          PRINT_ERROR(
              "Trajectory face %d has no hex, tet or wedge owner "
              "in the analysis blocks.\n",
              face_id);
          return false;
        }

        collected_elements.insert(
            owners->second.begin(), owners->second.end());
      }

      // This trajectory is complete; skip the sphere branch.
      continue;
    }

    if (type != 2 || !loadstrajectory->bodyheatfluxsphere)
    {
      PRINT_ERROR("Invalid trajectory load type for %d.\n",
                  trajectory_id);
      return false;
    }

    const int subindex =
        loadstrajectory->bodyheatfluxsphere
            ->get_loads_data_id_from_load_id(subload_id);

    if (subindex < 0)
    {
      PRINT_ERROR("Missing body heat flux sphere trajectory data.\n");
      return false;
    }

    const auto targets =
        loadstrajectory->bodyheatfluxsphere
            ->get_element_ids(subload_id);

    // Targets are grouped by trajectory position and radius/depth.
    for (const auto& position : targets)
    {
      for (const auto& selection : position)
      {
        for (int element_id : selection)
        {
          if (element_id <= 0)
          {
            PRINT_ERROR("Invalid trajectory target element %d.\n",
                        element_id);
            return false;
          }

          collected_elements.insert(element_id);
        }
      }
    }
  }

  // Query only Cubit loads and BCs assigned to steps.
  const auto collect_cubit_bc_sets =
      [&](const std::set<int>& ids,
          BCEntityType export_type,
          int set_type,
          const char* description) -> bool
  {
    for (int id : ids)
    {
      BCEntityHandle handle;

      if (!me_iface->get_bc_handle(export_type, id, handle))
      {
        PRINT_ERROR("Unable to query %s %d.\n", description, id);
        return false;
      }

      int set_id = -1;

      if (set_type == 2)
      {
        NodesetHandle nodeset;

        if (!me_iface->get_bc_nodeset(handle, nodeset))
        {
          PRINT_ERROR("Unable to query nodeset for %s %d.\n",
                      description, id);
          return false;
        }

        set_id = me_iface->id_from_handle(nodeset);
      }
      else
      {
        SidesetHandle sideset;

        if (!me_iface->get_bc_sideset(handle, sideset))
        {
          PRINT_ERROR("Unable to query sideset for %s %d.\n",
                      description, id);
          return false;
        }

        set_id = me_iface->id_from_handle(sideset);
      }

      if (set_id <= 0)
      {
        PRINT_ERROR("Invalid set ID for %s %d.\n", description, id);
        return false;
      }

      collected[set_type].insert(set_id);
    }

    return true;
  };

  if (!collect_cubit_bc_sets(used_loads[1], BCTYPE_FORCE, 2, "force") ||
      !collect_cubit_bc_sets(used_loads[2], BCTYPE_PRESSURE, 3, "pressure") ||
      !collect_cubit_bc_sets(used_loads[3], BCTYPE_HEATFLUX, 3, "heat flux") ||
      !collect_cubit_bc_sets(used_bcs[1], BCTYPE_DISPLACEMENT, 2, "displacement") ||
      !collect_cubit_bc_sets(used_bcs[2], BCTYPE_TEMPERATURE, 2, "temperature"))
  {
    return false;
  }

  // get elements from nodes
  if (!collected_nodes.empty())
  {
    if (!me_iface)
    {
      PRINT_ERROR("Mesh export interface is not initialized.\n");
      return false;
    }

    const auto connectivity = get_element_id_type_connectivity();

    for (const auto& element : connectivity)
    {
      // Row: global element ID, element type, node IDs.
      if (element.size() < 3 || element[0] <= 0)
      {
        PRINT_ERROR("Invalid topology connectivity row.\n");
        return false;
      }

      for (size_t i = 2; i < element.size(); ++i)
      {
        if (collected_nodes.count(element[i]) != 0)
        {
          collected_elements.insert(element[0]);
          break;
        }
      }
    }
  }

  default_sets.swap(collected);
  default_elements.swap(collected_elements);
  return true;
}

bool CalculiXCore::resolve_topology_preservation(int topology_id,const std::map<int, std::set<int>>& default_sets,const std::set<int>& default_elements)
{
  if (!topology || !mat)
  {
    PRINT_ERROR("Topology/material cores are not initialized.\n");
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end())
  {
    PRINT_ERROR("Material collection for topology %d not found.\n",
                topology_id);
    return false;
  }

  std::vector<int> element_ids;

  for (const auto& assignment : collection->second.element_base_material)
  {
    element_ids.push_back(assignment.first);
  }

  return topology->resolve_preserved_elements(topology_id, element_ids, default_sets, default_elements);
}

int CalculiXCore::start_topology_iteration(int topology_id, int iteration_id)
{
  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return -1;
  }

  return topologyoptimization->start_topology_iteration(topology_id, iteration_id);
}

int CalculiXCore::create_topology_iteration_job(int topology_id, int iteration_id, const std::string& job_name)
{
  if (!topology || !mat || !jobs || iteration_id < 1 || job_name.empty())
  {
    PRINT_ERROR("Invalid topology iteration job request.\n");
    return -1;
  }

  const auto entry = topology->topology_cases.find(topology_id);
  const auto collection = mat->topology_material_data.find(topology_id);

  if (entry == topology->topology_cases.end() || collection == mat->topology_material_data.end())
  {
    PRINT_ERROR("Topology %d is not prepared.\n", topology_id);
    return -1;
  }

  auto& configuration = entry->second;

  if ((configuration.objective == TopologyObjective::Compliance ||
       configuration.objective == TopologyObjective::Displacement) &&
      !validate_topology_compliance_model())
  {
    return -1;
  }

  if (collection->second.iteration_factors.count(iteration_id) == 0)
  {
    PRINT_ERROR("Factors for topology %d iteration %d are missing.\n", topology_id, iteration_id);
    return -1;
  }

  if (configuration.iteration_jobs.count(iteration_id) != 0)
  {
    PRINT_ERROR("Topology %d iteration %d already has a job.\n", topology_id, iteration_id);
    return -1;
  }

  for (const auto& iteration : configuration.iteration_jobs)
  {
    const int status = jobs->get_job_status(iteration.second);

    if (status == 1 || status == 5)
    {
      PRINT_ERROR("Topology %d already has an active job.\n", topology_id);
      return -1;
    }
  }

  for (const auto& job : jobs->jobs_data)
  {
    if (job.size() > 1 && job[1] == job_name)
    {
      PRINT_ERROR("Job name '%s' already exists.\n", job_name.c_str());
      return -1;
    }
  }

  int job_id = -1;

  if (!jobs->create_job({job_name, ""}, job_id))
  {
    return -1;
  }

  // Register before launch so export can retrieve the iteration context.
  configuration.iteration_jobs.emplace(iteration_id, job_id);

  return job_id;
}

bool CalculiXCore::create_topology_adjoint_job(int topology_id, int iteration_id, int& job_id, bool& skipped)
{
  job_id = -1;
  skipped = false;

  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return topologyoptimization->create_topology_adjoint_job(topology_id, iteration_id, job_id, skipped);
}

int CalculiXCore::get_topology_adjoint_job_source(int topology_id, int iteration_id)
{
  if (!topology || !jobs || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology adjoint job request.\n");
    return -1;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return -1;
  }

  if (entry->second.iteration_adjoint_jobs.count(iteration_id) != 0)
  {
    PRINT_ERROR("Topology %d iteration %d already has an adjoint job.\n", topology_id, iteration_id);
    return -1;
  }

  const int physical_job = get_topology_iteration_job_id(topology_id, iteration_id);

  if (physical_job <= 0 || jobs->get_job_status(physical_job) != 2)
  {
    PRINT_ERROR("A completed physical job is required before creating an adjoint job.\n");
    return -1;
  }

  return physical_job;
}

bool CalculiXCore::register_topology_adjoint_job(int topology_id, int iteration_id, int& job_id)
{
  job_id = -1;

  const int physical_job = get_topology_adjoint_job_source(topology_id, iteration_id);

  if (physical_job <= 0)
  {
    return false;
  }

  const int physical_index = jobs->get_jobs_data_id_from_job_id(physical_job);

  if (physical_index < 0 || jobs->jobs_data[physical_index].size() < 2)
  {
    PRINT_ERROR("Physical job data is missing.\n");
    return false;
  }

  const std::string name = jobs->jobs_data[physical_index][1] + "_adjoint";

  for (const auto& job : jobs->jobs_data)
  {
    if (job.size() > 1 && job[1] == name)
    {
      PRINT_ERROR("Job name '%s' already exists.\n", name.c_str());
      return false;
    }
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (!jobs->create_job({name, ""}, job_id))
  {
    return false;
  }

  // Register ownership before running or exporting.
  entry->second.iteration_adjoint_jobs.emplace(iteration_id, job_id);

  return true;
}

bool CalculiXCore::read_topology_job_displacements(int job_id, std::map<int, std::array<double, 3>>& displacements)
{
  displacements.clear();

  if (!results || !jobs || !topology || !mat || job_id <= 0 || jobs->get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Topology displacement results require a completed job.\n");
    return false;
  }

  int topology_id = -1;
  int iteration_id = -1;

  if (!get_job_topology_context(job_id, topology_id, iteration_id))
  {
    return false;
  }

  if (topology_id <= 0 || iteration_id < 1)
  {
    PRINT_ERROR("Job %d is not associated with a topology iteration.\n", job_id);
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Prepared elements for topology %d are missing.\n", topology_id);
    return false;
  }

  const int frd_index = results->get_frd_data_id_from_job_id(job_id);

  if (frd_index < 0)
  {
    PRINT_ERROR("FRD results for job %d are missing.\n", job_id);
    return false;
  }

  const auto& frd = results->frd_data[frd_index];

  if (frd.current_total_increment < 1)
  {
    PRINT_ERROR("Job %d has no completed result increment.\n", job_id);
    return false;
  }

  std::map<int, std::array<double, 3>> measured;
  const std::array<std::string, 3> component_names = {"D1", "D2", "D3"};

  for (const auto& block : frd.result_blocks)
  {
    if (block.size() < 7)
    {
      PRINT_ERROR("Invalid FRD result block for job %d.\n", job_id);
      return false;
    }

    if (block[3] != frd.current_total_increment)
    {
      continue;
    }

    const int type = block[5];

    if (type < 0 || static_cast<std::size_t>(type) >= frd.result_block_type.size())
    {
      PRINT_ERROR("Invalid FRD result type for job %d.\n", job_id);
      return false;
    }

    if (frd.result_block_type[type] != "DISP")
    {
      continue;
    }

    const int data_id = block[6];

    if (data_id < 0 || static_cast<std::size_t>(data_id) >= frd.result_block_data.size() || static_cast<std::size_t>(data_id) >= frd.result_block_node_data.size() || static_cast<std::size_t>(data_id) >= frd.result_block_components.size())
    {
      PRINT_ERROR("Invalid displacement result storage for job %d.\n", job_id);
      return false;
    }

    const auto& values = frd.result_block_data[data_id];
    const auto& mapping = frd.result_block_node_data[data_id];
    const auto& components = frd.result_block_components[data_id];

    if (values.empty() || mapping.size() != values.size())
    {
      PRINT_ERROR("Incomplete displacement node mapping for job %d.\n", job_id);
      return false;
    }

    std::array<std::size_t, 3> component_ids{};

    for (std::size_t axis = 0; axis < component_names.size(); ++axis)
    {
      const auto component = std::find(components.begin(), components.end(), component_names[axis]);

      if (component == components.end())
      {
        PRINT_ERROR("Displacement component %s is missing for job %d.\n", component_names[axis].c_str(), job_id);
        return false;
      }

      component_ids[axis] = static_cast<std::size_t>(component - components.begin());
    }

    std::set<int> used_rows;

    for (const auto& record : mapping)
    {
      if (record.size() < 2 || record[0] <= 0 || record[1] < 0 || static_cast<std::size_t>(record[1]) >= values.size())
      {
        PRINT_ERROR("Invalid displacement node record for job %d.\n", job_id);
        return false;
      }

      if (!used_rows.insert(record[1]).second)
      {
        PRINT_ERROR("Duplicate displacement data mapping for job %d.\n", job_id);
        return false;
      }

      const auto& row = values[record[1]];
      std::array<double, 3> displacement{};

      for (std::size_t axis = 0; axis < displacement.size(); ++axis)
      {
        if (component_ids[axis] >= row.size() || !std::isfinite(row[component_ids[axis]]))
        {
          PRINT_ERROR("Invalid displacement for node %d in job %d.\n", record[0], job_id);
          return false;
        }

        displacement[axis] = row[component_ids[axis]];
      }

      if (!measured.emplace(record[0], displacement).second)
      {
        PRINT_ERROR("Duplicate displacement result for node %d in job %d.\n", record[0], job_id);
        return false;
      }
    }
  }

  if (measured.empty())
  {
    PRINT_ERROR("Final-increment displacement results are missing for job %d.\n", job_id);
    return false;
  }

  std::set<int> required_nodes;

  for (const auto& assignment : collection->second.element_base_material)
  {
    const int element_id = assignment.first;

    if (!CubitInterface::get_element_exists(element_id))
    {
      PRINT_ERROR("Topology element %d no longer exists.\n", element_id);
      return false;
    }

    std::string entity = CubitInterface::get_element_type(element_id);

    if (entity != "hex" && entity != "tet" && entity != "wedge")
    {
      entity = get_cubit_element_type_entity(entity);
    }

    const int local_id = CubitInterface::get_element_type_id(element_id);

    if ((entity != "hex" && entity != "tet" && entity != "wedge") || local_id <= 0)
    {
      PRINT_ERROR("Invalid topology volume element %d.\n", element_id);
      return false;
    }

    const auto connectivity = CubitInterface::get_expanded_connectivity(entity, local_id);

    if (connectivity.empty())
    {
      PRINT_ERROR("Missing connectivity for topology element %d.\n", element_id);
      return false;
    }

    for (int node_id : connectivity)
    {
      if (node_id <= 0)
      {
        PRINT_ERROR("Invalid connectivity for topology element %d.\n", element_id);
        return false;
      }

      required_nodes.insert(node_id);
    }
  }

  for (int node_id : required_nodes)
  {
    if (measured.count(node_id) == 0)
    {
      PRINT_ERROR("Job %d is missing displacement results for topology node %d.\n", job_id, node_id);
      return false;
    }
  }

  displacements.swap(measured);

  return true;
}

bool CalculiXCore::check_topology_adjoint_results(int job_id)
{
  std::map<int, std::array<double, 3>> displacements;

  return read_topology_job_displacements(job_id, displacements);
}

int CalculiXCore::run_topology_iteration(int topology_id, int iteration_id)
{
  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization controller is not initialized.\n");
    return -1;
  }

  return topologyoptimization->run_topology_iteration(topology_id, iteration_id);
}

bool CalculiXCore::evaluate_topology_compliance(int topology_id, int iteration_id, const std::map<int, double>& energies)
{
  if (!topology || !mat || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology compliance evaluation request.\n");
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end())
  {
    PRINT_ERROR("Topology material data is missing.\n");
    return false;
  }

  const auto factors = collection->second.iteration_factors.find(iteration_id);

  if (factors == collection->second.iteration_factors.end())
  {
    PRINT_ERROR("Topology iteration stiffness factors are missing.\n");
    return false;
  }

  if (!topology->set_iteration_element_energies(topology_id, iteration_id, energies))
  {
    return false;
  }

  return topology->evaluate_compliance(topology_id, iteration_id, factors->second);
}

bool CalculiXCore::evaluate_topology_displacement(int topology_id, int iteration_id, double value, int node_id, const std::map<int, double>& element_integrals)
{
  if (!topology || !mat || !jobs || iteration_id < 1 || !std::isfinite(value) || value < 0.0 || node_id <= 0)
  {
    PRINT_ERROR("Invalid topology displacement evaluation request.\n");
    return false;
  }

  const int physical_job = get_topology_iteration_job_id(topology_id, iteration_id);

  if (physical_job <= 0 || jobs->get_job_status(physical_job) != 2)
  {
    PRINT_ERROR("Completed physical results are required for displacement evaluation.\n");
    return false;
  }

  if (value > 0.0)
  {
    const int adjoint_job = get_topology_iteration_adjoint_job_id(topology_id, iteration_id);

    if (adjoint_job <= 0 || jobs->get_job_status(adjoint_job) != 2)
    {
      PRINT_ERROR("Completed adjoint results are required for displacement evaluation.\n");
      return false;
    }
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Topology material data is missing.\n");
    return false;
  }

  if (element_integrals.size() != collection->second.element_base_material.size())
  {
    PRINT_ERROR("Incomplete topology displacement integrals.\n");
    return false;
  }

  for (const auto& assignment : collection->second.element_base_material)
  {
    if (element_integrals.count(assignment.first) == 0)
    {
      PRINT_ERROR("Missing displacement integral for element %d.\n", assignment.first);
      return false;
    }
  }

  const auto factors = collection->second.iteration_factors.find(iteration_id);

  if (factors == collection->second.iteration_factors.end())
  {
    PRINT_ERROR("Topology iteration stiffness factors are missing.\n");
    return false;
  }

  return topology->evaluate_displacement(topology_id, iteration_id, value, node_id, element_integrals, factors->second, collection->second.iteration_material_caps.count(iteration_id) != 0);
}

bool CalculiXCore::set_topology_iteration_displacement(int topology_id, int iteration_id, double displacement, int node_id)
{
  if (!topology || iteration_id < 1 || !std::isfinite(displacement) || displacement < 0.0 || node_id <= 0)
  {
    PRINT_ERROR("Invalid topology displacement measurement.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  auto& configuration = entry->second;
  const auto evaluation = configuration.iteration_evaluations.find(iteration_id);

  if (evaluation == configuration.iteration_evaluations.end() || configuration.displacement_nodeset <= 0 || configuration.displacement_component < 0 || configuration.displacement_component > 3)
  {
    PRINT_ERROR("Missing topology evaluation or invalid displacement selection.\n");
    return false;
  }

  evaluation->second.displacement_available = true;
  evaluation->second.displacement_value = displacement;
  evaluation->second.displacement_node = node_id;
  evaluation->second.displacement_nodeset = configuration.displacement_nodeset;
  evaluation->second.displacement_component = configuration.displacement_component;
  return true;
}

bool CalculiXCore::mark_topology_iteration_completed(int topology_id, int iteration_id)
{
  if (!topology || !jobs || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology iteration completion request.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto job = entry->second.iteration_jobs.find(iteration_id);

  if (job == entry->second.iteration_jobs.end() || jobs->get_job_status(job->second) != 2 || entry->second.iteration_evaluations.count(iteration_id) == 0)
  {
    PRINT_ERROR("Topology iteration is not ready for completion.\n");
    return false;
  }

  entry->second.selected_iteration = iteration_id;
  return true;
}

bool CalculiXCore::finish_topology_iteration(int topology_id, int iteration_id)
{
  return topologyoptimization && topologyoptimization->finish_topology_iteration(topology_id, iteration_id);
}

bool CalculiXCore::run_topology(int topology_id, bool wait)
{
  return topologyoptimization && topologyoptimization->run_topology(topology_id, wait);
}

bool CalculiXCore::begin_topology_run(int topology_id)
{
  if (!topology || !mat || !sections || !jobs || !steps || !historyoutputs || !me_iface)
  {
    PRINT_ERROR("Topology run cores are not initialized.\n");
    return false;
  }
  if (checking_jobs)
  {
    PRINT_ERROR("Job results are being processed. Try again shortly.\n");
    return false;
  }
  const auto entry = topology->topology_cases.find(topology_id);
  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }
  if (entry->second.run_state == TopologyRunState::Running)
  {
    PRINT_ERROR("Topology %d is already running.\n", topology_id);
    return false;
  }
  if (entry->second.method != TopologyMethod::Density ||
      (entry->second.objective != TopologyObjective::Compliance &&
       entry->second.objective != TopologyObjective::Displacement))
  {
    PRINT_ERROR("Topology currently supports density optimization with compliance or displacement objectives.\n");
    return false;
  }
  if (!validate_topology_compliance_model())
  {
    return false;
  }

  const int nodeset_id = entry->second.displacement_nodeset;
  const int component = entry->second.displacement_component;

  if (entry->second.objective == TopologyObjective::Displacement && nodeset_id <= 0)
  {
    PRINT_ERROR("Displacement optimization requires a displacement nodeset.\n");
    return false;
  }

  if (nodeset_id > 0)
  {
    if (component < 0 || component > 3)
    {
      PRINT_ERROR("Invalid topology displacement component.\n");
      return false;
    }

    if (!check_nodeset_exists(nodeset_id) || CubitInterface::get_nodeset_nodes_inclusive(nodeset_id).empty())
    {
      PRINT_ERROR("Topology displacement nodeset %d is missing or empty.\n", nodeset_id);
      return false;
    }
  }

  const QScopedValueRollback<bool> guard(checking_jobs, true);

  if (!reset_topology_run(topology_id))
  {
    return false;
  }

  entry->second.run_state = TopologyRunState::Running;
  return true;
}

bool CalculiXCore::wait_topology(int topology_id)
{
  return topologyoptimization && topologyoptimization->wait_topology(topology_id);
}

bool CalculiXCore::kill_topology(int topology_id)
{
  return topologyoptimization && topologyoptimization->kill_topology(topology_id);
}

bool CalculiXCore::get_topology_run_info(int topology_id, TopologyRunInfo& information)
{
  information = {};
  return topologyoptimization && topologyoptimization->get_topology_run_info(topology_id, information);
}

bool CalculiXCore::cancel_topology_runs()
{
  return !topologyoptimization || topologyoptimization->cancel_topology_runs();
}

bool CalculiXCore::prepare_topology_run(int topology_id)
{
  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return topologyoptimization->prepare_topology_run(topology_id);
}

bool CalculiXCore::collect_topology_run_sections(int topology_id, std::vector<int>& section_ids, std::set<int>& block_ids)
{
  section_ids.clear();
  block_ids.clear();

  if (!topology || !mat || !sections || !jobs || !steps || !historyoutputs || !me_iface)
  {
    PRINT_ERROR("Topology run cores are not initialized.\n");
    return false;
  }

  if (topology->topology_cases.find(topology_id) == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (!validate_topology_compliance_model())
  {
    return false;
  }

  for (const auto& section : sections->sections_data)
  {
    if (section.size() < 3)
    {
      PRINT_ERROR("Invalid section data.\n");
      return false;
    }

    if (section[1] != 1)
    {
      continue;
    }

    const int solid_index = sections->get_solid_section_data_id_from_solid_section_id(section[2]);

    if (solid_index < 0)
    {
      PRINT_ERROR("Solid section data missing for section %d.\n", section[0]);
      return false;
    }

    const int block_id = std::stoi(sections->solid_section_data[solid_index][1]);

    if (get_ccx_element_type(block_id).rfind("C3D", 0) == 0)
    {
      section_ids.push_back(section[0]);
      block_ids.insert(block_id);
    }
  }

  if (section_ids.empty())
  {
    PRINT_ERROR("Topology optimization requires solid sections containing volume elements.\n");
    return false;
  }

  return true;
}

bool CalculiXCore::prepare_topology_history_outputs(int topology_id, const std::set<int>& block_ids)
{
  if (!topology || !steps || !historyoutputs)
  {
    PRINT_ERROR("Topology history output cores are not initialized.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const int nodeset_id = entry->second.displacement_nodeset;
  const bool needs_displacement_sensitivities = entry->second.objective == TopologyObjective::Displacement || nodeset_id > 0;
  std::vector<int> step_ids;

  for (const auto& step : steps->steps_data)
  {
    if (step.empty())
    {
      PRINT_ERROR("Invalid step data.\n");
      return false;
    }

    step_ids.push_back(step[0]);
  }

  if (step_ids.empty())
  {
    PRINT_ERROR("Topology optimization requires an analysis step.\n");
    return false;
  }

  if (nodeset_id > 0)
  {
    if (!check_nodeset_exists(nodeset_id) || CubitInterface::get_nodeset_nodes_inclusive(nodeset_id).empty())
    {
      PRINT_ERROR("Topology displacement nodeset %d is missing or empty.\n", nodeset_id);
      return false;
    }
  }

  for (int step_id : step_ids)
  {
    for (int block_id : block_ids)
    {
      if (!ensure_topology_element_history_output(topology_id, step_id, block_id, "ELSE"))
      {
        return false;
      }

      if (needs_displacement_sensitivities)
      {
        if (!ensure_topology_element_history_output(topology_id, step_id, block_id, "S"))
        {
          return false;
        }

        if (!ensure_topology_element_history_output(topology_id, step_id, block_id, "ME"))
        {
          return false;
        }
      }
    }

    if (nodeset_id > 0)
    {
      if (!ensure_topology_node_history_output(topology_id, step_id, nodeset_id, "U"))
      {
        return false;
      }
    }
  }

  if (nodeset_id > 0)
  {
    if (!ensure_topology_displacement_field_output(topology_id, step_ids.back()))
    {
      return false;
    }
  }

  return true;
}

bool CalculiXCore::initialize_topology_run_design(int topology_id, const std::vector<int>& section_ids)
{
  if (!topology || !mat || !sections)
  {
    PRINT_ERROR("Topology initialization cores are not initialized.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (entry->second.method != TopologyMethod::Density ||
      (entry->second.objective != TopologyObjective::Compliance &&
       entry->second.objective != TopologyObjective::Displacement))
  {
    PRINT_ERROR("The topology iteration loop supports density optimization with compliance or displacement objectives.\n");
    return false;
  }

  const auto discard_prepared_data = [&]()
  {
    entry->second.element_preserved.clear();
    mat->topology_material_data.erase(topology_id);
    sections->topology_solid_sections.erase(topology_id);
  };

  if (!initialize_topology(topology_id, section_ids))
  {
    discard_prepared_data();
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end())
  {
    PRINT_ERROR("Topology material initialization failed.\n");
    discard_prepared_data();
    return false;
  }

  std::map<int, std::set<int>> default_sets;
  std::set<int> default_elements;

  if (!collect_topology_default_sets(default_sets, default_elements) || !resolve_topology_preservation(topology_id, default_sets, default_elements))
  {
    PRINT_ERROR("Failed to resolve preservation for topology %d.\n", topology_id);
    discard_prepared_data();
    return false;
  }

  std::map<int, double> densities;

  for (const auto& assignment : collection->second.element_base_material)
  {
    densities.emplace(assignment.first, 1.0);
  }

  if (!set_topology_iteration_densities(topology_id, 1, densities))
  {
    discard_prepared_data();
    return false;
  }

  return true;
}

bool CalculiXCore::initialize_topology(int topology_id,const std::vector<int>& section_ids)
{
  if (topology_id <= 0 || section_ids.empty())
  {
    PRINT_ERROR("Invalid topology initialization.\n");
    return false;
  }

  // Reset data belonging to this topology configuration.
  mat->topology_material_data.erase(topology_id);
  sections->topology_solid_sections.erase(topology_id);

  std::vector<int> element_ids;
  std::vector<int> base_material_ids;

  for (int section_id : section_ids)
  {
    const int section_index =
        sections->get_sections_data_id_from_section_id(section_id);

    if (section_index < 0 || sections->sections_data[section_index][1] != 1)
    {
      PRINT_ERROR("Section %d is not an existing solid section.\n", section_id);
      return false;
    }

    const int solid_index = sections->get_solid_section_data_id_from_solid_section_id(sections->sections_data[section_index][2]);

    if (solid_index < 0)
    {
      PRINT_ERROR("Solid section data missing for section %d.\n", section_id);
      return false;
    }

    const auto& solid = sections->solid_section_data[solid_index];
    const int block_id = std::stoi(solid[1]);
    const int material_id = std::stoi(solid[2]);

    if (cb->get_blocks_data_id_from_block_id(block_id) < 0)
    {
      PRINT_ERROR("Block %d for section %d does not exist.\n", block_id, section_id);
      return false;
    }

    const auto block_elements = get_block_element_ids(block_id);

    if (block_elements.empty())
    {
      PRINT_ERROR("Block %d contains no elements.\n", block_id);
      return false;
    }

    for (int element_id : block_elements)
    {
      element_ids.push_back(element_id);
      base_material_ids.push_back(material_id);
    }
  }

  if (!mat->initialize_topology_materials(topology_id, element_ids, base_material_ids))
  {
    PRINT_ERROR("Failed to initialize materials for topology %d.\n", topology_id);
    return false;
  }

  if (!sections->initialize_topology_solid_sections(topology_id, section_ids))
  {
    // Remove only the collection created by this call.
    mat->topology_material_data.erase(topology_id);
    return false;
  }

  PRINT_INFO("Topology %d initialized with %d elements.\n", topology_id, static_cast<int>(element_ids.size()));

  return true;
}

std::vector<std::vector<int>> CalculiXCore::get_topology_element_preservation(int topology_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return {};
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return {};
  }

  const auto& flags = entry->second.element_preserved;

  if (flags.empty())
  {
    PRINT_ERROR("Preservation for topology %d has not been resolved.\n", topology_id);
    return {};
  }

  std::vector<std::vector<int>> rows;
  rows.reserve(flags.size());

  for (const auto& flag : flags)
  {
    rows.push_back({flag.first, flag.second ? 1 : 0});
  }

  return rows;
}

bool CalculiXCore::set_topology_iteration_factors(int topology_id,int iteration_id,const std::vector<int>& element_ids,const std::vector<double>& stiffness_factors)
{
  if (!topology || !mat)
  {
    PRINT_ERROR("Topology/material cores are not initialized.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  if (element_ids.empty() || element_ids.size() != stiffness_factors.size())
  {
    PRINT_ERROR("Invalid topology iteration factor data.\n");
    return false;
  }

  const auto& preserved = entry->second.element_preserved;

  if (preserved.empty())
  {
    PRINT_ERROR("Preservation for topology %d has not been resolved.\n", topology_id);
    return false;
  }

  // Keep the caller's data unchanged.
  std::vector<double> factors = stiffness_factors;

  for (size_t i = 0; i < element_ids.size(); ++i)
  {
    const int element_id = element_ids[i];
    const auto flag = preserved.find(element_id);

    if (flag == preserved.end())
    {
      PRINT_ERROR("Missing preservation flag for element %d.\n", element_id);
      return false;
    }

    if (!std::isfinite(factors[i]) ||
        factors[i] <= 0.0 || factors[i] > 1.0)
    {
      PRINT_ERROR("Invalid stiffness factor for element %d.\n", element_id);
      return false;
    }

    if (flag->second)
    {
      factors[i] = 1.0;
    }
  }

  if (!mat->set_topology_iteration_factors(topology_id, iteration_id, element_ids, factors, entry->second.material_cap))
  {
    PRINT_ERROR("Failed to store topology %d iteration %d factors.\n", topology_id, iteration_id);
    return false;
  }

  return true;
}

bool CalculiXCore::check_topology_material_assignment(int topology_id, int iteration_id, int element_id)
{
  const auto collection_it = mat->topology_material_data.find(topology_id);

  if (collection_it == mat->topology_material_data.end())
  {
    return false;
  }

  const auto& collection = collection_it->second;
  const auto assignment = collection.element_base_material.find(element_id);
  const auto iteration = collection.iteration_factors.find(iteration_id);

  if (assignment == collection.element_base_material.end() || iteration == collection.iteration_factors.end())
  {
    return false;
  }

  return collection.base_materials.count(assignment->second) != 0 && iteration->second.count(element_id) != 0;
}

bool CalculiXCore::get_topology_group_material_name(int topology_id, int iteration_id, int element_id, std::string& material_name)
{
  if (!mat)
  {
    PRINT_ERROR("Topology material core is not initialized.\n");
    return false;
  }

  return mat->get_topology_group_material_name(topology_id, iteration_id, element_id, material_name);
}

std::string CalculiXCore::get_topology_material_name(int topology_id, int element_id)
{
  return mat->get_topology_material_name(topology_id, element_id);
}

std::vector<double> CalculiXCore::get_topology_parameters(int topology_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return {};
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return {};
  }

  const auto& configuration = entry->second;

  return {
      configuration.target_volume_fraction,
      configuration.void_stiffness_ratio,
      static_cast<double>(configuration.maximum_iterations)
  };
}

bool CalculiXCore::get_topology_settings(int topology_id, TopologyOptions& options)
{
  options = {};

  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto& configuration = entry->second;

  options.has_name = true;
  options.name = configuration.name;

  options.has_method = true;
  options.method = configuration.method;

  options.has_objective = true;
  options.objective = configuration.objective;

  options.has_volume_fraction = true;
  options.volume_fraction = configuration.target_volume_fraction;

  options.has_void_ratio = true;
  options.void_ratio = configuration.void_stiffness_ratio;

  options.has_maximum_iterations = true;
  options.maximum_iterations = configuration.maximum_iterations;

  options.has_material_cap = true;
  options.material_cap = configuration.material_cap;

  options.has_filter_radius = true;
  options.filter_radius = configuration.filter_radius;

  options.has_move_limit = true;
  options.move_limit = configuration.move_limit;

  options.has_adaptive_move_limits = true;
  options.adaptive_move_limits = configuration.adaptive_move_limits;

  options.has_minimum_move_limit = true;
  options.minimum_move_limit = configuration.minimum_move_limit;

  options.has_density_tolerance = true;
  options.density_tolerance = configuration.density_tolerance;

  options.has_compliance_tolerance = true;
  options.compliance_tolerance = configuration.compliance_tolerance;

  options.has_displacement_nodeset = true;
  options.displacement_nodeset = configuration.displacement_nodeset;

  options.has_displacement_component = true;
  options.displacement_component = configuration.displacement_component;

  return true;
}

int CalculiXCore::get_topology_iteration_job_id(int topology_id, int iteration_id)
{
  if (!topology || !jobs || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology iteration query.\n");
    return -1;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return -1;
  }

  const auto iteration = entry->second.iteration_jobs.find(iteration_id);

  if (iteration == entry->second.iteration_jobs.end())
  {
    PRINT_ERROR("Topology %d iteration %d has no job.\n", topology_id, iteration_id);
    return -1;
  }

  const int job_id = iteration->second;

  if (jobs->get_jobs_data_id_from_job_id(job_id) < 0)
  {
    PRINT_ERROR("Topology iteration job %d no longer exists.\n", job_id);
    return -1;
  }

  return job_id;
}

int CalculiXCore::get_topology_iteration_adjoint_job_id(int topology_id, int iteration_id)
{
  if (!topology || !jobs || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology adjoint job query.\n");
    return -1;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return -1;
  }

  const auto job = entry->second.iteration_adjoint_jobs.find(iteration_id);

  if (job == entry->second.iteration_adjoint_jobs.end() || jobs->get_jobs_data_id_from_job_id(job->second) < 0)
  {
    PRINT_ERROR("Topology %d iteration %d has no available adjoint job.\n", topology_id, iteration_id);
    return -1;
  }

  return job->second;
}

bool CalculiXCore::ensure_topology_element_history_output(int topology_id,int step_id,int block_id,const std::string& key)
{
  if (!historyoutputs || !steps || !check_block_exists(block_id))
  {
    PRINT_ERROR("Invalid topology history output request.\n");
    return false;
  }

  const auto key_it = std::find(historyoutputs->element_keys.begin(),historyoutputs->element_keys.end(), key);

  if (key_it == historyoutputs->element_keys.end())
  {
    PRINT_ERROR("Unknown element history output key '%s'.\n", key.c_str());
    return false;
  }

  const size_t key_column = 7 + static_cast<size_t>(key_it - historyoutputs->element_keys.begin());
  const bool tensor_output = key == "S" || key == "ME";

  const int step_index = steps->get_steps_data_id_from_step_id(step_id);

  if (step_index < 0 || steps->steps_data[step_index].size() < 8)
  {
    PRINT_ERROR("Missing step %d.\n", step_id);
    return false;
  }

  // Copy IDs before executing commands that may change the vectors.
  std::vector<int> assigned_ids;

  for (int index : steps->get_historyoutput_data_ids_from_historyoutputs_id(steps->steps_data[step_index][7]))
  {
    const auto& assignment = steps->historyoutputs_data[index];

    if (assignment.size() < 2)
    {
      PRINT_ERROR("Invalid step history output assignment.\n");
      return false;
    }

    assigned_ids.push_back(assignment[1]);
  }

  const auto element_index = [&](int output_id) -> int
  {
    const int index = historyoutputs->get_outputs_data_id_from_output_id(output_id);

    if (index < 0)
    {
      return -1;
    }

    const auto& output = historyoutputs->outputs_data[index];

    if (output.size() < 4 || output[2] != 2)
    {
      return -1;
    }

    const int element = historyoutputs->get_element_data_id_from_element_id(output[3]);

    if (element < 0)
    {
      return -1;
    }

    const auto& data = historyoutputs->element_data[element];

    if (data.size() <= key_column || data[1] != std::to_string(block_id))
    {
      return -1;
    }

    return element;
  };

  // Prefer an output that already provides individual element values.
  const auto choose_output = [&](const std::vector<int>& ids) -> int
  {
    int fallback = -1;

    for (int id : ids)
    {
      const int index = element_index(id);

      if (index < 0)
      {
        continue;
      }

      if (fallback < 0)
      {
        fallback = id;
      }

      const auto& data = historyoutputs->element_data[index];

      if (data[key_column] == key && data[4] != "ONLY")
      {
        return id;
      }
    }

    return fallback;
  };

  int output_id = choose_output(assigned_ids);
  std::vector<int> existing_ids;

  for (const auto& output : historyoutputs->outputs_data)
  {
    if (output.size() < 4)
    {
      PRINT_ERROR("Invalid history output data.\n");
      return false;
    }
    existing_ids.push_back(output[0]);
  }

  if (output_id < 0)
  {
    output_id = choose_output(existing_ids);
  }

  bool created = false;

  if (output_id < 0)
  {
    const std::string name = "Topology_" + std::to_string(topology_id) + "_Block_" + std::to_string(block_id) + "_" + key;

    if (!silent_cmd("ccx create historyoutput name " + name + " element"))
    {
      return false;
    }

    // Discover the created ID and verify creation actually happened.
    for (const auto& output : historyoutputs->outputs_data)
    {
      if (output.empty() || std::find(existing_ids.begin(), existing_ids.end(),output[0]) != existing_ids.end())
      {
        continue;
      }

      if (output_id >= 0)
      {
        PRINT_ERROR("Unexpected history output creation result.\n");
        return false;
      }

      output_id = output[0];
    }

    if (output_id < 0)
    {
      PRINT_ERROR("Automatic history output creation failed.\n");
      return false;
    }

    created = true;

    std::string command = "ccx modify historyoutput " + std::to_string(output_id) + " element block " + std::to_string(block_id) + " totals_no key_on " + key;

    if (tensor_output)
    {
      command += " global_yes frequency 1";
    }

    if (!silent_cmd(command))
    {
      return false;
    }
  }

  int index = element_index(output_id);

  if (index < 0)
  {
    PRINT_ERROR("Automatic history output configuration failed.\n");
    return false;
  }

  const bool missing_key = historyoutputs->element_data[index][key_column] != key;
  const bool totals_only = historyoutputs->element_data[index][4] == "ONLY";
  const bool needs_tensor_settings = tensor_output && (historyoutputs->element_data[index][5] != "YES" || historyoutputs->element_data[index][2] != "1");

  if (missing_key || totals_only || needs_tensor_settings)
  {
    std::string command = "ccx modify historyoutput " + std::to_string(output_id) + " element";

    if (totals_only)
    {
      command += " totals_yes";
    }

    command += " key_on " + key;

    if (tensor_output)
    {
      command += " global_yes frequency 1";
    }

    if (!silent_cmd(command))
    {
      return false;
    }
  }

  index = element_index(output_id);

  if (index < 0 ||
      historyoutputs->element_data[index][key_column] != key ||
      historyoutputs->element_data[index][4] == "ONLY")
  {
    PRINT_ERROR("History output %d does not provide element %s values.\n", output_id, key.c_str());
    return false;
  }

  if (tensor_output && (historyoutputs->element_data[index][5] != "YES" || historyoutputs->element_data[index][2] != "1"))
  {
    PRINT_ERROR("History output %d does not provide global element %s values at every increment.\n", output_id, key.c_str());
    return false;
  }

  const bool needs_assignment = std::find(assigned_ids.begin(), assigned_ids.end(), output_id) == assigned_ids.end();

  if (needs_assignment)
  {
    if (!silent_cmd("ccx step " + std::to_string(step_id) + " add historyoutput " + std::to_string(output_id)))
    {
      return false;
    }
  }

  // Re-query after the commands.
  const int updated_step = steps->get_steps_data_id_from_step_id(step_id);

  bool assigned = false;

  if (updated_step >= 0 && steps->steps_data[updated_step].size() >= 8)
  {
    for (int assignment : steps->get_historyoutput_data_ids_from_historyoutputs_id(steps->steps_data[updated_step][7]))
    {
      const auto& data = steps->historyoutputs_data[assignment];

      if (data.size() >= 2 && data[1] == output_id)
      {
        assigned = true;
      }
    }
  }

  if (!assigned)
  {
    PRINT_ERROR("History output %d was not assigned to step %d.\n", output_id, step_id);
    return false;
  }

  if (created)
  {
    PRINT_INFO("Automatically created history output %d for block %d with key %s.\n", output_id, block_id, key.c_str());
  }
  else if (missing_key || totals_only || needs_tensor_settings)
  {
    PRINT_INFO("Automatically configured history output %d to provide element %s values.\n", output_id, key.c_str());
  }
  if (needs_assignment)
  {
    PRINT_INFO("Automatically assigned history output %d to step %d.\n", output_id, step_id);
  }
  return true;
}

bool CalculiXCore::ensure_topology_displacement_field_output(int topology_id, int step_id)
{
  if (!fieldoutputs || !steps)
  {
    PRINT_ERROR("Topology field output interfaces are not initialized.\n");
    return false;
  }

  const auto key = std::find(fieldoutputs->node_keys.begin(), fieldoutputs->node_keys.end(), "U");

  if (key == fieldoutputs->node_keys.end())
  {
    PRINT_ERROR("Displacement field output key U is unavailable.\n");
    return false;
  }

  const std::size_t key_column = 11 + static_cast<std::size_t>(key - fieldoutputs->node_keys.begin());
  const int step_index = steps->get_steps_data_id_from_step_id(step_id);

  if (step_index < 0 || steps->steps_data[step_index].size() < 9)
  {
    PRINT_ERROR("Missing or invalid step %d.\n", step_id);
    return false;
  }

  std::set<int> assigned_ids;

  for (int index : steps->get_fieldoutput_data_ids_from_fieldoutputs_id(steps->steps_data[step_index][8]))
  {
    if (index < 0 || static_cast<std::size_t>(index) >= steps->fieldoutputs_data.size() || steps->fieldoutputs_data[index].size() < 2)
    {
      PRINT_ERROR("Invalid field output assignment for step %d.\n", step_id);
      return false;
    }

    assigned_ids.insert(steps->fieldoutputs_data[index][1]);
  }

  const auto compatible_node = [&](int output_id) -> int
  {
    const int index = fieldoutputs->get_outputs_data_id_from_output_id(output_id);

    if (index < 0)
    {
      return -1;
    }

    const auto& output = fieldoutputs->outputs_data[index];

    if (output.size() < 4 || output[2] != 1)
    {
      return -1;
    }

    const int node_index = fieldoutputs->get_node_data_id_from_node_id(output[3]);

    if (node_index < 0)
    {
      return -1;
    }

    const auto& node = fieldoutputs->node_data[node_index];

    if (node.size() < 11 + fieldoutputs->node_keys.size())
    {
      return -1;
    }

    if (!node[1].empty() || !node[3].empty() || !node[8].empty() || !node[9].empty() || !node[10].empty())
    {
      return -1;
    }

    if ((!node[5].empty() && node[5] != "YES") || (!node[6].empty() && node[6] != "OUTPUT=3D"))
    {
      return -1;
    }

    return node_index;
  };

  int output_id = -1;

  // Prefer a complete output already assigned to this step.
  for (int candidate : assigned_ids)
  {
    const int node_index = compatible_node(candidate);

    if (node_index >= 0)
    {
      const auto& node = fieldoutputs->node_data[node_index];

      if (node[5] == "YES" && node[2] == "1" && node[key_column] == "U")
      {
        return true;
      }

      if (output_id < 0)
      {
        output_id = candidate;
      }
    }
  }

  std::set<int> existing_ids;

  for (const auto& output : fieldoutputs->outputs_data)
  {
    if (output.size() >= 4)
    {
      existing_ids.insert(output[0]);

      if (output_id < 0 && compatible_node(output[0]) >= 0)
      {
        output_id = output[0];
      }
    }
  }

  if (output_id < 0)
  {
    const std::string name = "Topology_" + std::to_string(topology_id) + "_Displacement";

    if (!silent_cmd("ccx create fieldoutput name " + name + " node"))
    {
      return false;
    }

    for (const auto& output : fieldoutputs->outputs_data)
    {
      if (output.size() >= 4 && existing_ids.count(output[0]) == 0 && compatible_node(output[0]) >= 0)
      {
        output_id = output[0];
        break;
      }
    }

    if (output_id < 0)
    {
      PRINT_ERROR("Unable to identify the created topology field output.\n");
      return false;
    }

    PRINT_INFO("Automatically created field output %d for topology displacement results.\n", output_id);
  }

  const int node_index = compatible_node(output_id);

  if (node_index < 0)
  {
    PRINT_ERROR("Invalid topology displacement field output.\n");
    return false;
  }

  const auto& node = fieldoutputs->node_data[node_index];
  const bool needs_configuration = node[5] != "YES" || node[2] != "1" || node[key_column] != "U";

  if (needs_configuration)
  {
    if (!silent_cmd("ccx modify fieldoutput " + std::to_string(output_id) + " node global_yes frequency 1 key_on U"))
    {
      return false;
    }

    PRINT_INFO("Automatically configured field output %d for global displacement results.\n", output_id);
  }

  const int configured_index = compatible_node(output_id);

  if (configured_index < 0 || fieldoutputs->node_data[configured_index][5] != "YES" || fieldoutputs->node_data[configured_index][2] != "1" || fieldoutputs->node_data[configured_index][key_column] != "U")
  {
    PRINT_ERROR("Topology displacement field output configuration failed.\n");
    return false;
  }

  if (assigned_ids.count(output_id) == 0)
  {
    if (!silent_cmd("ccx step " + std::to_string(step_id) + " add fieldoutput " + std::to_string(output_id)))
    {
      return false;
    }

    const int updated_step = steps->get_steps_data_id_from_step_id(step_id);
    bool assigned = false;

    if (updated_step >= 0 && steps->steps_data[updated_step].size() >= 9)
    {
      for (int index : steps->get_fieldoutput_data_ids_from_fieldoutputs_id(steps->steps_data[updated_step][8]))
      {
        if (index >= 0 && static_cast<std::size_t>(index) < steps->fieldoutputs_data.size() && steps->fieldoutputs_data[index].size() >= 2 && steps->fieldoutputs_data[index][1] == output_id)
        {
          assigned = true;
          break;
        }
      }
    }

    if (!assigned)
    {
      PRINT_ERROR("Topology displacement field output assignment failed.\n");
      return false;
    }

    PRINT_INFO("Automatically assigned field output %d to step %d.\n", output_id, step_id);
  }

  return true;
}

bool CalculiXCore::ensure_topology_node_history_output(int topology_id,int step_id,int nodeset_id,const std::string& key)
{
  if (!historyoutputs || !steps || !check_nodeset_exists(nodeset_id))
  {
    PRINT_ERROR("Invalid topology history output request.\n");
    return false;
  }

  const auto key_it = std::find(historyoutputs->node_keys.begin(),historyoutputs->node_keys.end(), key);

  if (key_it == historyoutputs->node_keys.end())
  {
    PRINT_ERROR("Unknown node history output key '%s'.\n", key.c_str());
    return false;
  }

  const size_t key_column = 7 + static_cast<size_t>(key_it - historyoutputs->node_keys.begin());

  const int step_index = steps->get_steps_data_id_from_step_id(step_id);

  if (step_index < 0 || steps->steps_data[step_index].size() < 8)
  {
    PRINT_ERROR("Missing step %d.\n", step_id);
    return false;
  }

  // Copy IDs before executing commands that may change the vectors.
  std::vector<int> assigned_ids;

  for (int index : steps->get_historyoutput_data_ids_from_historyoutputs_id(steps->steps_data[step_index][7]))
  {
    const auto& assignment = steps->historyoutputs_data[index];

    if (assignment.size() < 2)
    {
      PRINT_ERROR("Invalid step history output assignment.\n");
      return false;
    }

    assigned_ids.push_back(assignment[1]);
  }

  const auto node_index = [&](int output_id) -> int
  {
    const int index = historyoutputs->get_outputs_data_id_from_output_id(output_id);

    if (index < 0)
    {
      return -1;
    }

    const auto& output = historyoutputs->outputs_data[index];

    if (output.size() < 4 || output[2] != 1)
    {
      return -1;
    }

    const int node = historyoutputs->get_node_data_id_from_node_id(output[3]);

    if (node < 0)
    {
      return -1;
    }

    const auto& data = historyoutputs->node_data[node];

    if (data.size() <= key_column || data[1] != std::to_string(nodeset_id))
    {
      return -1;
    }

    return node;
  };

  // Prefer an output that already provides individual nodal values.
  const auto choose_output = [&](const std::vector<int>& ids) -> int
  {
    int fallback = -1;

    for (int id : ids)
    {
      const int index = node_index(id);

      if (index < 0)
      {
        continue;
      }

      if (fallback < 0)
      {
        fallback = id;
      }

      const auto& data = historyoutputs->node_data[index];

      if (data[key_column] == key && data[4] != "ONLY")
      {
        return id;
      }
    }

    return fallback;
  };

  int output_id = choose_output(assigned_ids);
  std::vector<int> existing_ids;

  for (const auto& output : historyoutputs->outputs_data)
  {
    if (output.size() < 4)
    {
      PRINT_ERROR("Invalid history output data.\n");
      return false;
    }
    existing_ids.push_back(output[0]);
  }

  if (output_id < 0)
  {
    output_id = choose_output(existing_ids);
  }

  bool created = false;

  if (output_id < 0)
  {
    const std::string name = "Topology_" + std::to_string(topology_id) + "_Nodeset_" + std::to_string(nodeset_id) + "_" + key;

    if (!silent_cmd("ccx create historyoutput name " + name + " node"))
    {
      return false;
    }

    // Discover the created ID and verify creation actually happened.
    for (const auto& output : historyoutputs->outputs_data)
    {
      if (output.empty() || std::find(existing_ids.begin(), existing_ids.end(),output[0]) != existing_ids.end())
      {
        continue;
      }

      if (output_id >= 0)
      {
        PRINT_ERROR("Unexpected history output creation result.\n");
        return false;
      }

      output_id = output[0];
    }

    if (output_id < 0)
    {
      PRINT_ERROR("Automatic history output creation failed.\n");
      return false;
    }

    created = true;

    if (!silent_cmd("ccx modify historyoutput " + std::to_string(output_id) + " node nodeset " + std::to_string(nodeset_id) + " totals_no key_on " + key))
    {
      return false;
    }
  }

  int index = node_index(output_id);

  if (index < 0)
  {
    PRINT_ERROR("Automatic history output configuration failed.\n");
    return false;
  }

  const bool missing_key = historyoutputs->node_data[index][key_column] != key;

  const bool totals_only = historyoutputs->node_data[index][4] == "ONLY";

  if (missing_key || totals_only)
  {
    std::string command = "ccx modify historyoutput " + std::to_string(output_id) + " node";

    if (totals_only)
    {
      command += " totals_yes";
    }

    command += " key_on " + key;

    if (!silent_cmd(command))
    {
      return false;
    }
  }

  index = node_index(output_id);

  if (index < 0 ||
      historyoutputs->node_data[index][key_column] != key ||
      historyoutputs->node_data[index][4] == "ONLY")
  {
    PRINT_ERROR("History output %d does not provide nodal %s values.\n", output_id, key.c_str());
    return false;
  }

  index = node_index(output_id);

  if (index < 0)
  {
    PRINT_ERROR("Invalid nodal history output configuration.\n");
    return false;
  }

  const bool configure_coordinates = historyoutputs->node_data[index][5] != "YES";
  const bool configure_frequency = historyoutputs->node_data[index][2] != "1";

  if (configure_coordinates || configure_frequency)
  {
    const std::string command = "ccx modify historyoutput " + std::to_string(output_id) + " node global_yes frequency 1";

    if (!silent_cmd(command))
    {
      return false;
    }

    index = node_index(output_id);

    if (index < 0 || historyoutputs->node_data[index][5] != "YES" || historyoutputs->node_data[index][2] != "1")
    {
      PRINT_ERROR("Unable to configure nodal displacement history output.\n");
      return false;
    }

    PRINT_INFO("Automatically configured history output %d for global nodal values at every increment.\n", output_id);
  }

  const bool needs_assignment = std::find(assigned_ids.begin(), assigned_ids.end(), output_id) == assigned_ids.end();

  if (needs_assignment)
  {
    if (!silent_cmd("ccx step " + std::to_string(step_id) + " add historyoutput " + std::to_string(output_id)))
    {
      return false;
    }
  }

  // Re-query after the commands.
  const int updated_step = steps->get_steps_data_id_from_step_id(step_id);

  bool assigned = false;

  if (updated_step >= 0 && steps->steps_data[updated_step].size() >= 8)
  {
    for (int assignment : steps->get_historyoutput_data_ids_from_historyoutputs_id(steps->steps_data[updated_step][7]))
    {
      const auto& data = steps->historyoutputs_data[assignment];

      if (data.size() >= 2 && data[1] == output_id)
      {
        assigned = true;
      }
    }
  }

  if (!assigned)
  {
    PRINT_ERROR("History output %d was not assigned to step %d.\n", output_id, step_id);
    return false;
  }

  if (created)
  {
    PRINT_INFO("Automatically created history output %d for nodeset %d with key %s.\n", output_id, nodeset_id, key.c_str());
  }
  else if (missing_key || totals_only)
  {
    PRINT_INFO("Automatically configured history output %d to provide nodal %s values.\n", output_id, key.c_str());
  }
  if (needs_assignment)
  {
    PRINT_INFO("Automatically assigned history output %d to step %d.\n", output_id, step_id);
  }
  return true;
}

bool CalculiXCore::read_topology_displacement(int topology_id, int iteration_id, double& value, int& node_id, std::array<double, 3>* critical_displacement)
{
  value = 0.0;
  node_id = -1;

  if (critical_displacement != nullptr)
  {
    critical_displacement->fill(0.0);
  }

  if (!topology || !results || !jobs)
  {
    PRINT_ERROR("Topology displacement interfaces are not initialized.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto& configuration = entry->second;
  const int nodeset_id = configuration.displacement_nodeset;
  const int component = configuration.displacement_component;
  const int job_id = get_topology_iteration_job_id(topology_id, iteration_id);

  if (nodeset_id <= 0 || component < 0 || component > 3 || job_id <= 0 || jobs->get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Invalid topology displacement request.\n");
    return false;
  }

  const int dat_index = results->get_dat_data_id_from_job_id(job_id);

  if (dat_index < 0)
  {
    PRINT_ERROR("Missing DAT results for topology displacement measurement.\n");
    return false;
  }

  auto& dat = results->dat_data[dat_index];
  const auto nodes = CubitInterface::get_nodeset_nodes_inclusive(nodeset_id);
  const std::set<int> required(nodes.begin(), nodes.end());
  const int final_step = dat.current_step;
  const int final_increment = dat.current_increment;

  if (required.empty() || final_step <= 0 || final_increment <= 0)
  {
    PRINT_ERROR("Missing displacement nodes or final solver increment.\n");
    return false;
  }

  const QString expected_set = QString::fromStdString(get_nodeset_name(nodeset_id));
  std::map<int, std::array<double, 3>> measured;

  for (const auto& block : dat.result_blocks)
  {
    if (block.size() < 7)
    {
      PRINT_ERROR("Invalid DAT result block.\n");
      return false;
    }

    if (block[5] != final_step || block[6] != final_increment)
    {
      continue;
    }

    const int type = block[2];
    const int set = block[3];
    const int data = block[4];

    if (type < 0 || static_cast<size_t>(type) >= dat.result_block_type.size() || set < 0 || static_cast<size_t>(set) >= dat.result_block_set.size())
    {
      PRINT_ERROR("Invalid DAT displacement block reference.\n");
      return false;
    }

    if (dat.erase_whitespace(dat.result_block_type[type]) != "displacements")
    {
      continue;
    }

    if (QString::compare(QString::fromStdString(dat.result_block_set[set]), expected_set, Qt::CaseInsensitive) != 0)
    {
      continue;
    }

    if (data < 0 || static_cast<size_t>(data) >= dat.result_block_components.size() || static_cast<size_t>(data) >= dat.result_block_data.size() || static_cast<size_t>(data) >= dat.result_block_c1_data.size())
    {
      PRINT_ERROR("Invalid DAT displacement data reference.\n");
      return false;
    }

    const auto& components = dat.result_block_components[data];
    std::array<int, 3> columns{};
    const std::array<std::string, 3> names = {"vx", "vy", "vz"};

    for (std::size_t axis = 0; axis < names.size(); ++axis)
    {
      const auto found = std::find(components.begin(), components.end(), names[axis]);

      if (found == components.end())
      {
        PRINT_ERROR("Missing displacement component %s.\n", names[axis].c_str());
        return false;
      }

      columns[axis] = static_cast<int>(found - components.begin());
    }

    for (const auto& record : dat.result_block_c1_data[data])
    {
      if (record.size() < 3)
      {
        PRINT_ERROR("Invalid DAT displacement record.\n");
        return false;
      }

      if (record[2] != 1 || required.count(record[0]) == 0)
      {
        continue;
      }

      const int row = record[1];

      if (row < 0 || static_cast<size_t>(row) >= dat.result_block_data[data].size())
      {
        PRINT_ERROR("Invalid DAT displacement row.\n");
        return false;
      }

      const auto& values = dat.result_block_data[data][row];
      std::array<double, 3> displacement{};

      for (std::size_t axis = 0; axis < columns.size(); ++axis)
      {
        if (static_cast<size_t>(columns[axis]) >= values.size() || !std::isfinite(values[columns[axis]]))
        {
          PRINT_ERROR("Missing or invalid displacement for node %d.\n", record[0]);
          return false;
        }

        displacement[axis] = values[columns[axis]];
      }

      const double measured_value = component == 0 ? std::hypot(displacement[0], displacement[1], displacement[2]) : std::abs(displacement[component - 1]);

      if (!std::isfinite(measured_value) || !measured.emplace(record[0], displacement).second)
      {
        PRINT_ERROR("Invalid or duplicate displacement for node %d.\n", record[0]);
        return false;
      }
    }
  }

  if (measured.size() != required.size())
  {
    PRINT_ERROR("Incomplete nodal displacements in the final solver step/increment.\n");
    return false;
  }

  for (const auto& measurement : measured)
  {
    const auto& displacement = measurement.second;
    const double measured_value = component == 0 ? std::hypot(displacement[0], displacement[1], displacement[2]) : std::abs(displacement[component - 1]);

    if (node_id < 0 || measured_value > value)
    {
      value = measured_value;
      node_id = measurement.first;

      if (critical_displacement != nullptr)
      {
        *critical_displacement = displacement;
      }
    }
  }

  return true;
}

bool CalculiXCore::read_topology_job_element_tensors(int job_id, const std::string& key, std::map<int, std::map<int, std::array<double, 6>>>& tensors)
{
  tensors.clear();

  if (key != "S" && key != "ME")
  {
    PRINT_ERROR("Topology tensor reader supports S and ME only.\n");
    return false;
  }

  if (!topology || !mat || !results || !jobs || job_id <= 0 || jobs->get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Topology tensor results require a completed job.\n");
    return false;
  }

  int topology_id = -1;
  int iteration_id = -1;

  if (!get_job_topology_context(job_id, topology_id, iteration_id))
  {
    return false;
  }

  if (topology_id <= 0 || iteration_id < 1)
  {
    PRINT_ERROR("Job %d is not associated with a topology iteration.\n", job_id);
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Prepared elements for topology %d are missing.\n", topology_id);
    return false;
  }

  const int dat_index = results->get_dat_data_id_from_job_id(job_id);

  if (dat_index < 0)
  {
    PRINT_ERROR("DAT results for job %d are missing.\n", job_id);
    return false;
  }

  auto& dat = results->dat_data[dat_index];
  const int final_step = dat.current_step;
  const int final_increment = dat.current_increment;

  if (final_step <= 0 || final_increment <= 0)
  {
    PRINT_ERROR("Final solver step/increment is missing for job %d.\n", job_id);
    return false;
  }

  const std::string expected_type = key == "S" ? "stresses" : "mechanicalstrains";
  const std::array<std::string, 6> component_names = key == "S" ? std::array<std::string, 6>{"sxx", "syy", "szz", "sxy", "sxz", "syz"} : std::array<std::string, 6>{"exx", "eyy", "ezz", "exy", "exz", "eyz"};
  const auto& required_elements = collection->second.element_base_material;

  std::map<int, std::map<int, std::array<double, 6>>> collected;

  for (const auto& block : dat.result_blocks)
  {
    if (block.size() < 7)
    {
      PRINT_ERROR("Invalid DAT result block for job %d.\n", job_id);
      return false;
    }

    if (block[5] != final_step || block[6] != final_increment)
    {
      continue;
    }

    const int type_id = block[2];

    if (type_id < 0 || static_cast<std::size_t>(type_id) >= dat.result_block_type.size())
    {
      PRINT_ERROR("Invalid DAT result type for job %d.\n", job_id);
      return false;
    }

    if (dat.erase_whitespace(dat.result_block_type[type_id]) != expected_type)
    {
      continue;
    }

    const int data_id = block[4];

    if (data_id < 0 || static_cast<std::size_t>(data_id) >= dat.result_block_data.size() || static_cast<std::size_t>(data_id) >= dat.result_block_c1_data.size() || static_cast<std::size_t>(data_id) >= dat.result_block_components.size())
    {
      PRINT_ERROR("Invalid DAT tensor storage for job %d.\n", job_id);
      return false;
    }

    const auto& components = dat.result_block_components[data_id];

    const auto find_column = [&](const std::string& name) -> int
    {
      int column = -1;

      for (std::size_t index = 0; index < components.size(); ++index)
      {
        if (dat.erase_whitespace(components[index]) == name)
        {
          if (column >= 0)
          {
            return -1;
          }

          column = static_cast<int>(index);
        }
      }

      return column;
    };

    const int element_column = find_column("elem");
    const int integration_column = find_column("integ.pnt.");

    if (element_column < 0 || integration_column < 0)
    {
      PRINT_ERROR("Missing element/integration-point columns for job %d.\n", job_id);
      return false;
    }

    std::array<int, 6> columns{};

    for (std::size_t component = 0; component < columns.size(); ++component)
    {
      columns[component] = find_column(component_names[component]);

      if (columns[component] < 0)
      {
        PRINT_ERROR("Missing or duplicate tensor component %s for job %d.\n", component_names[component].c_str(), job_id);
        return false;
      }
    }

    for (const auto& record : dat.result_block_c1_data[data_id])
    {
      if (record.size() < 3)
      {
        PRINT_ERROR("Invalid DAT tensor record for job %d.\n", job_id);
        return false;
      }

      if (required_elements.count(record[0]) == 0)
      {
        continue;
      }

      if (record[2] != 2 || record[1] < 0 || static_cast<std::size_t>(record[1]) >= dat.result_block_data[data_id].size())
      {
        PRINT_ERROR("Invalid integration-point record for element %d.\n", record[0]);
        return false;
      }

      const auto& row = dat.result_block_data[data_id][record[1]];

      if (static_cast<std::size_t>(element_column) >= row.size() || static_cast<std::size_t>(integration_column) >= row.size())
      {
        PRINT_ERROR("Missing element/integration-point values for job %d.\n", job_id);
        return false;
      }

      if (!std::isfinite(row[element_column]) || row[element_column] != static_cast<double>(record[0]))
      {
        PRINT_ERROR("Inconsistent DAT element mapping for job %d.\n", job_id);
        return false;
      }

      const double integration_value = row[integration_column];

      if (!std::isfinite(integration_value) || integration_value < 1.0 || integration_value > static_cast<double>(std::numeric_limits<int>::max()) || std::trunc(integration_value) != integration_value)
      {
        PRINT_ERROR("Invalid integration-point ID for element %d.\n", record[0]);
        return false;
      }

      const int integration_id = static_cast<int>(integration_value);
      std::array<double, 6> tensor{};

      for (std::size_t component = 0; component < tensor.size(); ++component)
      {
        if (static_cast<std::size_t>(columns[component]) >= row.size() || !std::isfinite(row[columns[component]]))
        {
          PRINT_ERROR("Invalid %s tensor for element %d, integration point %d.\n", key.c_str(), record[0], integration_id);
          return false;
        }

        tensor[component] = row[columns[component]];
      }

      auto& element_tensors = collected[record[0]];
      const auto existing = element_tensors.find(integration_id);

      if (existing != element_tensors.end())
      {
        if (existing->second != tensor)
        {
          PRINT_ERROR("Conflicting %s results for element %d, integration point %d.\n", key.c_str(), record[0], integration_id);
          return false;
        }

        continue;
      }

      element_tensors.emplace(integration_id, tensor);
    }
  }

  for (const auto& element : required_elements)
  {
    const auto found = collected.find(element.first);

    if (found == collected.end() || found->second.empty())
    {
      PRINT_ERROR("Missing final-increment %s results for topology element %d in job %d.\n", key.c_str(), element.first, job_id);
      return false;
    }
  }

  tensors.swap(collected);

  return true;
}

bool CalculiXCore::read_topology_element_energies(int topology_id,int iteration_id,std::map<int, double>& element_energies)
{
  element_energies.clear();

  if (!mat || !results || !jobs)
  {
    PRINT_ERROR("Invalid topology energy query.\n");
    return false;
  }

  const int job_id = get_topology_iteration_job_id(topology_id, iteration_id);

  if (job_id <= 0 || jobs->get_job_status(job_id) != 2)
  {
    PRINT_ERROR("Topology iteration has no successfully completed job.\n");
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Topology material assignments are missing.\n");
    return false;
  }

  const int dat_index = results->get_dat_data_id_from_job_id(job_id);

  if (dat_index < 0)
  {
    PRINT_ERROR("DAT results for job %d are missing.\n", job_id);
    return false;
  }

  auto& dat = results->dat_data[dat_index];

  // The parser updates these when reading STEP/INCREMENT headers.
  const int last_step = dat.current_step;
  const int last_increment = dat.current_increment;

  if (last_step <= 0 || last_increment <= 0)
  {
    PRINT_ERROR("Final solver step/increment is missing in job %d.\n", job_id);
    return false;
  }

  int energy_type = -1;

  for (size_t i = 0; i < dat.result_block_type.size(); ++i)
  {
    if (dat.erase_whitespace(dat.result_block_type[i]) == "internalenergy")
    {
      energy_type = static_cast<int>(i);
      break;
    }
  }

  if (energy_type < 0)
  {
    PRINT_ERROR("Job %d has no element internal energy results.\n", job_id);
    return false;
  }

  const int energy_component = dat.get_result_block_component_id(energy_type, "energy");

  if (energy_component < 0)
  {
    PRINT_ERROR("Internal energy component is missing.\n");
    return false;
  }

  const auto& analysis_elements = collection->second.element_base_material;

  std::map<int, double> collected;

  // Read each result block once.
  for (const auto& block : dat.result_blocks)
  {
    if (block.size() < 7 || block[1] < 0 || static_cast<size_t>(block[1]) >= dat.total_times.size())
    {
      PRINT_ERROR("Invalid DAT result block.\n");
      return false;
    }

    if (block[2] != energy_type || block[5] != last_step || block[6] != last_increment)
    {
      continue;
    }

    const int data_id = block[4];

    if (data_id < 0 || static_cast<size_t>(data_id) >= dat.result_block_data.size() || static_cast<size_t>(data_id) >= dat.result_block_c1_data.size())
    {
      PRINT_ERROR("Invalid DAT energy data reference.\n");
      return false;
    }

    for (const auto& record : dat.result_block_c1_data[data_id])
    {
      if (record.size() < 3)
      {
        PRINT_ERROR("Invalid DAT element record.\n");
        return false;
      }

      // Whole-element results only.
      if (record[2] != 3 || analysis_elements.count(record[0]) == 0)
      {
        continue;
      }

      const int row_id = record[1];

      if (row_id < 0 || static_cast<size_t>(row_id) >= dat.result_block_data[data_id].size())
      {
        PRINT_ERROR("Invalid DAT energy row reference.\n");
        return false;
      }

      const auto& values = dat.result_block_data[data_id][row_id];

      if (static_cast<size_t>(energy_component) >= values.size())
      {
        PRINT_ERROR("Missing energy value for element %d.\n", record[0]);
        return false;
      }

      const double energy = values[energy_component];

      if (!std::isfinite(energy))
      {
        PRINT_ERROR("Non-finite energy for element %d.\n", record[0]);
        return false;
      }

      if (!collected.emplace(record[0], energy).second)
      {
        PRINT_ERROR("Multiple energy values for element %d in final step %d, increment %d.\n", record[0], last_step, last_increment);
        return false;
      }
    }
  }

  for (const auto& element : analysis_elements)
  {
    if (collected.count(element.first) == 0)
    {
      PRINT_ERROR("Missing energy for topology element %d in final step %d, increment %d.\n", element.first, last_step, last_increment);
      return false;
    }
  }

  element_energies.swap(collected);
  return true;
}

bool CalculiXCore::validate_topology_compliance_model(bool require_zero_displacements)
{
  if (!steps || !sections || !me_iface || !bcsdisplacements || !contactpairs || !initialconditions || !hbcs)
  {
    PRINT_ERROR("Compliance validation interfaces are not initialized.\n");
    return false;
  }

  if (steps->steps_data.empty())
  {
    PRINT_ERROR("Compliance optimization requires an analysis step.\n");
    return false;
  }

  if (!contactpairs->contactpairs_data.empty() || !initialconditions->initialconditions_data.empty())
  {
    PRINT_ERROR("The initial compliance evaluator does not support contact pairs or initial conditions.\n");
    return false;
  }

  const std::set<std::string> standard_volume_types = {
    "C3D4",
    "C3D6",
    "C3D8",
    "C3D10",
    "C3D15",
    "C3D20"
  };

  // The energy reader currently covers volume solid sections.
  for (const auto& section : sections->sections_data)
  {
    if (section.size() < 3 || section[1] != 1)
    {
      PRINT_ERROR("The initial compliance evaluator requires volume solid sections.\n");
      return false;
    }
    const int solid_index =
    sections->get_solid_section_data_id_from_solid_section_id(section[2]);

    if (solid_index < 0 || sections->solid_section_data[solid_index].size() < 2)
    {
      PRINT_ERROR("Missing solid section data for section %d.\n", section[0]);
      return false;
    }

    int block_id = -1;
    std::istringstream block_stream(sections->solid_section_data[solid_index][1]);

    if (!(block_stream >> block_id) || !(block_stream >> std::ws).eof() || block_id <= 0)
    {
      PRINT_ERROR("Invalid block for section %d.\n", section[0]);
      return false;
    }

    const std::string element_type = get_ccx_element_type(block_id);

    if (standard_volume_types.count(element_type) == 0)
    {
      PRINT_ERROR("Topology optimization does not support element type '%s' in block %d.\n", element_type.c_str(), block_id);
      return false;
    }
  }

  std::set<int> displacement_ids;

  for (const auto& step : steps->steps_data)
  {
    if (step.size() < 9 || step[3] != 2)
    {
      PRINT_ERROR("The initial compliance evaluator requires static steps.\n");
      return false;
    }

    const int parameter_index = steps->get_parameter_data_id_from_parameter_id(step[2]);

    if (parameter_index < 0 || steps->parameter_data[parameter_index].size() < 3)
    {
      PRINT_ERROR("Missing parameters for step %d.\n", step[0]);
      return false;
    }

    const std::string& nlgeom = steps->parameter_data[parameter_index][2];

    if (!nlgeom.empty() && nlgeom != "NO")
    {
      PRINT_ERROR("Compliance optimization requires NLGEOM=NO in step %d.\n", step[0]);
      return false;
    }

    for (int index : steps->get_load_data_ids_from_loads_id(step[5]))
    {
      const auto& load = steps->loads_data[index];

      // 1 = force, 2 = pressure, 9 = surface traction.
      if (load.size() < 3 || (load[1] != 1 && load[1] != 2 && load[1] != 9))
      {
        PRINT_ERROR("Step %d has a load unsupported by the initial compliance evaluator.\n", step[0]);
        return false;
      }
    }

    for (int index : steps->get_bc_data_ids_from_bcs_id(step[6]))
    {
      const auto& bc = steps->bcs_data[index];

      if (bc.size() < 3 || bc[1] != 1)
      {
        PRINT_ERROR("The initial compliance evaluator supports displacement BCs only, in step %d.\n", step[0]);
        return false;
      }
      displacement_ids.insert(bc[2]);
    }
  }

  // These model-level BCs are also exported.
  for (const auto& bc : hbcs->bcs_data)
  {
    if (bc.size() < 3 || bc[1] != 1)
    {
      PRINT_ERROR("The initial compliance evaluator supports displacement model-level BCs only.\n");
      return false;
    }

    displacement_ids.insert(bc[2]);
  }

  for (int bc_id : displacement_ids)
  {
    const int index = bcsdisplacements->get_bcs_data_id_from_bc_id(bc_id);

    if (index < 0 || bcsdisplacements->bcs_data[index].size() < 10)
    {
      PRINT_ERROR("Missing displacement BC %d.\n", bc_id);
      return false;
    }

    BCEntityHandle handle;
    std::vector<MeshExportBCData> attributes;

    if (!me_iface->get_bc_handle(BCTYPE_DISPLACEMENT, bc_id, handle) || !me_iface->get_bc_attributes(handle, attributes))
    {
      PRINT_ERROR("Unable to query displacement BC %d.\n", bc_id);
      return false;
    }

    for (const auto& attribute : attributes)
    {
      if (attribute.first < 0 || attribute.first >= 6 || !std::isfinite(attribute.second))
      {
        PRINT_ERROR("Invalid displacement BC %d attributes.\n", bc_id);
        return false;
      }

      // Match the exporter's fixed-DOF override.
      if (require_zero_displacements && !bcsdisplacements->check_dof_fixed(bc_id, attribute.first + 1) && attribute.second != 0.0)
      {
        PRINT_ERROR("The initial compliance evaluator requires zero prescribed displacement in BC %d.\n", bc_id);
        return false;
      }
    }
  }

  return true;
}

bool CalculiXCore::set_topology_iteration_densities(int topology_id, int iteration_id, const std::map<int, double>& densities, int source_iteration)
{
  if (!topology || !mat)
  {
    PRINT_ERROR("Topology/material cores are not initialized.\n");
    return false;
  }

  if (source_iteration < -1 || source_iteration == 0 || source_iteration >= iteration_id)
  {
    PRINT_ERROR("Invalid source iteration for topology density storage.\n");
    return false;
  }

  // get sensivity
  TopologyDensityDesign design;
  std::map<int, double> factors;
  if (!topology->build_density_design(topology_id, iteration_id, densities, design, factors))
  {
    return false;
  }

  if (source_iteration > 0)
  {
    if (!topology->set_density_update_diagnostics(topology_id, source_iteration, design))
    {
      return false;
    }
  }

  // get volumes
  std::map<int, double> element_volumes;
  if (!collect_topology_element_volumes(topology_id, element_volumes))
  {
    return false;
  }

  std::vector<int> element_ids;
  std::vector<double> stiffness_factors;

  for (const auto& factor : factors)
  {
    element_ids.push_back(factor.first);
    stiffness_factors.push_back(factor.second);
  }

  if (!set_topology_iteration_factors(topology_id, iteration_id, element_ids, stiffness_factors))
  {
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);
  auto& configuration = entry->second;

  configuration.iteration_density_designs.emplace(iteration_id, design);

  // These entries were created by this method, before any job exists.
  const auto discard_iteration = [&]()
  {
    configuration.iteration_density_designs.erase(iteration_id);
    const auto collection = mat->topology_material_data.find(topology_id);
    if (collection != mat->topology_material_data.end())
    {
      collection->second.iteration_factors.erase(iteration_id);
      collection->second.iteration_material_caps.erase(iteration_id);
      collection->second.iteration_material_groups.erase(iteration_id);
    }
  };

  if (!topology->evaluate_density_volume(topology_id, iteration_id, element_volumes))
  {
    discard_iteration();
    return false;
  }

  const auto stored = configuration.iteration_density_designs.find(iteration_id);

  const double minimum = stored->second.volume.minimum_design_volume_fraction;

  if (!std::isfinite(configuration.target_volume_fraction) ||
      configuration.target_volume_fraction <= 0.0 ||
      configuration.target_volume_fraction > 1.0)
  {
    PRINT_ERROR("Invalid topology target volume fraction.\n");
    discard_iteration();
    return false;
  }

  // Allow a small tolerance for accumulated floating-point rounding.
  if (configuration.target_volume_fraction + 1.0e-12 < minimum)
  {
    PRINT_ERROR(
        "Topology %d target volume fraction %.9g is below "
        "the minimum achievable fraction %.9g.\n",
        topology_id, configuration.target_volume_fraction, minimum);

    discard_iteration();
    return false;
  }

  return true;
}

bool CalculiXCore::collect_topology_element_volumes(int topology_id,std::map<int, double>& element_volumes,std::map<int, std::array<double, 3>>* element_centers)
{
  element_volumes.clear();
  if (element_centers)
  {
    element_centers->clear();
  }

  if (!mat)
  {
    PRINT_ERROR("Material core is not initialized.\n");
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end() ||
      collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Topology %d has no prepared elements.\n", topology_id);
    return false;
  }

  std::map<int, double> volumes;
  std::map<int, std::array<double, 3>> centers;

  PRINT_INFO("Topology %d: querying prepared elements directly.\n",topology_id);

  for (const auto& assignment : collection->second.element_base_material)
  {
    const int global_id = assignment.first;

    const std::string type = CubitInterface::get_element_type(global_id);

    std::string entity;
    if (type == "hex" || type == "tet" || type == "wedge")
    {
      entity = type;
    }
    else
    {
      entity = get_cubit_element_type_entity(type);
    }
    const int local_id = CubitInterface::get_element_type_id(global_id);

    if ((entity != "hex" && entity != "tet" && entity != "wedge") ||
        local_id <= 0)
    {
      PRINT_ERROR("Invalid topology element %d: type '%s', local ID %d.\n",global_id, type.c_str(), local_id);
      return false;
    }

    const bool first = volumes.empty();

    /*
    if (first)
    {
      PRINT_INFO("Querying volume: global %d, %s %d.\n", global_id, entity.c_str(), local_id);
    }
    */

    const double volume = CubitInterface::get_meshed_volume_or_area(entity, {local_id});

    if (!std::isfinite(volume) || volume <= 0.0)
    {
      PRINT_ERROR("Invalid volume for topology element %d.\n", global_id);
      return false;
    }

    volumes.emplace(global_id, volume);

    if (element_centers)
    {
      /*
      if (first)
      {
        PRINT_INFO("Querying center: global %d, %s %d.\n", global_id, entity.c_str(), local_id);
      }
      */

      const auto center = CubitInterface::get_center_point(entity, local_id);

      for (double coordinate : center)
      {
        if (!std::isfinite(coordinate))
        {
          PRINT_ERROR("Invalid center for topology element %d.\n", global_id);
          return false;
        }
      }

      centers.emplace(global_id, center);

      if (first)
      {
        PRINT_INFO("First element center queried successfully.\n");
      }
    }
  }

  element_volumes.swap(volumes);

  if (element_centers)
  {
    element_centers->swap(centers);
  }

  PRINT_INFO("Topology %d: element geometry queries completed.\n", topology_id);
  return true;
}

bool CalculiXCore::collect_topology_integration_weights(int topology_id, std::map<int, std::map<int, double>>& weights)
{
  weights.clear();

  if (!topology || !mat || !cb)
  {
    PRINT_ERROR("Topology integration weight interfaces are not initialized.\n");
    return false;
  }

  if (topology->topology_cases.find(topology_id) == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto collection = mat->topology_material_data.find(topology_id);

  if (collection == mat->topology_material_data.end() || collection->second.element_base_material.empty())
  {
    PRINT_ERROR("Topology %d has no prepared elements.\n", topology_id);
    return false;
  }

  std::map<int, std::map<int, double>> collected;

  for (const auto& assignment : collection->second.element_base_material)
  {
    const int element_id = assignment.first;

    if (!CubitInterface::get_element_exists(element_id))
    {
      PRINT_ERROR("Topology element %d no longer exists.\n", element_id);
      return false;
    }

    std::string entity = CubitInterface::get_element_type(element_id);

    if (entity != "hex" && entity != "tet" && entity != "wedge")
    {
      entity = get_cubit_element_type_entity(entity);
    }

    const int local_id = CubitInterface::get_element_type_id(element_id);

    if ((entity != "hex" && entity != "tet" && entity != "wedge") || local_id <= 0)
    {
      PRINT_ERROR("Invalid topology volume element %d.\n", element_id);
      return false;
    }

    const int block_id = CubitInterface::get_element_block(element_id);

    if (block_id <= 0 || cb->get_blocks_data_id_from_block_id(block_id) < 0)
    {
      PRINT_ERROR("Missing CalculiX block assignment for topology element %d.\n", element_id);
      return false;
    }

    const std::string calculix_type = get_ccx_element_type(block_id);
    const auto connectivity = CubitInterface::get_expanded_connectivity(entity, local_id);

    int cubit_type = -1;

    if (entity == "hex")
    {
      if (connectivity.size() == 8)
      {
        cubit_type = 42; // HEX8
      }
      else if (connectivity.size() == 20)
      {
        cubit_type = 44; // HEX20
      }
    }
    else if (entity == "tet")
    {
      if (connectivity.size() == 4)
      {
        cubit_type = 31; // TETRA4
      }
      else if (connectivity.size() == 10)
      {
        cubit_type = 33; // TETRA10
      }
    }
    else if (entity == "wedge")
    {
      if (connectivity.size() == 6)
      {
        cubit_type = 50; // WEDGE6
      }
      else if (connectivity.size() == 15)
      {
        cubit_type = 52; // WEDGE15
      }
    }

    if (cubit_type < 0)
    {
      PRINT_ERROR("Unsupported connectivity for topology element %d: %s with %d nodes.\n", element_id, entity.c_str(), static_cast<int>(connectivity.size()));
      return false;
    }

    std::vector<ElementIntegration::Coordinates> coordinates;
    coordinates.reserve(connectivity.size());

    std::set<int> unique_nodes;

    for (int node_id : connectivity)
    {
      if (node_id <= 0 || !CubitInterface::get_node_exists(node_id))
      {
        PRINT_ERROR("Invalid node %d in topology element %d.\n", node_id, element_id);
        return false;
      }

      if (!unique_nodes.insert(node_id).second)
      {
        PRINT_ERROR("Duplicate node %d in topology element %d.\n", node_id, element_id);
        return false;
      }

      coordinates.push_back(CubitInterface::get_nodal_coordinates(node_id));
    }

    std::map<int, double> element_weights;

    if (!ElementIntegration::calculate_weights(calculix_type, cubit_type, coordinates, element_weights))
    {
      PRINT_ERROR("Unable to calculate integration weights for topology %d element %d in block %d.\n", topology_id, element_id, block_id);
      return false;
    }

    collected.emplace(element_id, element_weights);
  }

  weights.swap(collected);
  return true;
}

bool CalculiXCore::prepare_topology_density_iteration(int topology_id, int source_iteration, int next_iteration)
{
  return topologyoptimization && topologyoptimization->prepare_topology_density_iteration(topology_id, source_iteration, next_iteration);
}

bool CalculiXCore::filter_topology_density_sensitivities(int topology_id, int iteration_id, const std::map<int, double>& element_volumes, const std::map<int, std::array<double, 3>>& element_centers)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->filter_density_sensitivities(topology_id, iteration_id, element_volumes, element_centers);
}

bool CalculiXCore::build_topology_density_update(int topology_id, int source_iteration, std::map<int, double>& densities)
{
  densities.clear();

  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->build_density_update(topology_id, source_iteration, densities);
}

bool CalculiXCore::evaluate_topology_compliance_oc_residual(int topology_id, int iteration_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->evaluate_compliance_oc_residual(topology_id, iteration_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_topology_iteration_data(int topology_id)
{
  if (!topology || !jobs)
  {
    PRINT_ERROR("Topology/job cores are not initialized.\n");
    return {};
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return {};
  }

  const auto& configuration = entry->second;
  std::vector<std::vector<std::string>> rows;
  
  const auto format_value = [](double value)
  {
    std::ostringstream stream;
    stream << std::scientific << std::setprecision(15) << value;
    return stream.str();
  };

  for (const auto& iteration : configuration.iteration_jobs)
  {
    std::string objective;
    std::string fraction;
    std::string density_change;
    std::string objective_change;
    std::string converged;
    std::string displacement;
    std::string displacement_node;
    std::string displacement_nodeset;
    std::string displacement_component;

    const auto evaluation = configuration.iteration_evaluations.find(iteration.first);

    if (evaluation != configuration.iteration_evaluations.end())
    {
      objective = format_value(evaluation->second.value);

      if (evaluation->second.displacement_available)
      {
        displacement = format_value(evaluation->second.displacement_value);
        displacement_node = std::to_string(evaluation->second.displacement_node);
        displacement_nodeset = std::to_string(evaluation->second.displacement_nodeset);
        displacement_component = std::to_string(evaluation->second.displacement_component);
      }

      if (evaluation->second.convergence_checked)
      {
        density_change = format_value(evaluation->second.maximum_density_change);

        objective_change = format_value(evaluation->second.relative_objective_change);

        converged = evaluation->second.converged ? "1" : "0";
      }
    }

    const auto design = configuration.iteration_density_designs.find(iteration.first);

    if (design != configuration.iteration_density_designs.end() && design->second.volume.available)
    {
      fraction = format_value(design->second.volume.volume_fraction);
    }

    rows.push_back({
        std::to_string(iteration.first),
        std::to_string(iteration.second),
        std::to_string(jobs->get_job_status(iteration.second)),
        objective,
        fraction,
        configuration.selected_iteration == iteration.first ? "1" : "0",
        density_change,
        objective_change,
        converged,
        displacement,
        displacement_node,
        displacement_nodeset,
        displacement_component
    });
  }

  return rows;
}

bool CalculiXCore::select_topology_iteration(int topology_id, int iteration_id)
{
  if (!topology || !mat || !jobs || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology iteration selection request.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);
  const auto collection = mat->topology_material_data.find(topology_id);

  if (entry == topology->topology_cases.end() || collection == mat->topology_material_data.end())
  {
    PRINT_ERROR("Topology %d has no prepared iteration data.\n",
                topology_id);
    return false;
  }

  auto& configuration = entry->second;

  for (const auto& iteration : configuration.iteration_jobs)
  {
    const int status = jobs->get_job_status(iteration.second);

    if (status == 1 || status == 5)
    {
      PRINT_ERROR("Topology %d still has an active job.\n", topology_id);
      return false;
    }
  }

  const int job_id = get_topology_iteration_job_id(topology_id, iteration_id);

  if (job_id <= 0 || jobs->get_job_status(job_id) != 2 ||
      collection->second.iteration_factors.count(iteration_id) == 0 ||
      configuration.iteration_evaluations.count(iteration_id) == 0)
  {
    PRINT_ERROR("Topology %d iteration %d is not a completed, evaluated result.\n", topology_id, iteration_id);
    return false;
  }

  configuration.selected_iteration = iteration_id;

  PRINT_INFO("Topology %d selected iteration %d, job %d.\n", topology_id, iteration_id, job_id);

  return true;
}

std::string CalculiXCore::get_topology_run_state(int topology_id)
{
  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return "";
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return "";
  }

  switch (entry->second.run_state)
  {
    case TopologyRunState::NotRun:
      return "not_run";
    case TopologyRunState::Running:
      return "running";
    case TopologyRunState::Converged:
      return "converged";
    case TopologyRunState::IterationLimit:
      return "iteration_limit";
    case TopologyRunState::Failed:
      return "failed";
    case TopologyRunState::Killed:
      return "killed";
  }

  PRINT_ERROR("Invalid topology run state.\n");
  return "";
}

std::vector<std::vector<double>> CalculiXCore::get_topology_iteration_element_data(int topology_id,int iteration_id)
{
  if (!topology || !mat || !jobs || iteration_id < -1)
  {
    PRINT_ERROR("Invalid topology element-data request.\n");
    return {};
  }

  const auto entry = topology->topology_cases.find(topology_id);
  const auto collection = mat->topology_material_data.find(topology_id);

  if (entry == topology->topology_cases.end() ||
      collection == mat->topology_material_data.end())
  {
    PRINT_ERROR("Topology %d has no prepared design data.\n", topology_id);
    return {};
  }

  const auto& configuration = entry->second;

  if (iteration_id == -1)
  {
    iteration_id = configuration.selected_iteration;
  }

  if (iteration_id < 1)
  {
    PRINT_ERROR("Topology %d has no selected iteration.\n", topology_id);
    return {};
  }

  const int job_id = get_topology_iteration_job_id(topology_id, iteration_id);

  if (job_id <= 0 ||
      jobs->get_job_status(job_id) != 2 ||
      configuration.iteration_evaluations.count(iteration_id) == 0)
  {
    PRINT_ERROR("Topology iteration %d is not a completed evaluation.\n", iteration_id);
    return {};
  }

  const auto design = configuration.iteration_density_designs.find(iteration_id);
  const auto factors = collection->second.iteration_factors.find(iteration_id);

  if (design == configuration.iteration_density_designs.end() ||
      factors == collection->second.iteration_factors.end())
  {
    PRINT_ERROR("Missing topology iteration densities or factors.\n");
    return {};
  }

  const auto& densities = design->second.element_densities;
  const auto& preserved = configuration.element_preserved;
  const auto& assignments = collection->second.element_base_material;

  if (densities.empty() ||
      densities.size() != factors->second.size() ||
      densities.size() != preserved.size() ||
      densities.size() != assignments.size())
  {
    PRINT_ERROR("Incomplete topology iteration element data.\n");
    return {};
  }

  std::vector<std::vector<double>> rows;
  rows.reserve(densities.size());

  for (const auto& density : densities)
  {
    const auto factor = factors->second.find(density.first);
    const auto flag = preserved.find(density.first);

    if (density.first <= 0 ||
        assignments.count(density.first) == 0 ||
        factor == factors->second.end() ||
        flag == preserved.end() ||
        !std::isfinite(density.second) ||
        density.second <= 0.0 || density.second > 1.0 ||
        !std::isfinite(factor->second) ||
        factor->second <= 0.0 || factor->second > 1.0)
    {
      PRINT_ERROR("Invalid topology data for element %d.\n",
                  density.first);
      return {};
    }

    rows.push_back({
        static_cast<double>(density.first),
        density.second,
        factor->second,
        flag->second ? 1.0 : 0.0
    });
  }

  return rows;
}

bool CalculiXCore::build_topology_adjoint_load(int topology_id, int iteration_id, double& objective_value, int& node_id, std::array<double, 3>& load)
{
  objective_value = 0.0;
  node_id = -1;
  load.fill(0.0);

  if (!topologyoptimization)
  {
    PRINT_ERROR("Topology optimization core is not initialized.\n");
    return false;
  }

  return topologyoptimization->build_topology_adjoint_load(topology_id, iteration_id, objective_value, node_id, load);
}

bool CalculiXCore::check_topology_displacement_selection(int topology_id, int iteration_id)
{
  if (!topology || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology displacement selection request.\n");
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  const auto& configuration = entry->second;

  if (configuration.displacement_nodeset <= 0 || configuration.displacement_component < 0 || configuration.displacement_component > 3)
  {
    PRINT_ERROR("A displacement nodeset and valid component are required.\n");
    return false;
  }

  const auto evaluation = configuration.iteration_evaluations.find(iteration_id);

  if (evaluation != configuration.iteration_evaluations.end() && evaluation->second.displacement_available)
  {
    if (evaluation->second.displacement_nodeset != configuration.displacement_nodeset || evaluation->second.displacement_component != configuration.displacement_component)
    {
      PRINT_ERROR("The displacement selection differs from the stored iteration measurement.\n");
      return false;
    }
  }

  return true;
}


bool CalculiXCore::is_topology_adjoint_export() const
{
  return job_export_context.adjoint;
}

bool CalculiXCore::check_topology_export(int topology_id, int iteration_id)
{
  if (job_export_context.job_id > 0)
  {
    if (job_export_context.topology_id != topology_id || job_export_context.iteration_id != iteration_id)
    {
      PRINT_ERROR("Export selection differs from the active job context.\n");
      return false;
    }
  }

  if (!is_topology_adjoint_export())
  {
    return true;
  }

  if (topology_id <= 0 || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology adjoint export context.\n");
    return false;
  }

  if (!customlines)
  {
    PRINT_ERROR("Custom input interface is not initialized.\n");
    return false;
  }

  if (!customlines->customlines_data.empty())
  {
    PRINT_ERROR("Adjoint export does not yet support custom input lines.\n");
    return false;
  }

  return validate_topology_compliance_model(false);
}

bool CalculiXCore::set_job_conversion(int job_id, int conversion)
{
  return jobs->set_job_conversion(job_id,conversion);
}

bool CalculiXCore::result_cgx_job(int job_id)
{
  return jobs->result_cgx_job(job_id);
}

bool CalculiXCore::result_paraview_job(int job_id)
{
  return jobs->result_paraview_job(job_id);
}

bool CalculiXCore::result_plot_job_frd(int job_id,int x_node_id, std::string x_block_type, std::string x_block_component, bool x_increment,bool x_time,int y_node_id, std::string y_block_type, std::string y_block_component, bool y_increment, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath)
{ 
  bool plot_possible = false;
  std::vector<int> increments;
  std::vector<double> times;
  QString windowtitle = "FRD Plot";
  std::vector<double> x_data;
  std::vector<double> y_data;

  //std::string log;
  //log = "plotting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  increments = frd_get_total_increments(job_id);
  for (size_t i = 0; i < increments.size(); i++)
  {
    times.push_back(frd_get_time_from_total_increment(job_id,increments[i]));
  }

  //x data
  if (x_increment)
  {
    for (size_t i = 0; i < increments.size(); i++)
    {
      x_data.push_back(increments[i]);
    }
  }
  if (x_time)
  {
    for (size_t i = 0; i < times.size(); i++)
    {
      x_data.push_back(times[i]);
    }
  }
  if (x_node_id!=-1)
  {
    for (size_t i = 0; i < increments.size(); i++)
    {
      x_data.push_back(frd_get_node_value(job_id,x_node_id, increments[i], x_block_type,x_block_component));
    }
  }

  //y data
  if (y_increment)
  {
    for (size_t i = 0; i < increments.size(); i++)
    {
      y_data.push_back(increments[i]);
    }
  }
  if (y_time)
  {
    for (size_t i = 0; i < times.size(); i++)
    {
      y_data.push_back(times[i]);
    }
  }
  if (y_node_id!=-1)
  {
    for (size_t i = 0; i < increments.size(); i++)
    {
      y_data.push_back(frd_get_node_value(job_id,y_node_id, increments[i], y_block_type,y_block_component));
    }
  }

  if (x_axis=="")
  {
    std::string tmp;
    if (x_increment)
    {
      tmp = "Increment";
    }
    if (x_time)
    {
      tmp = "Time";
    }
    if (x_node_id!=-1)
    {
      tmp = "Node ID " + std::to_string(x_node_id) + ", " + x_block_type + "[" + x_block_component + "]";
    }
    x_axis = QString::fromStdString(tmp);
  }
  if (y_axis=="")
  {
    std::string tmp;
    if (y_increment)
    {
      tmp = "Increment";
    }
    if (y_time)
    {
      tmp = "Time";
    }
    if (y_node_id!=-1)
    {
      tmp = "Node ID " + std::to_string(y_node_id) + ", " + y_block_type + "[" + y_block_component + "]";
    }
    y_axis = QString::fromStdString(tmp);
  }
  

  if ((x_data.size()>0)&&(y_data.size()>0)&&(x_data.size()==y_data.size()))
  {
    plot_possible = true;
  }
    
  if (plot_possible)
  {
    plotchart = new PlotChart(nullptr,windowtitle, title, x_axis, y_axis, x_data, y_data,save,save_filepath);
    plotchart->show();
    if (save)
    {
      plotchart->close();
    }
  }  

  return plot_possible;
}

bool CalculiXCore::result_plot_job_dat(int job_id,int x_node_id,int x_element_id,int x_element_ip, std::string x_block_set,std::string x_block_type, std::string x_block_component, bool x_time,int y_node_id,int y_element_id,int y_element_ip, std::string y_block_set,std::string y_block_type, std::string y_block_component, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath)
{
  bool plot_possible = false;
  std::vector<double> x_times;
  std::vector<double> y_times;
  QString windowtitle = "Dat Plot";
  std::vector<double> x_data;
  std::vector<double> y_data;

  //std::string log;
  //log = "plotting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  if (x_time&&y_time) //can't show time over time
  {
    return false;
  }
  //get times
  if (x_time)
  {
    x_times = this->dat_get_result_block_times(job_id,y_block_type,y_block_set);
    y_times = this->dat_get_result_block_times(job_id,y_block_type,y_block_set);
    x_data = x_times;
  }
  if (y_time)
  {
    x_times = this->dat_get_result_block_times(job_id,x_block_type,x_block_set);
    y_times = this->dat_get_result_block_times(job_id,x_block_type,x_block_set);
    y_data = y_times;
  }
  if ((!x_time)&&(!y_time))
  {
    x_times = this->dat_get_result_block_times(job_id,x_block_type,x_block_set);
    y_times = this->dat_get_result_block_times(job_id,y_block_type,y_block_set);
  }

  //x data
  if (x_node_id!=-1)
  {
    for (size_t i = 0; i < x_times.size(); i++)
    {
      x_data.push_back(this->dat_get_node_value(job_id, x_node_id,x_times[i],x_block_type,x_block_set, x_block_component));
    }
  }
  if (x_element_id!=-1)
  {
    for (size_t i = 0; i < x_times.size(); i++)
    {
      std::vector<double> element_result = this->dat_get_element_values_for_component(job_id, x_element_id,x_times[i],x_block_type,x_block_set, x_block_component);
      for (size_t ii = 0; ii < element_result.size(); ii++)
      {
        if (ii+1==x_element_ip)
        {
          x_data.push_back(element_result[ii]);
        }
      }
    }
  }

  //y data
  if (y_node_id!=-1)
  {
    for (size_t i = 0; i < y_times.size(); i++)
    {
      y_data.push_back(this->dat_get_node_value(job_id, y_node_id,y_times[i],y_block_type,y_block_set, y_block_component));
    }
  }
  if (y_element_id!=-1)
  {
    for (size_t i = 0; i < y_times.size(); i++)
    {
      std::vector<double> element_result = this->dat_get_element_values_for_component(job_id, y_element_id,y_times[i],y_block_type,y_block_set, y_block_component);
      for (size_t ii = 0; ii < element_result.size(); ii++)
      {
        if (ii+1==y_element_ip)
        {
          y_data.push_back(element_result[ii]);
        }
      }
    }
  }

  if (x_axis=="")
  {
    std::string tmp;
    if (x_time)
    {
      tmp = "Time";
    }
    if (x_node_id!=-1)
    {
      tmp = "Node ID " + std::to_string(x_node_id) + ", " + x_block_type + "[" + x_block_component + "]";
    }
    if (x_element_id!=-1)
    {
      tmp = "Element ID " + std::to_string(x_element_id) + ", Int. Point "+ std::to_string(x_element_ip) + ", " + x_block_type + "[" + x_block_component + "]";
    }
    x_axis = QString::fromStdString(tmp);
  }
  if (y_axis=="")
  {
    std::string tmp;
    if (y_time)
    {
      tmp = "Time";
    }
    if (y_node_id!=-1)
    {
      tmp = "Node ID " + std::to_string(y_node_id) + ", " + y_block_type + "[" + y_block_component + "]";
    }
    if (y_element_id!=-1)
    {
      tmp = "Element ID " + std::to_string(y_element_id) + ", Int. Point "+ std::to_string(y_element_ip) + ", " + y_block_type + "[" + y_block_component + "]";
    }
    y_axis = QString::fromStdString(tmp);
  }
  
  /*
  std::string log;
  log = "x_times.size() "+ std::to_string(x_times.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  log = "y_times.size() "+ std::to_string(y_times.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  log = "x_data.size() "+ std::to_string(x_data.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  log = "y_data.size() "+ std::to_string(y_data.size()) + "\n";
  PRINT_INFO("%s", log.c_str());
  */

  if ((x_data.size()>0)&&(y_data.size()>0)&&(x_data.size()==y_data.size()))
  {
    plot_possible = true;
  }
    
  if (plot_possible)
  {
    plotchart = new PlotChart(nullptr,windowtitle, title, x_axis, y_axis, x_data, y_data,save,save_filepath);
    plotchart->show();
    if (save)
    {
      plotchart->close();
    }
  }

  return plot_possible;
}

bool CalculiXCore::result_plot_job_dat_section(int job_id, std::string x_section_set, std::string x_section_component, bool x_time, std::string y_section_set, std::string y_section_component, bool y_time,QString title,QString x_axis,QString y_axis,bool save, QString save_filepath)
{
  bool plot_possible = false;
  std::vector<double> x_times;
  std::vector<double> y_times;
  QString windowtitle = "Dat Section Plot";
  std::vector<double> x_data;
  std::vector<double> y_data;

  //std::string log;
  //log = "plotting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  if (x_time&&y_time) //can't show time over time
  {
    return false;
  }

  std::vector<std::vector<double>> section_data;
  std::vector<std::string> components;
  components = this->dat_get_section_label(job_id);
  int component_id = -1;

  if (x_time)
  {
    // get results
    section_data = this->dat_get_section_data(job_id,y_section_set);

    //prepare times
    for (size_t i = 0; i < section_data.size(); i++)
    {
      x_times.push_back(section_data[i][0]);
    }

    x_data = x_times;

    for (size_t i = 0; i < components.size(); i++)
    {
      if (components[i]==y_section_component)
      {
        component_id = int(i);
        break;
      }
    }
  }

  if (y_time)
  {
    // get results
    section_data = this->dat_get_section_data(job_id,x_section_set);

    //prepare times
    for (size_t i = 0; i < section_data.size(); i++)
    {
      y_times.push_back(section_data[i][0]);
    }

    y_data = y_times;

    for (size_t i = 0; i < components.size(); i++)
    {
      if (components[i]==x_section_component)
      {
        component_id = int(i);
        break;
      }
    }
  }

  if (component_id==-1) //requested component not found
  {
    return false;
  }

  //x data
  if (!x_time)
  {
    for (size_t i = 0; i < section_data.size(); i++)
    {
      x_data.push_back(section_data[i][component_id]);
    }
  }
  
  //y data
  if (!y_time)
  {
    for (size_t i = 0; i < section_data.size(); i++)
    {
      y_data.push_back(section_data[i][component_id]);
    }
  }

  if (x_axis=="")
  {
    std::string tmp;
    if (x_time)
    {
      tmp = "Time";
    }else{
      tmp = "Section Set " + x_section_set + " [" + x_section_component + "]";
    }
    x_axis = QString::fromStdString(tmp);
  }
  if (y_axis=="")
  {
    std::string tmp;
    if (y_time)
    {
      tmp = "Time";
    }else{
      tmp = "Section Set " + y_section_set + " [" + y_section_component + "]";
    }
    y_axis = QString::fromStdString(tmp);
  }

  if ((x_data.size()>0)&&(y_data.size()>0)&&(x_data.size()==y_data.size()))
  {
    plot_possible = true;
  }
    
  if (plot_possible)
  {
    plotchart = new PlotChart(nullptr,windowtitle, title, x_axis, y_axis, x_data, y_data,save,save_filepath);
    plotchart->show();
    if (save)
    {
      plotchart->close();
    }
  }

  return plot_possible;
}

bool CalculiXCore::result_csv_job_frd(int job_id,std::string block_type, std::string block_component, std::string increment,int node_id,int block_id,int nodeset_id,int sideset_id, bool overwrite, std::string save_filepath)
{ 
  std::string log;
  //log = "exporting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  if(job_id == -1)
  {
    log = "Can't export results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (block_type=="")
  {
    log = "Can't export results -> no result block set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if(block_component=="")
  {
    log = "Can't export results -> no result component set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  if (increment=="")
  {
    log = "Can't export results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  std::vector<int> nodes;
  std::vector<int> frd_nodes = this->frd_get_nodes(job_id);

  // check if filter was chosen
  if (block_id!=-1)
  {
    std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in block " + std::to_string(block_id));; 
    for (size_t i = 0; i < node_ids.size(); i++)
    {
      if (this->frd_check_node_exists(job_id, node_ids[i]))
      {
        nodes.push_back(node_ids[i]);
      }
    }
  }
  if (nodeset_id!=-1)
  {
    std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in nodeset " + std::to_string(nodeset_id));; 
    for (size_t i = 0; i < node_ids.size(); i++)
    {
      if (this->frd_check_node_exists(job_id, node_ids[i]))
      {
        nodes.push_back(node_ids[i]);
      }
    }
  }
  if (sideset_id!=-1)
  {
    std::vector<int> node_ids = CubitInterface::parse_cubit_list("node","all in sideset " + std::to_string(sideset_id));; 
    for (size_t i = 0; i < node_ids.size(); i++)
    {
      if (this->frd_check_node_exists(job_id, node_ids[i]))
      {
        nodes.push_back(node_ids[i]);
      }
    }
  }
  
  if (node_id > 0)
  {
    if (this->frd_check_node_exists(job_id, node_id))
    {
        nodes.push_back(node_id);
    }else
    {
      log = "Can't find node id " + std::to_string(node_id) + " in frd data -> reference points for example are not written into frd \n";
      PRINT_INFO("%s", log.c_str());
    }  
  }

  if (nodes.size()==0) // this means no filter for sets was applied
  {
    nodes = frd_nodes;
  }
  
  //prepare components
  std::vector<std::string> components;
  if (block_component=="all")
  {
    components = this->frd_get_result_block_components(job_id, block_type);
  }else{
    components.push_back(block_component);
  }

  //prepare increments
  std::vector<int> increments;
  if (increment=="all")
  {
    increments = this->frd_get_total_increments(job_id);
  }else{
    increments.push_back(std::stoi(increment));
  }

  //get header
  std::vector<std::string> header;
  header.push_back("Node ID");
  header.push_back("Increment");
  header.push_back("Time");
  for (size_t ii = 0; ii < components.size(); ii++)
  {
    header.push_back(components[ii]);
  }
    
  //get results
  std::vector<std::vector<double>> results;
  if ((nodes.size()>0)&&(components.size()>0)&&(increments.size()>0)) //check if data can be queried
  {
    for (size_t i = 0; i < nodes.size(); i++)
    {
      for (size_t ii = 0; ii < increments.size(); ii++)
      {
        std::vector<double> tmp_result;
        double increment_time = this->frd_get_time_from_total_increment(job_id, increments[ii]);
        tmp_result.push_back(double(nodes[i]));
        tmp_result.push_back(double(increments[ii]));
        tmp_result.push_back(increment_time);
        for (size_t iii = 0; iii < components.size(); iii++)
        {
          double node_result = this->frd_get_node_value(job_id, nodes[i] , increments[ii], block_type, components[iii]);
          tmp_result.push_back(node_result);
        }
        results.push_back(tmp_result);
      }
    }
  }
  //write csv
  this->export_to_csv(save_filepath, header, results,overwrite);
  
 return true;
}

bool CalculiXCore::result_csv_job_dat(int job_id,std::string block_type,std::string block_set, std::string block_component, std::string time,int node_id,int element_id,bool overwrite, std::string save_filepath)
{ 
  std::string log;
  //log = "exporting job "+ std::to_string(job_id) + "\n";
  //PRINT_INFO("%s", log.c_str());

  if(job_id == -1)
  {
    log = "Can't export results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (block_set=="")
  {
    log = "Can't export results -> no result_set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (block_type=="")
  {
    log = "Can't export results -> no result block set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if(block_component=="")
  {
    log = "Can't export results -> no result component set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (time=="")
  {
    log = "Can't export results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if ((node_id>0)&&(element_id>0))
  {
    log = "Can't export results -> Can't filter for Node ID and Element ID at the same time! \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  //prepare components
  std::vector<std::string> components;
  if (block_component=="all")
  {
    components = this->dat_get_result_block_components(job_id, block_type);
  }else{
    components.push_back(block_component);
  }

  //prepare times
  std::vector<double> times;
  if (time=="all")
  {
    times = this->dat_get_result_block_times(job_id,block_type,block_set);
  }else{
    times.push_back(std::stod(time));
  }

  //get nodes and elements
  std::vector<int> nodes;
  std::vector<int> elements;

  if (node_id>0)
  {
    nodes.push_back(node_id);
  }else{
    nodes = this->dat_get_result_block_nodes(job_id,times[0],block_type,block_set);
  }
  if (element_id>0)
  {
    elements.push_back(element_id);
  }else{
    elements = this->dat_get_result_block_elements(job_id,times[0],block_type,block_set);
  }
  
  if ((nodes.size()>0)&&(elements.size()>0))
  {
    log = "Can't export results -> Can't request for Nodes and Element Data at the same time! \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  //get results
  std::vector<std::vector<double>> node_results;
  std::vector<std::vector<double>> element_results;

  if ((nodes.size()>0)&&(components.size()>0)&&(times.size()>0)) //check if data can be queried
  {
    int ic = 0;
    for (size_t i = 0; i < nodes.size(); i++)
    {
      for (size_t ii = 0; ii < times.size(); ii++)
      {
        std::vector<double> tmp_result;
        tmp_result.push_back(double(nodes[i]));
        tmp_result.push_back(double(times[ii]));
        for (size_t iii = 0; iii < components.size(); iii++)
        {
          double node_result = this->dat_get_node_value(job_id, nodes[i],times[ii],block_type,block_set, components[iii]);
          tmp_result.push_back(node_result);
        }
        node_results.push_back(tmp_result);
      }
    }
  }

  if ((elements.size()>0)&&(components.size()>0)&&(times.size()>0)) //check if data can be queried
  {
    int ic = 0;
    for (size_t i = 0; i < elements.size(); i++)
    {
      for (size_t ii = 0; ii < times.size(); ii++)
      { 
        std::vector<std::vector<double>> tmp_result;
        for (size_t iii = 0; iii < components.size(); iii++)
        {
          std::vector<double> element_result = this->dat_get_element_values_for_component(job_id, elements[i],times[ii],block_type,block_set, components[iii]);
          if (iii == 0)
          {
            for (size_t iv = 0; iv < element_result.size(); iv++)
            {
              std::vector<double> tmp;
              tmp.push_back(double(elements[i]));
              tmp.push_back(double(times[ii]));
              tmp.push_back(element_result[iv]);
              tmp_result.push_back(tmp);
            }
          }else{
            for (size_t iv = 0; iv < element_result.size(); iv++)
            {
              tmp_result[iv].push_back(element_result[iv]);
            }
          }
        }
        for (size_t iii = 0; iii < tmp_result.size(); iii++)
        {
          element_results.push_back(tmp_result[iii]);
        }  
      }
    }
  }
  //get header
  std::vector<std::string> header;
  if (node_results.size()>0)
  {
    header.push_back("Node ID");
    header.push_back("Time");
    for (size_t ii = 0; ii < components.size(); ii++)
    {
      header.push_back(components[ii]);
    }
    //write csv
    this->export_to_csv(save_filepath, header, node_results,overwrite);
  }
  if (element_results.size()>0)
  {
    header.push_back("Element ID");
    header.push_back("Time");
    for (size_t ii = 0; ii < components.size(); ii++)
    {
      header.push_back(components[ii]);
    }
    //write csv
    this->export_to_csv(save_filepath, header, element_results,overwrite);
  }
  
 return true;
}

bool CalculiXCore::result_csv_job_dat_section(int job_id, std::string result_set, std::string time, bool overwrite, std::string save_filepath)
{ 
  std::string log;

  if(job_id == -1)
  {
    log = "Can't export results -> no job set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (result_set=="")
  {
    log = "Can't export results -> no result_set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (time=="")
  {
    log = "Can't export results -> no increment set \n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  
  // get results
  std::vector<std::vector<double>> section_data = this->dat_get_section_data(job_id,result_set);

  //prepare components
  std::vector<std::string> components;
  components = this->dat_get_section_label(job_id);

  //prepare times
  std::vector<double> times;
  if (time=="all")
  {
    for (size_t i = 0; i < section_data.size(); i++)
    {
      times.push_back(section_data[i][0]);
    }
  }else{
    times.push_back(std::stod(time));
  }

  //get results
  std::vector<std::vector<double>> section_results;
  
  if ((section_data.size()>0)&&(components.size()>0)&&(times.size()>0)) //check if data can be queried
  {
    bool bool_time = false;
    for (size_t i = 0; i < section_data.size(); i++)
    {
      for (size_t ii = 0; ii < times.size(); ii++)
      {
        if (section_data[i][0]==times[ii])
        {
          bool_time = true;
          break;
        }
      }

      if (bool_time)// check if items are in range
      {
        section_results.push_back(section_data[i]);
      }
      bool_time = false;
    }
  }

  //get header
  std::vector<std::string> header;
  if (section_results.size()>0)
  {
    for (size_t ii = 0; ii < components.size(); ii++)
    {
      header.push_back(components[ii]);
    }
    //write csv
    this->export_to_csv(save_filepath, header, section_results,overwrite);
  }
  
 return true;
}

std::vector<std::string> CalculiXCore::get_job_data(int job_id)
{
  return jobs->get_job_data(job_id);
}

std::vector<std::string> CalculiXCore::get_job_console_output(int job_id)
{
  return jobs->get_job_console_output(job_id);
}

std::vector<std::string> CalculiXCore::get_job_cvg(int job_id)
{
  return jobs->get_job_cvg(job_id);
}

std::vector<std::string> CalculiXCore::get_job_sta(int job_id)
{
  return jobs->get_job_sta(job_id);
}

int CalculiXCore::get_job_status(int job_id)
{
  return jobs->get_job_status(job_id);
}

bool CalculiXCore::create_result(int job_id)
{
  return results->create_result(job_id);
}

bool CalculiXCore::delete_result(int job_id)
{
  return results->delete_result(job_id);
}

bool CalculiXCore::load_result(int job_id)
{
  return results->load_result(job_id);
}

int CalculiXCore::convert_result(int job_id, std::vector<int> options, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids)
{
  return results->convert_result(job_id, options, block_ids, nodeset_ids, sideset_ids);
}

bool CalculiXCore::project_result(int job_id,int step,int totalincrement,double scale)
{
  return results->project_result(job_id,step,totalincrement,scale);
}

bool CalculiXCore::project_topology(int topology_id, int iteration_id, double threshold, bool preview)
{
  if (!results)
  {
    PRINT_ERROR("Results core is not initialized.\n");
    return false;
  }

  return results->project_topology(topology_id, iteration_id, threshold, preview);
}

double CalculiXCore::compute_von_mises_stress(std::vector<double> vec)
{
  double von_mises = 0;

  von_mises = std::sqrt(0.5*(
    (vec[0]-vec[1])*(vec[0]-vec[1])
    +(vec[1]-vec[2])*(vec[1]-vec[2])
    +(vec[2]-vec[0])*(vec[2]-vec[0]))
    +3*(vec[3]*vec[3]+vec[4]*vec[4]+vec[5]*vec[5]) 
    );

  return von_mises;
}

double CalculiXCore::compute_von_mises_strain(std::vector<double> vec)
{
  double von_mises = 0;

  von_mises = 2./3. * std::sqrt(0.5*(
    (vec[0]-vec[1])*(vec[0]-vec[1])
    +(vec[1]-vec[2])*(vec[1]-vec[2])
    +(vec[2]-vec[0])*(vec[2]-vec[0]))
    +3*(vec[3]*vec[3]+vec[4]*vec[4]+vec[5]*vec[5]) 
    );
  
  return von_mises;
}

std::vector<double> CalculiXCore::compute_principal_stresses(std::vector<double> vec)
{
  std::vector<double> ps;
  
  // vec[0] = SXX
  // vec[1] = SYY
  // vec[2] = SZZ
  // vec[3] = SXY
  // vec[4] = SYZ
  // vec[5] = SZX

  Eigen::Matrix3d m;
  m(0,0) = vec[0];
  m(1,0) = vec[3];
  m(2,0) = vec[5];
  m(0,1) = vec[3];
  m(1,1) = vec[1];
  m(2,1) = vec[4];
  m(0,2) = vec[5];
  m(1,2) = vec[4];
  m(2,2) = vec[2];
  
  // compute eigenvalues
  //Eigen::EigenSolver<Eigen::Matrix3d> es(m);
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(m);
  // get eigenvalues, only real parts should be here

  for (size_t i = 0; i < 3; i++)
  {
    std::complex<double> lambda = es.eigenvalues()[i];
    ps.push_back(lambda.real());
  }
  // sort values
  std::sort(std::begin(ps), std::end(ps), std::greater<double>{});
  //std::sort(std::begin(ps), std::end(ps));

  // get worst ps
  double worstPS = 0.0;
  for (size_t i = 0; i < 3; i++)
  {
    if (std::fabs(worstPS) < std::fabs(ps[i]))
    {
      worstPS = ps[i];
    }
  }
  ps.push_back(worstPS);

  //ps.push_back(0.0);
  //ps.push_back(0.0);
  //ps.push_back(0.0);
  //ps.push_back(0.0);

  //std::string log;
  //log = std::to_string(ps[0]) + " " + std::to_string(ps[1]) + " " + std::to_string(ps[2]) + " "+ std::to_string(ps[3]) + "\n";
  //PRINT_INFO("%s", log.c_str());  

  return ps;
}

std::vector<double> CalculiXCore::compute_principal_strains(std::vector<double> vec)
{
  std::vector<double> pe;
  
  // vec[0] = EXX
  // vec[1] = EYY
  // vec[2] = EZZ
  // vec[3] = EXY
  // vec[4] = EYZ
  // vec[5] = EZX

  Eigen::Matrix3d m;
  m(0,0) = vec[0];
  m(1,0) = vec[3];
  m(2,0) = vec[5];
  m(0,1) = vec[3];
  m(1,1) = vec[1];
  m(2,1) = vec[4];
  m(0,2) = vec[5];
  m(1,2) = vec[4];
  m(2,2) = vec[2];
  
  // compute eigenvalues
  //Eigen::EigenSolver<Eigen::Matrix3d> es(m);
  Eigen::SelfAdjointEigenSolver<Eigen::Matrix3d> es(m);
  // get eigenvalues, only real parts should be here
  for (size_t i = 0; i < 3; i++)
  {
    std::complex<double> lambda = es.eigenvalues()[i];
    pe.push_back(lambda.real());
  } 
  // sort values
  std::sort(std::begin(pe), std::end(pe), std::greater<double>{});
  //std::sort(std::begin(pe), std::end(pe));

  // get worst pe
  double worstPE = 0.0;
  for (size_t i = 0; i < 3; i++)
  {
    if (std::fabs(worstPE) < std::fabs(pe[i]))
    {
      worstPE = pe[i];
    }
  }
  pe.push_back(worstPE);

  //std::string log;
  //log = std::to_string(pe[0]) + " " + std::to_string(pe[1]) + " " + std::to_string(pe[2]) + " "+ std::to_string(pe[3]) + "\n";
  //PRINT_INFO("%s", log.c_str());

  //pe.push_back(0.0);
  //pe.push_back(0.0);
  //pe.push_back(0.0);
  //pe.push_back(0.0);

  return pe;
}

bool CalculiXCore::create_customline(std::vector<std::string> options)
{
  return customlines->create_customline(options);
}

bool CalculiXCore::modify_customline(int customline_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return customlines->modify_customline(customline_id, options, options_marker);
}

bool CalculiXCore::delete_customline(int customline_id)
{
  return customlines->delete_customline(customline_id);
}

std::vector<std::string> CalculiXCore::get_customline_data(std::string position,std::string keyword,int keyword_id)
{
  return customlines->get_customline_data(position,keyword,keyword_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_entities(std::string entity, int id)
{
  std::vector<std::vector<std::string>> entities;
  int data_id = -1;
  int sub_data_id = -1;
  std::vector<int> sub_data_ids;

  if (entity=="block")
  {
    data_id = cb->get_blocks_data_id_from_block_id(id);
    if (data_id!=-1)
    {
      entities.push_back({"block",std::to_string(cb->blocks_data[data_id][0])});
    }
  }else if (entity=="nodeset")
  {
    entities.push_back({"nodeset",std::to_string(id)});
  }else if (entity=="sideset")
  {
    entities.push_back({"sideset",std::to_string(id)});
  }else if (entity=="material")
  {
    for (size_t i = 0; i < sections->sections_data.size(); i++)
    {
      if (sections->sections_data[i][1] == 1)
      {
        sub_data_id = sections->get_solid_section_data_id_from_solid_section_id(sections->sections_data[i][2]);
        if (sections->solid_section_data[sub_data_id][2]==std::to_string(id))
        {
          entities.push_back({"block",sections->solid_section_data[sub_data_id][1]});
        }
      }else if (sections->sections_data[i][1] == 2)
      {
        sub_data_id = sections->get_shell_section_data_id_from_shell_section_id(sections->sections_data[i][2]);
        if (sections->solid_section_data[sub_data_id][2]==std::to_string(id))
        {
          entities.push_back({"block",sections->shell_section_data[sub_data_id][1]});
        }
      }else if (sections->sections_data[i][1] == 3)
      {
        sub_data_id = sections->get_beam_section_data_id_from_beam_section_id(sections->sections_data[i][2]);
        if (sections->solid_section_data[sub_data_id][2]==std::to_string(id))
        {
          entities.push_back({"block",sections->beam_section_data[sub_data_id][1]});
        }
      }else if (sections->sections_data[i][1] == 4)
      {
        sub_data_id = sections->get_membrane_section_data_id_from_membrane_section_id(sections->sections_data[i][2]);
        if (sections->solid_section_data[sub_data_id][2]==std::to_string(id))
        {
          entities.push_back({"block",sections->membrane_section_data[sub_data_id][1]});
        }
      }
    }
    
  }else if (entity=="section")
  {
    data_id = sections->get_sections_data_id_from_section_id(id);
    if (data_id!=-1)
    {
      if (sections->sections_data[data_id][1] == 1)
      {
        sub_data_id = sections->get_solid_section_data_id_from_solid_section_id(sections->sections_data[data_id][2]);
        entities.push_back({"block",sections->solid_section_data[sub_data_id][1]});
      }else if (sections->sections_data[data_id][1] == 2)
      {
        sub_data_id = sections->get_shell_section_data_id_from_shell_section_id(sections->sections_data[data_id][2]);
        entities.push_back({"block",sections->shell_section_data[sub_data_id][1]});
      }else if (sections->sections_data[data_id][1] == 3)
      {
        sub_data_id = sections->get_beam_section_data_id_from_beam_section_id(sections->sections_data[data_id][2]);
        entities.push_back({"block",sections->beam_section_data[sub_data_id][1]});
      }else if (sections->sections_data[data_id][1] == 4)
      {
        sub_data_id = sections->get_membrane_section_data_id_from_membrane_section_id(sections->sections_data[data_id][2]);
        entities.push_back({"block",sections->membrane_section_data[sub_data_id][1]});
      }
    }
  }else if (entity=="constraint")
  {
    data_id = constraints->get_constraints_data_id_from_constraint_id(id);
    if (data_id!=-1)
    {
      if (constraints->constraints_data[data_id][1] == 1)
      {
        sub_data_id = constraints->get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(constraints->constraints_data[data_id][2]);
        if (constraints->rigidbody_constraint_data[sub_data_id][1]=="1")
        {
          entities.push_back({"nodeset",constraints->rigidbody_constraint_data[sub_data_id][2]});
          entities.push_back({"vertex",constraints->rigidbody_constraint_data[sub_data_id][3]});
        }else if (constraints->rigidbody_constraint_data[sub_data_id][1]=="2")
        {
          entities.push_back({"block",constraints->rigidbody_constraint_data[sub_data_id][2]});
          entities.push_back({"vertex",constraints->rigidbody_constraint_data[sub_data_id][3]});
        }
      }else if (constraints->constraints_data[data_id][1] == 2)
      {
        sub_data_id = constraints->get_tie_constraint_data_id_from_tie_constraint_id(constraints->constraints_data[data_id][2]);
        entities.push_back({"sideset",constraints->tie_constraint_data[sub_data_id][2]});
        entities.push_back({"sideset",constraints->tie_constraint_data[sub_data_id][3]});
      }else if (constraints->constraints_data[data_id][1] == 3)
      {
        //sub_data_id = constraints->get_equation_constraint_data_id_from_equation_constraint_id(constraints->constraints_data[data_id][2]);
        sub_data_ids = constraints->get_equation_data_ids_from_equation_constraint_id(constraints->constraints_data[data_id][2]);
        for (size_t i = 0; i < sub_data_ids.size(); i++)
        {
          entities.push_back({"node",std::to_string(int(constraints->equation_data[sub_data_ids[i]][1]))});
        }
      }else if (constraints->constraints_data[data_id][1] == 4)
      {
        sub_data_ids = constraints->get_equation_group_data_ids_from_equation_group_constraint_id(constraints->constraints_data[data_id][2]);
        std::string node_ids = "";
        for (size_t i = 0; i < sub_data_ids.size(); i++)
        {
          node_ids.append(" ");
          node_ids.append(std::to_string(int(constraints->equation_group_data[sub_data_ids[i]][1])));
          node_ids.append(" ");
          node_ids.append(std::to_string(int(constraints->equation_group_data[sub_data_ids[i]][2])));
        }
        entities.push_back({"node",node_ids});
      }
    }
  }else if (entity=="surfaceinteraction")
  {
    
  }else if (entity=="contactpair")
  {
    data_id = contactpairs->get_contactpairs_data_id_from_contactpair_id(id);
    if (data_id!=-1)
    {
      entities.push_back({"sideset",std::to_string(contactpairs->contactpairs_data[data_id][3])});
      entities.push_back({"sideset",std::to_string(contactpairs->contactpairs_data[data_id][4])});
      
      sub_data_id = contactpairs->get_adjust_contactpair_data_id_from_adjust_contactpair_id(contactpairs->contactpairs_data[data_id][5]);
      if (contactpairs->adjust_contactpair_data[sub_data_id][2]!="")
      {
        entities.push_back({"nodeset",contactpairs->adjust_contactpair_data[sub_data_id][2]});
      }  
    }
  }else if (entity=="amplitude")
  {
    
  }else if (entity=="orientation")
  {
    
  }else if (entity=="loadsforce")
  {
    entities.push_back({"force",std::to_string(id)});
  }else if (entity=="loadspressure")
  {
    entities.push_back({"pressure",std::to_string(id)});
  }else if (entity=="loadsheatflux")
  {
    entities.push_back({"heatflux",std::to_string(id)});
  }else if (entity=="loadsgravity")
  {
    data_id = loadsgravity->get_loads_data_id_from_load_id(id);
    if (data_id!=-1)
    {
      entities.push_back({"block",std::to_string(loadsgravity->loads_data[data_id][4])});
    }
  }else if (entity=="loadscentrifugal")
  {
    data_id = loadscentrifugal->get_loads_data_id_from_load_id(id);
    if (data_id!=-1)
    {
      entities.push_back({"block",std::to_string(loadscentrifugal->loads_data[data_id][4])});
    }
  }else if (entity=="loadstrajectory")
  {
    data_id = loadstrajectory->get_loads_data_id_from_load_id(id);
    if (data_id!=-1)
    {
      if (loadstrajectory->get_load_type(id)=="HEATFLUX")
      {
        sub_data_id = loadstrajectory->heatflux->get_loads_data_id_from_load_id(loadstrajectory->loads_data[data_id][2]);
        entities.push_back({"curve",std::to_string(loadstrajectory->heatflux->loads_data[sub_data_id][2])});
        entities.push_back({"vertex",std::to_string(loadstrajectory->heatflux->loads_data[sub_data_id][3])});
        std::vector<int> surface_ids = loadstrajectory->heatflux->get_fire_ray_surface_ids_from_fire_ray_surface_id(loadstrajectory->heatflux->loads_data[sub_data_id][4]);
        for (size_t i = 0; i < surface_ids.size(); i++)
        {
          entities.push_back({"surface",std::to_string(surface_ids[i])});
        }
      }
      if (loadstrajectory->get_load_type(id)=="BODYHEATFLUXSPHERE")
      {
        sub_data_id = loadstrajectory->bodyheatfluxsphere->get_loads_data_id_from_load_id(loadstrajectory->loads_data[data_id][2]);
        entities.push_back({"curve",std::to_string(loadstrajectory->bodyheatfluxsphere->loads_data[sub_data_id][2])});
        entities.push_back({"vertex",std::to_string(loadstrajectory->bodyheatfluxsphere->loads_data[sub_data_id][3])});
        std::vector<int> surface_ids = loadstrajectory->bodyheatfluxsphere->get_fire_ray_surface_ids_from_fire_ray_surface_id(loadstrajectory->bodyheatfluxsphere->loads_data[sub_data_id][4]);
        for (size_t i = 0; i < surface_ids.size(); i++)
        {
          entities.push_back({"surface",std::to_string(surface_ids[i])});
        }
      }
    }
  }else if (entity=="loadsfilm")
  {
    data_id = loadsfilm->get_loads_data_id_from_load_id(id);
    if (data_id!=-1)
    {
      entities.push_back({"sideset",std::to_string(loadsfilm->loads_data[data_id][4])});
    }
  }else if (entity=="loadsradiation")
  {
    data_id = loadsradiation->get_loads_data_id_from_load_id(id);
    if (data_id!=-1)
    {
      entities.push_back({"sideset",std::to_string(loadsradiation->loads_data[data_id][4])});
    }
  }else if (entity=="loadssurfacetraction")
  {
    data_id = loadssurfacetraction->get_loads_data_id_from_load_id(id);
    if (data_id!=-1)
    {
      entities.push_back({"sideset",std::to_string(loadssurfacetraction->loads_data[data_id][4])});
    }
  }else if (entity=="loadsbodyheatflux")
  {
    data_id = loadsbodyheatflux->get_loads_data_id_from_load_id(id);
    if (data_id!=-1)
    {
      if (loadsbodyheatflux->loads_data[data_id][5]==1)
      {
        std::vector<int> elements = loadsbodyheatflux->get_elements_from_element_id(loadsbodyheatflux->loads_data[data_id][4]);
        for (size_t i = 1; i < elements.size(); i++)
        {
          entities.push_back({"block",std::to_string(elements[i])});
        }  
      }else if (loadsbodyheatflux->loads_data[data_id][5]==2)
      {
        std::vector<int> elements = loadsbodyheatflux->get_elements_from_element_id(loadsbodyheatflux->loads_data[data_id][4]);
        for (size_t i = 1; i < elements.size(); i++)
        {
          entities.push_back({CubitInterface::get_element_type(elements[i]),std::to_string(elements[i])});
        }
      }
    }
  }else if (entity=="bcsdisplacement")
  {
    entities.push_back({"displacement",std::to_string(id)});
  }else if (entity=="bcstemperature")
  {
    entities.push_back({"temperature",std::to_string(id)});
  }else if (entity=="initialcondition")
  {
    int data_id = initialconditions->get_initialconditions_data_id_from_initialcondition_id(id);
    int sub_data_id;
    
    if (initialconditions->initialconditions_data[data_id][1]==1)
    {
      sub_data_id = initialconditions->get_displacement_data_id_from_displacement_id(initialconditions->initialconditions_data[data_id][2]);
      if (initialconditions->displacement_data[sub_data_id][1] != "")
      {
        entities.push_back({"displacement",initialconditions->displacement_data[sub_data_id][1]});
      }
    }else if (initialconditions->initialconditions_data[data_id][1]==2)
    {
      sub_data_id = initialconditions->get_temperature_data_id_from_temperature_id(initialconditions->initialconditions_data[data_id][2]);
      if (initialconditions->temperature_data[sub_data_id][1] != "")
      {
        entities.push_back({"temperature",initialconditions->temperature_data[sub_data_id][1]});
      }
    }else if (initialconditions->initialconditions_data[data_id][1]==3)
    {
      sub_data_ids = initialconditions->get_stress_block_data_ids_from_stress_id(initialconditions->initialconditions_data[data_id][2]);
      for (size_t i = 0; i < sub_data_ids.size(); i++)
      {
        entities.push_back({"block",std::to_string(int(initialconditions->stress_block_data[sub_data_ids[i]][1]))});
      }
      sub_data_ids = initialconditions->get_stress_element_data_ids_from_stress_id(initialconditions->initialconditions_data[data_id][2]);
      for (size_t i = 0; i < sub_data_ids.size(); i++)
      {
        entities.push_back({CubitInterface::get_element_type(int(initialconditions->stress_element_data[sub_data_ids[i]][1])),std::to_string(int(initialconditions->stress_element_data[sub_data_ids[i]][1]))});
      }
    }
  }else if (entity=="hbcsdisplacement")
  {
    entities.push_back({"displacement",std::to_string(id)});
  }else if (entity=="hbcstemperature")
  {
    entities.push_back({"temperature",std::to_string(id)});
  }
  return entities;
}

bool CalculiXCore::create_materiallibrary_group(std::string name)
{
  return mat_library->create_group(name);
}

bool CalculiXCore::modify_materiallibrary_group(std::string name,  std::string data, int mode)
{
  return mat_library->modify_group(name,data,mode);
}

bool CalculiXCore::delete_materiallibrary_group(std::string name)
{
  return mat_library->delete_group(name);
}

bool CalculiXCore::create_materiallibrary_material(std::string name, std::string groupname)
{
  return mat_library->create_material(name, groupname);
}

bool CalculiXCore::modify_materiallibrary_material(std::string name, std::string groupname, std::string data, int mode, std::vector<double> value_data)
{
  return mat_library->modify_material(name, groupname, data, mode, value_data);
}

bool CalculiXCore::delete_materiallibrary_material(std::string name, std::string groupname)
{
  return mat_library->delete_material(name, groupname);
}

bool CalculiXCore::export_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name)
{
  return mat_library->export_material(name, groupname, cubit_name);
}

bool CalculiXCore::import_materiallibrary_material(std::string name, std::string groupname, std::string cubit_name)
{
  return mat_library->import_material(name, groupname, cubit_name);
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_force(int id)
{
  int bc_set_id=-1;
  BCSetHandle bc_set;
  NodesetHandle nodeset;
  std::string command;
  std::vector<BCEntityHandle> bc_handles;
  std::vector<std::vector<double>> draw_data;

  me_iface->create_default_bcset(0,true,true,true,bc_set);
  bc_set_id = me_iface->id_from_handle(bc_set);

  me_iface->get_bc_loads(bc_set, bc_handles);
  for (size_t i = 0; i < bc_handles.size(); i++)
  {  
    std::vector<MeshExportBCData> bc_attribs;
    me_iface->get_bc_attributes(bc_handles[i],bc_attribs); 
    if ((get_bc_fea_type(bc_attribs)==5))
    {
      if (id==me_iface->id_from_handle(bc_handles[i]))
      {     
        me_iface->get_bc_nodeset(bc_handles[i],nodeset);
        
        std::vector<std::vector<double>> coords = get_nodeset_entities_coords(me_iface->id_from_handle(nodeset));
        
        for (size_t ii = 0; ii < coords.size(); ii++)
        {
          std::vector<double> data;
          
          data.push_back(coords[ii][0]);
          data.push_back(coords[ii][1]);
          data.push_back(coords[ii][2]);
          data.push_back(bc_attribs[1].second*bc_attribs[0].second);
          data.push_back(bc_attribs[2].second*bc_attribs[0].second);
          data.push_back(bc_attribs[3].second*bc_attribs[0].second);
          draw_data.push_back(data);

          data[3] = bc_attribs[5].second*bc_attribs[4].second;
          data[4] = bc_attribs[6].second*bc_attribs[4].second;
          data[5] = bc_attribs[7].second*bc_attribs[4].second;
          draw_data.push_back(data);

          data.clear();
        }
      }
    }
  }

  //command.append("delete bcset " + std::to_string(bc_set_id));
  //CubitInterface::silent_cmd_without_running_journal_lines(command.c_str());

  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_pressure(int id)
{
  int bc_set_id=-1;
  BCSetHandle bc_set;
  SidesetHandle sideset;
  std::string command;
  std::vector<BCEntityHandle> bc_handles;
  std::vector<std::vector<double>> draw_data;

  me_iface->create_default_bcset(0,true,true,true,bc_set);
  bc_set_id = me_iface->id_from_handle(bc_set);

  me_iface->get_bc_loads(bc_set, bc_handles);
  for (size_t i = 0; i < bc_handles.size(); i++)
  {  
    std::vector<MeshExportBCData> bc_attribs;
    me_iface->get_bc_attributes(bc_handles[i],bc_attribs); 
    if ((get_bc_fea_type(bc_attribs)==6))
    {
      if (id==me_iface->id_from_handle(bc_handles[i]))
      {     
        me_iface->get_bc_sideset(bc_handles[i],sideset);
        std::vector<std::vector<double>> coords_normals = get_sideset_entities_coords_normals(me_iface->id_from_handle(sideset));
        
        for (size_t ii = 0; ii < coords_normals.size(); ii++)
        {
          std::vector<double> data;
          
          data.push_back(coords_normals[ii][0]);
          data.push_back(coords_normals[ii][1]);
          data.push_back(coords_normals[ii][2]);
          data.push_back(coords_normals[ii][3]);
          data.push_back(coords_normals[ii][4]);
          data.push_back(coords_normals[ii][5]);
          draw_data.push_back(data);
          data.clear();
        }
      }
    }
  }

  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_heatflux(int id)
{
  int bc_set_id=-1;
  BCSetHandle bc_set;
  SidesetHandle sideset;
  std::string command;
  std::vector<BCEntityHandle> bc_handles;
  std::vector<std::vector<double>> draw_data;

  me_iface->create_default_bcset(0,true,true,true,bc_set);
  bc_set_id = me_iface->id_from_handle(bc_set);

  me_iface->get_bc_loads(bc_set, bc_handles);
  for (size_t i = 0; i < bc_handles.size(); i++)
  {  
    std::vector<MeshExportBCData> bc_attribs;
    me_iface->get_bc_attributes(bc_handles[i],bc_attribs); 
    if ((get_bc_fea_type(bc_attribs)==7))
    {
      if (id==me_iface->id_from_handle(bc_handles[i]))
      {     
        me_iface->get_bc_sideset(bc_handles[i],sideset);
        std::vector<std::vector<double>> coords_normals = get_sideset_entities_coords_normals(me_iface->id_from_handle(sideset));
        
        for (size_t ii = 0; ii < coords_normals.size(); ii++)
        {
          std::vector<double> data;
          
          data.push_back(coords_normals[ii][0]);
          data.push_back(coords_normals[ii][1]);
          data.push_back(coords_normals[ii][2]);
          data.push_back(coords_normals[ii][3]);
          data.push_back(coords_normals[ii][4]);
          data.push_back(coords_normals[ii][5]);
          draw_data.push_back(data);
          data.clear();
        }
      }
    }
  }

  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_gravity(int id)
{
  std::vector<std::vector<double>> draw_data;

  for (size_t i = 0; i < loadsgravity->loads_data.size(); i++)
  {
    // check for right id
    if (id==loadsgravity->loads_data[i][0])
    { 
      int direction_data_id = loadsgravity->get_direction_data_id_from_direction_id(loadsgravity->loads_data[i][5]);

      std::vector<double> data;
      
      data.push_back(0);
      data.push_back(0);
      data.push_back(0);
      data.push_back(string_scientific_to_double(loadsgravity->direction_data[direction_data_id][1]));
      data.push_back(string_scientific_to_double(loadsgravity->direction_data[direction_data_id][2]));
      data.push_back(string_scientific_to_double(loadsgravity->direction_data[direction_data_id][3]));
      draw_data.push_back(data);
      data.clear();
    }

  }

  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_centrifugal(int id)
{
  std::vector<std::vector<double>> draw_data;

  for (size_t i = 0; i < loadscentrifugal->loads_data.size(); i++)
  {
    // check for right id
    if (id==loadscentrifugal->loads_data[i][0])
    { 
      int direction_data_id = loadscentrifugal->get_direction_data_id_from_direction_id(loadscentrifugal->loads_data[i][5]);
      int coordinate_data_id = loadscentrifugal->get_coordinate_data_id_from_coordinate_id(loadscentrifugal->loads_data[i][7]);

      std::vector<double> data;
      
      data.push_back(string_scientific_to_double(loadscentrifugal->coordinate_data[coordinate_data_id][1]));
      data.push_back(string_scientific_to_double(loadscentrifugal->coordinate_data[coordinate_data_id][2]));
      data.push_back(string_scientific_to_double(loadscentrifugal->coordinate_data[coordinate_data_id][3]));
      data.push_back(string_scientific_to_double(loadscentrifugal->direction_data[direction_data_id][1]));
      data.push_back(string_scientific_to_double(loadscentrifugal->direction_data[direction_data_id][2]));
      data.push_back(string_scientific_to_double(loadscentrifugal->direction_data[direction_data_id][3]));
      draw_data.push_back(data);
      data.clear();
    }

  }

  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_film(int id)
{
  std::vector<std::vector<double>> draw_data;
  
  for (size_t i = 0; i < loadsfilm->loads_data.size(); i++)
  {
    // check for right id
    if (id==loadsfilm->loads_data[i][0])
    { 
      std::vector<std::vector<double>> coords_normals = get_sideset_entities_coords_normals(loadsfilm->loads_data[i][4]);
      for (size_t ii = 0; ii < coords_normals.size(); ii++)
      {
        std::vector<double> data;
        
        data.push_back(coords_normals[ii][0]);
        data.push_back(coords_normals[ii][1]);
        data.push_back(coords_normals[ii][2]);
        data.push_back(coords_normals[ii][3]);
        data.push_back(coords_normals[ii][4]);
        data.push_back(coords_normals[ii][5]);
        draw_data.push_back(data);
        data.clear();
      }
    }
  }
  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_radiation(int id)
{
  std::vector<std::vector<double>> draw_data;
  
  for (size_t i = 0; i < loadsradiation->loads_data.size(); i++)
  {
    // check for right id
    if (id==loadsradiation->loads_data[i][0])
    { 
      std::vector<std::vector<double>> coords_normals = get_sideset_entities_coords_normals(loadsradiation->loads_data[i][4]);
      for (size_t ii = 0; ii < coords_normals.size(); ii++)
      {
        std::vector<double> data;
        
        data.push_back(coords_normals[ii][0]);
        data.push_back(coords_normals[ii][1]);
        data.push_back(coords_normals[ii][2]);
        data.push_back(coords_normals[ii][3]);
        data.push_back(coords_normals[ii][4]);
        data.push_back(coords_normals[ii][5]);
        draw_data.push_back(data);
        data.clear();
      }
    }
  }
  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_surface_traction(int id)
{
  std::vector<std::vector<double>> draw_data;
  
  for (size_t i = 0; i < loadssurfacetraction->loads_data.size(); i++)
  {
    // check for right id
    if (id==loadssurfacetraction->loads_data[i][0])
    { 
      std::vector<std::vector<double>> coords_normals = get_sideset_entities_coords_normals(loadssurfacetraction->loads_data[i][4]);
      int force_data_id = loadssurfacetraction->get_force_data_id_from_force_id(loadssurfacetraction->loads_data[i][5]);

      for (size_t ii = 0; ii < coords_normals.size(); ii++)
      {
        std::vector<double> data;
        
        data.push_back(coords_normals[ii][0]);
        data.push_back(coords_normals[ii][1]);
        data.push_back(coords_normals[ii][2]);
        data.push_back(loadssurfacetraction->force_data[force_data_id][1]);
        data.push_back(loadssurfacetraction->force_data[force_data_id][2]);
        data.push_back(loadssurfacetraction->force_data[force_data_id][3]);
        draw_data.push_back(data);
        data.clear();
      }
    }
  }
  return draw_data;
}

std::vector<std::vector<int>> CalculiXCore::get_draw_data_for_load_bodyheatflux(int id)
{
  std::vector<std::vector<int>> draw_data;
  for (size_t i = 0; i < loadsbodyheatflux->loads_data.size(); i++)
  {
    // check for right id
    if (id==loadsbodyheatflux->loads_data[i][0])
    { 
      std::vector<int> block_ids;
      std::vector<int> element_ids;

      if (loadsbodyheatflux->loads_data[i][5]==1)
      {
        std::vector<int> elements = loadsbodyheatflux->get_elements_from_element_id(loadsbodyheatflux->loads_data[i][4]);
        for (size_t ii = 1; ii < elements.size(); ii++)
        {
          block_ids.push_back(elements[ii]);
        }  
      }else if (loadsbodyheatflux->loads_data[i][5]==2)
      {
        std::vector<int> elements = loadsbodyheatflux->get_elements_from_element_id(loadsbodyheatflux->loads_data[i][4]);
        for (size_t ii = 1; ii < elements.size(); ii++)
        {
          element_ids.push_back(elements[ii]);
        }
      }
      draw_data.push_back(block_ids);
      draw_data.push_back(element_ids);
    }
  }
  
  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_bc_displacement(int id) // returns coord(3) and dof
{
  int bc_set_id=-1;
  BCSetHandle bc_set;
  NodesetHandle nodeset;
  std::string command;
  std::vector<BCEntityHandle> bc_handles;
  std::vector<std::vector<double>> draw_data;

  me_iface->create_default_bcset(0,true,true,true,bc_set);
  bc_set_id = me_iface->id_from_handle(bc_set);

  me_iface->get_bc_restraints(bc_set, bc_handles);
  for (size_t i = 0; i < bc_handles.size(); i++)
  {  
    std::vector<MeshExportBCData> bc_attribs;
    me_iface->get_bc_attributes(bc_handles[i],bc_attribs); 
    if ((get_bc_fea_type(bc_attribs)==1))
    {
      if (id==me_iface->id_from_handle(bc_handles[i]))
      {     
        me_iface->get_bc_nodeset(bc_handles[i],nodeset);
        std::vector<std::vector<double>> coords = get_nodeset_entities_coords(me_iface->id_from_handle(nodeset));
        
        for (size_t ii = 0; ii < coords.size(); ii++)
        {   
          for (size_t iii = 0; iii < bc_attribs.size(); iii++)
          {
            std::vector<double> data;            
            data.push_back(coords[ii][0]);
            data.push_back(coords[ii][1]);
            data.push_back(coords[ii][2]);
            data.push_back(bc_attribs[iii].first+1);
            draw_data.push_back(data);
            data.clear();          
          }
        }
      }
    }
  }

  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_bc_temperature(int id) // returns coord(3) and dof
{
  int bc_set_id=-1;
  BCSetHandle bc_set;
  NodesetHandle nodeset;
  std::string command;
  std::vector<BCEntityHandle> bc_handles;
  std::vector<std::vector<double>> draw_data;

  me_iface->create_default_bcset(0,true,true,true,bc_set);
  bc_set_id = me_iface->id_from_handle(bc_set);

  me_iface->get_bc_restraints(bc_set, bc_handles);
  for (size_t i = 0; i < bc_handles.size(); i++)
  {  
    std::vector<MeshExportBCData> bc_attribs;
    me_iface->get_bc_attributes(bc_handles[i],bc_attribs); 
    if ((get_bc_fea_type(bc_attribs)==4))
    {
      if (id==me_iface->id_from_handle(bc_handles[i]))
      {     
        me_iface->get_bc_nodeset(bc_handles[i],nodeset);
        std::vector<std::vector<double>> coords = get_nodeset_entities_coords(me_iface->id_from_handle(nodeset));
        
        for (size_t ii = 0; ii < coords.size(); ii++)
        {   
          for (size_t iii = 0; iii < bc_attribs.size(); iii++)
          {
            std::vector<double> data;            
            data.push_back(coords[ii][0]);
            data.push_back(coords[ii][1]);
            data.push_back(coords[ii][2]);
            data.push_back(11);
            draw_data.push_back(data);
            data.clear();          
          }
        }
      }
    }
  }
  
  return draw_data;
}  

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_orientation(int id) // returns pairs of 4 for {system_type,local_axis_angle}, coord(3) of section center, a_coord(3) ,b_coord(3)
{
  int orientation_data_id = orientations->get_orientations_data_id_from_orientation_id(id);
  std::vector<std::vector<double>> draw_data;
  int sub_data_id = -1;
  std::vector<double> o_data(3);
  std::vector<double> a_coord(3);
  std::vector<double> b_coord(3);
  std::vector<int> linked_blocks;

  // orientations_data[0][0] orientation_id
  // orientations_data[0][1] name_id              option 0
  // orientations_data[0][2] system_type          option 1: 1 for rectangular, 2 for cylindrical
  // orientations_data[0][3] distribution_id      option 2: standard -1 not implemented yet
  // orientations_data[0][4] a_id                 option 3
  // orientations_data[0][5] b_id                 option 4
  // orientations_data[0][6] local axis           option 5: -1 if not used, 1 = x, 2 = y , 3 = z
  // orientations_data[0][7] rotation_id          option 6

  if (orientation_data_id != -1)
  {
    //system_type,local_axis, data
    o_data[0] = orientations->orientations_data[orientation_data_id][2];
    o_data[1] = orientations->orientations_data[orientation_data_id][6];
    sub_data_id = orientations->get_rotation_data_id_from_rotation_id(orientations->orientations_data[orientation_data_id][4]);
    o_data[2] = this->string_scientific_to_double(orientations->rotation_data[sub_data_id][1]);

    // get a_coord
    sub_data_id = orientations->get_a_data_id_from_a_id(orientations->orientations_data[orientation_data_id][4]);
    a_coord[0] = this->string_scientific_to_double(orientations->a_data[sub_data_id][1]);
    a_coord[1] = this->string_scientific_to_double(orientations->a_data[sub_data_id][2]);
    a_coord[2] = this->string_scientific_to_double(orientations->a_data[sub_data_id][3]);

    // get b_coord
    sub_data_id = orientations->get_a_data_id_from_a_id(orientations->orientations_data[orientation_data_id][5]);
    b_coord[0] = this->string_scientific_to_double(orientations->b_data[sub_data_id][1]);
    b_coord[1] = this->string_scientific_to_double(orientations->b_data[sub_data_id][2]);
    b_coord[2] = this->string_scientific_to_double(orientations->b_data[sub_data_id][3]);

    // search if orientation is linked in a section
    for (size_t i = 0; i < sections->solid_section_data.size(); i++)
    {
      if (std::stoi(sections->solid_section_data[i][3])==id)
      {
        linked_blocks.push_back(std::stoi(sections->solid_section_data[i][1]));
      }
    }
    for (size_t i = 0; i < sections->shell_section_data.size(); i++)
    {
      if (std::stoi(sections->shell_section_data[i][3])==id)
      {
        linked_blocks.push_back(std::stoi(sections->shell_section_data[i][1]));
      }
    }
    for (size_t i = 0; i < sections->beam_section_data.size(); i++)
    {
      if (std::stoi(sections->beam_section_data[i][13])==id)
      {
        linked_blocks.push_back(std::stoi(sections->beam_section_data[i][1]));
      }
    }
    for (size_t i = 0; i < sections->membrane_section_data.size(); i++)
    {
      if (std::stoi(sections->membrane_section_data[i][3])==id)
      {
        linked_blocks.push_back(std::stoi(sections->membrane_section_data[i][1]));
      }
    }
    
    // loop over all blocks for center point
    for (size_t i = 0; i < linked_blocks.size(); i++)
    {
      std::vector<double> section_center;
      section_center = this->get_block_bounding_box_center(linked_blocks[i]);
      if (section_center.size()!=0)
      {
        draw_data.push_back(o_data);
        draw_data.push_back(section_center);
        draw_data.push_back(a_coord);
        draw_data.push_back(b_coord);
      }
    }
  }  

  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_equation(int id)
{
  std::vector<std::vector<double>> draw_data;
  int constraint_data_id;
  constraint_data_id = constraints->get_constraints_data_id_from_constraint_id(id);

  //check if constraint exists
  if (constraint_data_id == -1)
  {
    return draw_data;
  }
  
  // check if its a equation constraint
  if (constraints->constraints_data[constraint_data_id][1]!=3)
  {
    return draw_data;
  }  

  // get equation data ids
  std::vector<int> equation_data_ids = constraints->get_equation_data_ids_from_equation_constraint_id(constraints->constraints_data[constraint_data_id][2]);

  for (size_t i = 0; i < equation_data_ids.size(); i++)
  {
    if (CubitInterface::get_node_exists(int(constraints->equation_data[equation_data_ids[i]][1])))
    {
      std::array<double, 3> coords = CubitInterface::get_nodal_coordinates(int(constraints->equation_data[equation_data_ids[i]][1]));
        
      if (coords.size() > 0)
      {
        std::vector<double> data;            
        data.push_back(coords[0]);
        data.push_back(coords[1]);
        data.push_back(coords[2]);
        data.push_back(constraints->equation_data[equation_data_ids[i]][2]);
        draw_data.push_back(data);
        data.clear();
      }
    }
  }
  return draw_data;
}

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_equationgroup(int id)
{
  std::vector<std::vector<double>> draw_data;
  int constraint_data_id;
  constraint_data_id = constraints->get_constraints_data_id_from_constraint_id(id);

  //check if constraint exists
  if (constraint_data_id == -1)
  {
    return draw_data;
  }
  
  // check if its a equationgroup constraint
  if (constraints->constraints_data[constraint_data_id][1]!=4)
  {
    return draw_data;
  }  

  // get equation data ids
  std::vector<int> equationgroup_data_ids = constraints->get_equation_group_data_ids_from_equation_group_constraint_id(constraints->constraints_data[constraint_data_id][2]);

  for (size_t i = 0; i < equationgroup_data_ids.size(); i++)
  {
    if (CubitInterface::get_node_exists(int(constraints->equation_group_data[equationgroup_data_ids[i]][1]))&&CubitInterface::get_node_exists(int(constraints->equation_group_data[equationgroup_data_ids[i]][2])))
    {
      std::array<double, 3> coords_1 = CubitInterface::get_nodal_coordinates(int(constraints->equation_group_data[equationgroup_data_ids[i]][1]));
      std::array<double, 3> coords_2 = CubitInterface::get_nodal_coordinates(int(constraints->equation_group_data[equationgroup_data_ids[i]][2]));
        
      if ((coords_1.size() > 0) && (coords_2.size() > 0))
      {
        std::vector<double> data;            
        data.push_back(coords_1[0]);
        data.push_back(coords_1[1]);
        data.push_back(coords_1[2]);
        data.push_back(constraints->equation_group_data[equationgroup_data_ids[i]][3]);
        draw_data.push_back(data);
        data.clear();

        data.push_back(coords_2[0]);
        data.push_back(coords_2[1]);
        data.push_back(coords_2[2]);
        data.push_back(constraints->equation_group_data[equationgroup_data_ids[i]][3]);
        draw_data.push_back(data);
        data.clear();
      }
    }
  }
  return draw_data;
}

bool CalculiXCore::draw_all(double size) // draw all bc and loads
{
  return draw->draw_all(size);
}

bool CalculiXCore::draw_load_force(std::vector<int> force_ids,double size)
{
  for (size_t i = 0; i < force_ids.size(); i++)
  {
    draw->draw_load_force(force_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_pressure(std::vector<int> pressure_ids,double size)
{
  for (size_t i = 0; i < pressure_ids.size(); i++)
  {
    draw->draw_load_pressure(pressure_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_heatflux(std::vector<int> heatflux_ids,double size)
{
  for (size_t i = 0; i < heatflux_ids.size(); i++)
  {
    draw->draw_load_heatflux(heatflux_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_gravity(std::vector<int> gravity_ids,double size)
{
  for (size_t i = 0; i < gravity_ids.size(); i++)
  {
    draw->draw_load_gravity(gravity_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_centrifugal(std::vector<int> centrifugal_ids,double size)
{
  for (size_t i = 0; i < centrifugal_ids.size(); i++)
  {
    draw->draw_load_centrifugal(centrifugal_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_trajectory(std::vector<int> trajectory_ids,double size)
{
  for (size_t i = 0; i < trajectory_ids.size(); i++)
  {
    draw->draw_load_trajectory(trajectory_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_film(std::vector<int> film_ids,double size)
{
  for (size_t i = 0; i < film_ids.size(); i++)
  {
    draw->draw_load_film(film_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_radiation(std::vector<int> radiation_ids,double size)
{
  for (size_t i = 0; i < radiation_ids.size(); i++)
  {
    draw->draw_load_radiation(radiation_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_surface_traction(std::vector<int> surface_traction_ids,double size)
{
  for (size_t i = 0; i < surface_traction_ids.size(); i++)
  {
    draw->draw_load_surface_traction(surface_traction_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_load_bodyheatflux(std::vector<int> surface_traction_ids,double size)
{
  for (size_t i = 0; i < surface_traction_ids.size(); i++)
  {
    draw->draw_load_bodyheatflux(surface_traction_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_bc_displacement(std::vector<int> displacement_ids,double size)
{
  for (size_t i = 0; i < displacement_ids.size(); i++)
  {
    draw->draw_bc_displacement(displacement_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_bc_temperature(std::vector<int> temperature_ids,double size)
{
  for (size_t i = 0; i < temperature_ids.size(); i++)
  {
    draw->draw_bc_temperature(temperature_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_orientation(std::vector<int> orientation_ids,double size)
{
  for (size_t i = 0; i < orientation_ids.size(); i++)
  {
    draw->draw_orientation(orientation_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_equation(std::vector<int> equation_ids,double size)
{
  for (size_t i = 0; i < equation_ids.size(); i++)
  {
    draw->draw_equation(equation_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_equationgroup(std::vector<int> equationgroup_ids,double size)
{
  for (size_t i = 0; i < equationgroup_ids.size(); i++)
  {
    draw->draw_equationgroup(equationgroup_ids[i],size);
  }
  
  return true;
}

bool CalculiXCore::draw_loads(double size) // draw all loads
{
  return draw->draw_loads(size);
}

bool CalculiXCore::draw_bcs(double size) // draw all bc
{
  return draw->draw_bcs(size);
}

bool CalculiXCore::draw_orientations(double size) // draw all orientations
{
  return draw->draw_orientations(size);
}

bool CalculiXCore::draw_equations(double size) // draw all equations
{
  return draw->draw_equations(size);
}

bool CalculiXCore::draw_equationgroups(double size) // draw all equationgroups
{
  return draw->draw_equationgroups(size);
}

bool CalculiXCore::draw_load_forces(double size) // draw all forces
{
  return draw->draw_load_forces(size);
}

bool CalculiXCore::draw_load_pressures(double size)
{
  return draw->draw_load_pressures(size);
}

bool CalculiXCore::draw_load_heatfluxes(double size)
{
  return draw->draw_load_heatfluxes(size);
}

bool CalculiXCore::draw_load_gravities(double size)
{
  return draw->draw_load_gravities(size);
}

bool CalculiXCore::draw_load_centrifugals(double size)
{
  return draw->draw_load_centrifugals(size);
}

bool CalculiXCore::draw_load_trajectories(double size)
{
  return draw->draw_load_trajectories(size);
}

bool CalculiXCore::draw_load_films(double size)
{
  return draw->draw_load_films(size);
}

bool CalculiXCore::draw_load_radiations(double size)
{
  return draw->draw_load_radiations(size);
}

bool CalculiXCore::draw_load_surface_tractions(double size)
{
  return draw->draw_load_surface_tractions(size);
}

bool CalculiXCore::draw_load_bodyheatfluxes(double size)
{
  return draw->draw_load_bodyheatfluxes(size);
}

bool CalculiXCore::draw_bc_displacements(double size)
{
  return draw->draw_bc_displacements(size);
}

bool CalculiXCore::draw_bc_temperatures(double size)
{
  return draw->draw_bc_temperatures(size);
}

bool CalculiXCore::draw_topology(int topology_id,int iteration_id,double threshold)
{
  if (!draw)
  {
    PRINT_ERROR("Drawing core is not initialized.\n");
    return false;
  }
  return draw->draw_topology(topology_id, iteration_id, threshold);
}

bool CalculiXCore::animate_topology(int topology_id,double threshold,int fps,std::string save_filepath,bool overwrite)
{
  if (!draw)
  {
    PRINT_ERROR("Drawing core is not initialized.\n");
    return false;
  }

  return draw->animate_topology(topology_id, threshold, fps, save_filepath, overwrite);
}

std::vector<int> CalculiXCore::frd_get_nodes(int job_id)
{
  std::vector<int> tmp;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  tmp = results->frd_data[frd_data_id].sorted_node_ids;
      
  return tmp;
}

bool CalculiXCore::frd_check_node_exists(int job_id,int node_id)
{
  bool node_exists = false;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return node_exists;
  }

  if (std::binary_search(results->frd_data[frd_data_id].sorted_node_ids.begin(), results->frd_data[frd_data_id].sorted_node_ids.end(), node_id))
  { 
    node_exists = true;
    return node_exists;  
  }

  return node_exists;
}

std::vector<std::string> CalculiXCore::frd_get_result_block_types(int job_id)
{
  std::vector<std::string> tmp;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  tmp = results->frd_data[frd_data_id].result_block_type;
      
  return tmp;
}

std::vector<std::string> CalculiXCore::frd_get_result_block_components(int job_id, std::string result_block_type)
{
  std::vector<std::string> tmp;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  tmp = results->frd_data[frd_data_id].get_result_block_components_from_result_block_type(result_block_type);
      
  return tmp;
}

std::vector<int> CalculiXCore::frd_get_total_increments(int job_id)
{
  std::vector<int> tmp;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    if (i==0)
    {
      tmp.push_back(results->frd_data[frd_data_id].result_blocks[i][3]);
    }else{
      // don't use double entries
      if (tmp[tmp.size()-1]!=results->frd_data[frd_data_id].result_blocks[i][3])
      {
        tmp.push_back(results->frd_data[frd_data_id].result_blocks[i][3]);
      }
    }    
  }
  
  return tmp;
}

double CalculiXCore::frd_get_time_from_total_increment(int job_id, int total_increment)
{
  double tmp = -1;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    if (total_increment==results->frd_data[frd_data_id].result_blocks[i][3])
    {
      tmp = results->frd_data[frd_data_id].total_times[results->frd_data[frd_data_id].result_blocks[i][4]];
      return tmp;
    }
  }
  
  return tmp;
}

std::vector<int> CalculiXCore::frd_get_node_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value)
{
  std::vector<int> tmp; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results->frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results->frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all nodes in result block
          for (size_t ii = 0; ii < results->frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
          {
            int node_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double value = results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if ((value >= lower_value)&&(value <= upper_value))
            {
              tmp.push_back(node_id);
            }            
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::frd_get_node_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  std::vector<int> tmp; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results->frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results->frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all nodes in result block
          for (size_t ii = 0; ii < results->frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
          {
            int node_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double node_value = results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if (node_value <= value)
            {
              tmp.push_back(node_id);
            }            
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::frd_get_node_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  std::vector<int> tmp; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results->frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results->frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all nodes in result block
          for (size_t ii = 0; ii < results->frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
          {
            int node_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double node_value = results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if (node_value >= value)
            {
              tmp.push_back(node_id);
            }            
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::frd_get_element_ids_between_values(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_value,double upper_value)
{
  std::vector<int> tmp; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results->frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results->frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all elements
          for (size_t ii = 0; ii < results->frd_data[frd_data_id].elements.size(); ii++)
          {
            int element_id = results->frd_data[frd_data_id].elements[ii][0];
            int element_connectivity_data_id = results->frd_data[frd_data_id].elements[ii][2];
            std::vector<double> values;
            double min_value=0;
            double max_value=0;

            // loop over all nodes in elements connectiviy for the result block
            for (size_t iii = 0; iii < results->frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id].size(); iii++)
            {
              int node_id = results->frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id][iii];
              int node_data_id = -1;
              if (std::binary_search(results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id))
              {
                auto lower = std::lower_bound(results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id);
                node_data_id = results->frd_data[frd_data_id].sorted_result_node_data_ids[frd_data_id][lower - results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin()];
                values.push_back(results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id]);
              }
            }
            // get min and max in element
            for (size_t iii = 0; iii < values.size(); iii++)
            {
              if (iii==0)
              {
                min_value = values[iii];
                max_value = values[iii];
              }else{
                if (values[iii] < min_value)
                {
                  min_value = values[iii];
                }
                if (values[iii] > max_value)
                {
                  max_value = values[iii];
                }
              }
            }
            // check if values are in range
            if ((min_value >= lower_value)&&(max_value <= upper_value))
            {
              tmp.push_back(element_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::frd_get_element_ids_smaller_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  std::vector<int> tmp; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results->frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results->frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all elements
          for (size_t ii = 0; ii < results->frd_data[frd_data_id].elements.size(); ii++)
          {
            int element_id = results->frd_data[frd_data_id].elements[ii][0];
            int element_connectivity_data_id = results->frd_data[frd_data_id].elements[ii][2];
            std::vector<double> values;
            double min_value=0;
            double max_value=0;

            // loop over all nodes in elements connectiviy for the result block
            for (size_t iii = 0; iii < results->frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id].size(); iii++)
            {
              int node_id = results->frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id][iii];
              int node_data_id = -1;
              if (std::binary_search(results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id))
              {
                auto lower = std::lower_bound(results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id);
                node_data_id = results->frd_data[frd_data_id].sorted_result_node_data_ids[frd_data_id][lower - results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin()];
                values.push_back(results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id]);
              }
            }
            // get min and max in element
            for (size_t iii = 0; iii < values.size(); iii++)
            {
              if (iii==0)
              {
                min_value = values[iii];
                max_value = values[iii];
              }else{
                if (values[iii] < min_value)
                {
                  min_value = values[iii];
                }
                if (values[iii] > max_value)
                {
                  max_value = values[iii];
                }
              }
            }
            // check if values are in range
            if (max_value <= value)
            {
              tmp.push_back(element_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::frd_get_element_ids_greater_value(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double value)
{
  std::vector<int> tmp; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results->frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results->frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all elements
          for (size_t ii = 0; ii < results->frd_data[frd_data_id].elements.size(); ii++)
          {
            int element_id = results->frd_data[frd_data_id].elements[ii][0];
            int element_connectivity_data_id = results->frd_data[frd_data_id].elements[ii][2];
            std::vector<double> values;
            double min_value=0;
            double max_value=0;

            // loop over all nodes in elements connectiviy for the result block
            for (size_t iii = 0; iii < results->frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id].size(); iii++)
            {
              int node_id = results->frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id][iii];
              int node_data_id = -1;
              if (std::binary_search(results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id))
              {
                auto lower = std::lower_bound(results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id);
                node_data_id = results->frd_data[frd_data_id].sorted_result_node_data_ids[frd_data_id][lower - results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin()];
                values.push_back(results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id]);
              }
            }
            // get min and max in element
            for (size_t iii = 0; iii < values.size(); iii++)
            {
              if (iii==0)
              {
                min_value = values[iii];
                max_value = values[iii];
              }else{
                if (values[iii] < min_value)
                {
                  min_value = values[iii];
                }
                if (values[iii] > max_value)
                {
                  max_value = values[iii];
                }
              }
            }
            // check if values are in range
            if (min_value >= value)
            {
              tmp.push_back(element_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::frd_get_element_ids_over_limit(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double limit)
{
  std::vector<int> tmp; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results->frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results->frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all elements
          for (size_t ii = 0; ii < results->frd_data[frd_data_id].elements.size(); ii++)
          {
            int element_id = results->frd_data[frd_data_id].elements[ii][0];
            int element_connectivity_data_id = results->frd_data[frd_data_id].elements[ii][2];
            std::vector<double> values;
            double min_value=0;
            double max_value=0;

            // loop over all nodes in elements connectiviy for the result block
            for (size_t iii = 0; iii < results->frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id].size(); iii++)
            {
              int node_id = results->frd_data[frd_data_id].elements_connectivity[element_connectivity_data_id][iii];
              int node_data_id = -1;
              if (std::binary_search(results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id))
              {
                auto lower = std::lower_bound(results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin(), results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].end(), node_id);
                node_data_id = results->frd_data[frd_data_id].sorted_result_node_data_ids[frd_data_id][lower - results->frd_data[frd_data_id].sorted_result_node_ids[frd_data_id].begin()];
                values.push_back(results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id]);
              }
            }
            // get min and max in element
            for (size_t iii = 0; iii < values.size(); iii++)
            {
              if (iii==0)
              {
                min_value = values[iii];
                max_value = values[iii];
              }else{
                if (values[iii] < min_value)
                {
                  min_value = values[iii];
                }
                if (values[iii] > max_value)
                {
                  max_value = values[iii];
                }
              }
            }
            // check if limit is reached
            if (max_value - min_value > limit)
            {
              tmp.push_back(element_id);
            }
          }
        }
      }
    }
  }
  return tmp;
}

double CalculiXCore::frd_get_node_value(int job_id,int node_id, int total_increment,std::string result_block_type,std::string result_block_component)
{
  double tmp = 0; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        //check for right result_block_component
        int result_block_data_id = results->frd_data[frd_data_id].result_blocks[i][6];
        int component_id = results->frd_data[frd_data_id].get_result_block_component_id(frd_result_block_type_data_id,result_block_component);
        if (component_id != -1)
        {
          // loop over all nodes in result block
          for (size_t ii = 0; ii < results->frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
          {
            int result_node_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double value = results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if (result_node_id == node_id)
            {
              return value;
            }            
          }
        }
      }
    }
  }

  return tmp;
}

std::vector<double> CalculiXCore::frd_get_node_values(int job_id,int node_id, int total_increment,std::string result_block_type)
{
  std::vector<double> tmp; 

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int frd_data_id = results->get_frd_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->frd_data[frd_data_id].result_blocks.size(); i++)
  {
    //check for right total increment
    if (results->frd_data[frd_data_id].result_blocks[i][3] == total_increment)
    {
      //check for right result_block_type
      int frd_result_block_type_data_id = results->frd_data[frd_data_id].result_blocks[i][5];
      std::string frd_result_block_type = results->frd_data[frd_data_id].result_block_type[frd_result_block_type_data_id];

      if (frd_result_block_type == result_block_type)
      {
        // loop over all nodes in result block
        for (size_t ii = 0; ii < results->frd_data[frd_data_id].result_block_node_data[i].size(); ii++)
        {
          int result_node_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][0];
          int node_data_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][1];
          if (result_node_id == node_id)
          {
            return results->frd_data[frd_data_id].result_block_data[i][node_data_id];
          }            
        }
      }
    }
  }

  return tmp;
}

std::vector<std::string> CalculiXCore::dat_get_result_block_types(int job_id)
{
  std::vector<std::string> tmp;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  tmp = results->dat_data[dat_data_id].result_block_type;
      
  return tmp;
}

std::vector<std::string> CalculiXCore::dat_get_result_block_set(int job_id)
{
  std::vector<std::string> tmp;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  tmp = results->dat_data[dat_data_id].result_block_set;
      
  return tmp;
}

std::vector<std::string> CalculiXCore::dat_get_result_block_components(int job_id, std::string result_block_type)
{
  std::vector<std::string> tmp;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  tmp = results->dat_data[dat_data_id].get_result_block_components_from_result_block_type(result_block_type);

  return tmp;
}

std::vector<double> CalculiXCore::dat_get_result_block_times(int job_id, std::string result_block_type, std::string result_block_set)
{
  std::vector<double> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      tmp.push_back(results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]);
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_result_block_nodes(int job_id, double time, std::string result_block_type, std::string result_block_set)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1) //check if nodal values
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          //int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          tmp.push_back(c1);
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_result_block_elements(int job_id, double time, std::string result_block_type, std::string result_block_set)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          //int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          tmp.push_back(c1);
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_node_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1) //check if nodal values
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((value >= lower_value)&&(value <= upper_value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_node_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1) //check if nodal values
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double c1_value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((c1_value <= value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_node_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1) //check if nodal values
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double c1_value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((c1_value >= value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_element_ids_between_values(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double lower_value,double upper_value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((value >= lower_value)&&(value <= upper_value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_element_ids_smaller_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double c1_value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((c1_value <= value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_element_ids_greater_value(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double value)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double c1_value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          if ((c1_value >= value))
          {
            tmp.push_back(c1);
          }
        }
      }
    }
  }
  return tmp;
}

std::vector<int> CalculiXCore::dat_get_element_ids_over_limit(int job_id,double time,std::string result_block_type,std::string result_block_set,std::string result_block_component,double limit)
{
  std::vector<int> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);
  int last_element_id = -1;

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      std::vector<double> values;

      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==2) //check if elemental values
        { 
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];

          if (ii==0)
          {
            last_element_id = c1;
          } 
          
          if (last_element_id!=c1)
          {
            double min_value=0;
            double max_value=0;
            // get min and max in element
            for (size_t iii = 0; iii < values.size(); iii++)
            {
              if (iii==0)
              {
                min_value = values[iii];
                max_value = values[iii];
              }else{
                if (values[iii] < min_value)
                {
                  min_value = values[iii];
                }
                if (values[iii] > max_value)
                {
                  max_value = values[iii];
                }
              }
            }
            // check if limit is reached
            if (max_value - min_value > limit)
            {
              tmp.push_back(last_element_id);
            }
            //reset for next element
            last_element_id = c1;
            values.clear();
            values.push_back(value);
          }else if (results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size()==ii+1)
          {
            values.push_back(value);

            double min_value=0;
            double max_value=0;
            // get min and max in element
            for (size_t iii = 0; iii < values.size(); iii++)
            {
              if (iii==0)
              {
                min_value = values[iii];
                max_value = values[iii];
              }else{
                if (values[iii] < min_value)
                {
                  min_value = values[iii];
                }
                if (values[iii] > max_value)
                {
                  max_value = values[iii];
                }
              }
            }
            // check if limit is reached
            if (max_value - min_value > limit)
            {
              tmp.push_back(c1);
            }
          }else{
            values.push_back(value);
          }
        }
      }
    }
  }
  return tmp;
}

double CalculiXCore::dat_get_node_value(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component)
{
  double tmp = 0;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if ((results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0]==node_id)&&(results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1)) //check if nodal values and if node id fits
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double tmp = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          return tmp;
        }
      }
    }
  }
  return tmp;
}

std::vector<double> CalculiXCore::dat_get_node_values(int job_id,int node_id, double time,std::string result_block_type,std::string result_block_set)
{
  std::vector<double> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        if ((results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0]==node_id)&&(results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][2]==1)) //check if nodal values and if node id fits
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          for (size_t iii = 0; iii < results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id].size(); iii++)
          {
            double value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][iii];
            tmp.push_back(value);
          }
          return tmp;
        }
      }
    }
  }
  return tmp;
}

std::vector<double> CalculiXCore::dat_get_element_values_for_component(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set,std::string result_block_component)
{
  std::vector<double> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int result_block_component_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);
  if (result_block_type_data_id!=-1)
  {
    result_block_component_id = results->dat_data[dat_data_id].get_result_block_component_id(result_block_type_data_id,result_block_component);
  }

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1)||(result_block_component_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        const auto& record = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii];
        if (record[0] == element_id && (record[2] == 2 || record[2] == 3))
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          double value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][result_block_component_id];
          tmp.push_back(value);
        }
      }
    }
  }
  return tmp;
}

std::vector<std::vector<double>> CalculiXCore::dat_get_element_values(int job_id,int element_id, double time,std::string result_block_type,std::string result_block_set)
{
  std::vector<std::vector<double>> tmp;
  int result_block_type_data_id = -1;
  int result_block_set_data_id = -1;
  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  result_block_type_data_id = results->dat_data[dat_data_id].get_result_block_type_data_id(result_block_type);
  result_block_set_data_id = results->dat_data[dat_data_id].get_result_block_set_data_id(result_block_set);

  if ((result_block_type_data_id==-1)||(result_block_set_data_id==-1))
  {
    return tmp;
  }
  
  for (size_t i = 0; i < results->dat_data[dat_data_id].result_blocks.size(); i++)
  {
    if ((results->dat_data[dat_data_id].total_times[results->dat_data[dat_data_id].result_blocks[i][1]]==time)&&(results->dat_data[dat_data_id].result_blocks[i][2]==result_block_type_data_id)&&(results->dat_data[dat_data_id].result_blocks[i][3]==result_block_set_data_id))
    {
      int result_block_data_id = results->dat_data[dat_data_id].result_blocks[i][4];
      
      for (size_t ii = 0; ii < results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id].size(); ii++)
      {
        const auto& record = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii];
        if (record[0] == element_id && (record[2] == 2 || record[2] == 3)) //check if element values and if node id fits
        {
          int c1 = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][0];
          int result_block_c1_data_id = results->dat_data[dat_data_id].result_block_c1_data[result_block_data_id][ii][1];
          std::vector<double> values;
          for (size_t iii = 0; iii < results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id].size(); iii++)
          {
            double value = results->dat_data[dat_data_id].result_block_data[result_block_data_id][result_block_c1_data_id][iii];
            values.push_back(value);
          }
          tmp.push_back(values);
        }
      }
    }
  }
  return tmp;
}

std::vector<std::vector<std::vector<double>>> CalculiXCore::dat_get_buckle(int job_id)
{
  std::vector<std::vector<std::vector<double>>> tmp;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  return results->dat_data[dat_data_id].buckle_data;
}

std::vector<std::string> CalculiXCore::dat_get_section_set(int job_id)
{
  std::vector<std::string> tmp;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  return results->dat_data[dat_data_id].result_section_set;
}

std::vector<std::vector<double>> CalculiXCore::dat_get_section_data(int job_id, std::string section_set)
{
  std::vector<std::vector<double>> tmp;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  for (size_t i = 0; i < results->dat_data[dat_data_id].result_section.size(); i++)
  {
    if (results->dat_data[dat_data_id].result_section_set[results->dat_data[dat_data_id].result_section[i][0]]==section_set)
    {
      tmp.push_back(results->dat_data[dat_data_id].result_section_data[results->dat_data[dat_data_id].result_section[i][1]]);
    }
  }

  return tmp;
}

std::vector<std::string> CalculiXCore::dat_get_section_label(int job_id)
{
  std::vector<std::string> tmp;

  int results_data_id = results->get_results_data_id_from_job_id(job_id);
  int dat_data_id = results->get_dat_data_id_from_job_id(job_id);

  if (results_data_id == -1)
  {
    return tmp;
  }

  return results->dat_data[dat_data_id].result_section_label;
}

QIcon* CalculiXCore::getIcon(std::string name)
{
  QIcon* icon = new QIcon();
  std::string filepath = "";

  filepath = ccx_uo.mPathIcons.toStdString() + name + ".svg";

  #ifdef WIN32
    if (_access(filepath.c_str(), 0) == 0)
    {
      icon = new QIcon(QString::fromStdString(filepath));
    }else{
    }
  #else
    if (access(filepath.c_str(), F_OK) == 0) 
    {
      icon = new QIcon(QString::fromStdString(filepath));
    }else{
    }
  #endif

  return icon;
}

QIcon CalculiXCore::getIcon2(std::string name)
{
  QIcon icon;
  std::string filepath = "";

  filepath = ccx_uo.mPathIcons.toStdString() + name + ".svg";

  #ifdef WIN32
    if (_access(filepath.c_str(), 0) == 0)
    {
      icon = QIcon(QString::fromStdString(filepath));
    }else{
    }
  #else
    if (access(filepath.c_str(), F_OK) == 0) 
    {
      icon = QIcon(QString::fromStdString(filepath));
    }else{
    }
  #endif

  return icon;
}

bool CalculiXCore::prepare_export()
{
  bool status = true;
  std::vector<int> check;
  
  check = this->get_loadstrajectory_ids();
  if (check.size()>0)
  {
    status = loadstrajectory->prepare_export();
  }

  return status;
}

bool CalculiXCore::clean_export()
{
  bool status = true;
  std::vector<int> check;
  
  check = this->get_loadstrajectory_ids();
  if (check.size()>0)
  {
    status = loadstrajectory->clean_export();
  }
  
  return status;
}

std::string CalculiXCore::get_material_export_data(int topology_id,int iteration_id) // gets the export data from materials core
{
  if (topology_id == -1 && iteration_id == -1)
  {
    return mat->get_material_export();
  }

  if (topology_id <= 0 || iteration_id < 1)
  {
    PRINT_ERROR("Invalid topology export selection.\n");
    return "";
  }

  const std::string topology_data = mat->get_topology_material_export(topology_id, iteration_id);

  if (topology_data.empty())
  {
    return "";
  }

  // Original materials remain available for unaffected sections.
  return mat->get_material_export() + "\n" + topology_data;
}

std::string CalculiXCore::get_section_export_data(int topology_id, int iteration_id) // gets the export data from sections core
{
  return sections->get_section_export(topology_id, iteration_id);
}

std::string CalculiXCore::get_constraint_export_data() // gets the export data from constraints core
{
  return constraints->get_constraint_export();
}

std::string CalculiXCore::get_surfaceinteraction_export_data() // gets the export data from surfaceinteractions core
{
  return surfaceinteractions->get_surfaceinteraction_export();
}

std::string CalculiXCore::get_contactpair_export_data() // gets the export data from contactpairs core
{
  return contactpairs->get_contactpair_export();
}

std::string CalculiXCore::get_amplitude_export_data() // gets the export data from amplitudes core
{
  return amplitudes->get_amplitude_export();
}

std::string CalculiXCore::get_orientation_export_data() // gets the export data from orientation core
{
  return orientations->get_orientation_export();
}

std::string CalculiXCore::get_damping_export_data() // gets the export data from damping core
{
  return damping->get_damping_export_data();
}

std::string CalculiXCore::get_physicalconstants_export_data() // gets the export data from physicalconstants core
{
  return physicalconstants->get_physicalconstants_export_data();
}

std::string CalculiXCore::get_initialcondition_export_data() // gets the export data from core
{
  std::vector<std::string> initialconditions_export_list;
  initialconditions_export_list.push_back("********************************** I N I T I A L C O N D I T I O N S ****************************");
  std::string str_temp;
  std::string log;
  int sub_data_id;
  std::vector<int> sub_data_ids;
  std::string command;
  int bc_set_id=-1;
  BCSetHandle bc_set;
  std::vector<BCSetHandle> bc_sets;
  NodesetHandle nodeset;
  std::vector<BCEntityHandle> bc_handles;
  std::vector<MeshExportBCData> bc_attribs; 

  //loop over all initialconditions
  for (size_t i = 0; i < initialconditions->initialconditions_data.size(); i++)
  { 
    if (i==0)
    { 
      log = "Creating BCSet for exporting Initial Conditions.\n";
      PRINT_INFO("%s", log.c_str());
      if (me_iface->get_bcset_list(bc_sets))
      {
        //bc_set_id = me_iface->id_from_handle(bc_sets[bc_sets.size()-1]);
        bc_set_id = bc_sets.size();     
        me_iface->create_default_bcset(bc_set_id,true,true,true,bc_set);
      }else{
        me_iface->create_default_bcset(0,true,true,true,bc_set);
      }
      me_iface->get_bc_restraints(bc_set, bc_handles);
      bc_set_id = me_iface->id_from_handle(bc_set);
    }

    // CUSTOMLINE START
    std::vector<std::string> customline = customlines->get_customline_data("BEFORE","INITIALCONDITION",initialconditions->initialconditions_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      initialconditions_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END

    for (size_t ii = 0; ii < bc_handles.size(); ii++)
    {  
      me_iface->get_bc_attributes(bc_handles[ii],bc_attribs);
    
      if ((get_bc_fea_type(bc_attribs)==1) && (initialconditions->initialconditions_data[i][1]==1))
      {
        sub_data_id = initialconditions->get_displacement_data_id_from_displacement_id(initialconditions->initialconditions_data[i][2]);
        if (initialconditions->displacement_data[sub_data_id][1]!="")
        {
          if (std::stoi(initialconditions->displacement_data[sub_data_id][1])==me_iface->id_from_handle(bc_handles[ii]))
          {
            me_iface->get_bc_nodeset(bc_handles[ii],nodeset);
            str_temp = "*INITIAL CONDITIONS,TYPE=DISPLACEMENT";
            initialconditions_export_list.push_back(str_temp);
            for (size_t iii = 0; iii < bc_attribs.size(); iii++)
            { 
              str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + std::to_string(bc_attribs[iii].first+1) + "," + to_string_scientific(bc_attribs[iii].second);
              initialconditions_export_list.push_back(str_temp);
            }

          }
        }
      }else if ((get_bc_fea_type(bc_attribs)==4) && (initialconditions->initialconditions_data[i][1]==2))
      {
        sub_data_id = initialconditions->get_temperature_data_id_from_temperature_id(initialconditions->initialconditions_data[i][2]);
        if (initialconditions->temperature_data[sub_data_id][1]!="")
        {
          if (std::stoi(initialconditions->temperature_data[sub_data_id][1])==me_iface->id_from_handle(bc_handles[ii]))
          {
            me_iface->get_bc_nodeset(bc_handles[ii],nodeset);
            str_temp = "*INITIAL CONDITIONS,TYPE=TEMPERATURE";
            initialconditions_export_list.push_back(str_temp);
            for (size_t iii = 0; iii < bc_attribs.size(); iii++)
            { 
              str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + to_string_scientific(bc_attribs[iii].second);
              initialconditions_export_list.push_back(str_temp);
            }
          }
        }
      }
      bc_attribs.clear();
    }
    if (initialconditions->initialconditions_data[i][1]==3)
    {
      str_temp = "*INITIAL CONDITIONS,TYPE=STRESS";
      initialconditions_export_list.push_back(str_temp);

      sub_data_ids = initialconditions->get_stress_element_data_ids_from_stress_id(initialconditions->initialconditions_data[i][2]);
      for (size_t ii = 0; ii < sub_data_ids.size(); ii++)
      {
        std::vector<double> data = initialconditions->stress_element_data[sub_data_ids[ii]];
        str_temp = std::to_string(int(data[1])) + "," + std::to_string(int(data[2])) + "," + to_string_scientific(data[3]) + "," + to_string_scientific(data[4]) + "," + to_string_scientific(data[5]) + "," + to_string_scientific(data[7]) + "," + to_string_scientific(data[8]);
        initialconditions_export_list.push_back(str_temp);
      }

      sub_data_ids = initialconditions->get_stress_block_data_ids_from_stress_id(initialconditions->initialconditions_data[i][2]);
      for (size_t ii = 0; ii < sub_data_ids.size(); ii++)
      {
        std::vector<double> data = initialconditions->stress_block_data[sub_data_ids[ii]];
        
        std::string element_type = CubitInterface::get_block_element_type(int(data[1]));
        int ip = this->get_ccx_element_type_integration_points(int(data[1]));
        std::vector<int> ids = this->get_block_element_ids(int(data[1]));
        
        for (size_t iii = 0; iii < ids.size(); iii++)
        {
          for (size_t iv = 0; iv < ip; iv++)
          {
            str_temp = std::to_string(ids[iii]) + "," + std::to_string(iv+1) + "," +  to_string_scientific(data[2]) + "," + to_string_scientific(data[3]) + "," + to_string_scientific(data[4]) + "," + to_string_scientific(data[5]) + "," + to_string_scientific(data[6]) + "," + to_string_scientific(data[7]);
            initialconditions_export_list.push_back(str_temp);
          }
        }      
      }
    }
    
    // CUSTOMLINE START
    customline = customlines->get_customline_data("AFTER","INITIALCONDITION",initialconditions->initialconditions_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      initialconditions_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END
  }
  
  if (bc_set_id!=-1)
  {
    log = "Deleting BCSet Initial Conditions Export.\n";
    PRINT_INFO("%s", log.c_str());
    command = "delete bcset " + std::to_string(bc_set_id);
    CubitInterface::cmd(command.c_str());
  }

  std::string initialcondition_export;

  for (size_t i = 0; i < initialconditions_export_list.size(); i++)
  {
    initialcondition_export.append(initialconditions_export_list[i] + "\n");
  }

  return initialcondition_export;
}

std::string CalculiXCore::get_hbc_export_data() // gets the export data from core
{
  std::vector<std::string> hbcs_export_list;
  hbcs_export_list.push_back("********************************** H B C s ****************************");
  std::string str_temp;
  std::vector<std::vector<std::string>> temp_list;
  std::string log;
  std::vector<int> sub_data_ids;
  std::string command;
  int bc_set_id=-1;
  BCSetHandle bc_set;
  std::vector<BCSetHandle> bc_sets;
  NodesetHandle nodeset;
  std::vector<BCEntityHandle> bc_handles;
  std::vector<MeshExportBCData> bc_attribs; 
  std::vector<std::string> customline;

  log = "Creating BCSet for exporting Homogeneous Boundary Conditions.\n";
  PRINT_INFO("%s", log.c_str());
  if (me_iface->get_bcset_list(bc_sets))
  {
    //bc_set_id = me_iface->id_from_handle(bc_sets[bc_sets.size()-1]);
    bc_set_id = bc_sets.size();     
    me_iface->create_default_bcset(bc_set_id,true,true,true,bc_set);
  }else{
    me_iface->create_default_bcset(1,true,true,true,bc_set);
  }
  bc_set_id = me_iface->id_from_handle(bc_set);
  
  // BCs
  me_iface->get_bc_restraints(bc_set, bc_handles);
  sub_data_ids = hbcs->get_bc_data_ids_from_bcs_id(0);
  for (size_t ii = 0; ii < bc_handles.size(); ii++)
  {  
    me_iface->get_bc_attributes(bc_handles[ii],bc_attribs);
    for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
    { 
      // DISPLACEMENT
      if ((get_bc_fea_type(bc_attribs)==1) && (hbcs->bcs_data[sub_data_ids[iii]][1]==1))
      {
        if (hbcs->bcs_data[sub_data_ids[iii]][2]==me_iface->id_from_handle(bc_handles[ii]))
        { 
          // CUSTOMLINE START
          customline = customlines->get_customline_data("BEFORE","DISPLACEMENT",hbcs->bcs_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            hbcs_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
          me_iface->get_bc_nodeset(bc_handles[ii],nodeset);
          str_temp = "*BOUNDARY";
          str_temp.append(bcsdisplacements->get_bc_parameter_export(hbcs->bcs_data[sub_data_ids[iii]][2]));
          hbcs_export_list.push_back(str_temp);
          for (size_t iv = 0; iv < bc_attribs.size(); iv++)
          { 
            //str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + std::to_string(bc_attribs[iv].first+1) + "," + std::to_string(bc_attribs[iv].first+1) + "," + to_string_scientific(bc_attribs[iv].second);
            str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + std::to_string(bc_attribs[iv].first+1) + "," + std::to_string(bc_attribs[iv].first+1);
            hbcs_export_list.push_back(str_temp);
          }
          // CUSTOMLINE START
          customline = customlines->get_customline_data("AFTER","DISPLACEMENT",hbcs->bcs_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            hbcs_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
        }  
      }
      // TEMPERATURE
      if ((get_bc_fea_type(bc_attribs)==4) && (hbcs->bcs_data[sub_data_ids[iii]][1]==2))
      {
        if (hbcs->bcs_data[sub_data_ids[iii]][2]==me_iface->id_from_handle(bc_handles[ii]))
        {
          // CUSTOMLINE START
          customline = customlines->get_customline_data("BEFORE","TEMPERATURE",hbcs->bcs_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            hbcs_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
          me_iface->get_bc_nodeset(bc_handles[ii],nodeset);
          str_temp = "*BOUNDARY";
          str_temp.append(bcstemperatures->get_bc_parameter_export(hbcs->bcs_data[sub_data_ids[iii]][2]));
          hbcs_export_list.push_back(str_temp);
          for (size_t iv = 0; iv < bc_attribs.size(); iv++)
          { 
            str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + ",11,11";
            hbcs_export_list.push_back(str_temp);
          }
          // CUSTOMLINE START
          customline = customlines->get_customline_data("AFTER","TEMPERATURE",hbcs->bcs_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            hbcs_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
        }  
      }
    }
    bc_attribs.clear();
  }
  
  log = "Deleting BCSet HBCs Export.\n";
  PRINT_INFO("%s", log.c_str());
  command = "delete bcset " + std::to_string(bc_set_id);
  CubitInterface::cmd(command.c_str());

  std::string hbc_export;

  for (size_t i = 0; i < hbcs_export_list.size(); i++)
  {
    hbc_export.append(hbcs_export_list[i] + "\n");
  }
  return hbc_export;
}

std::string CalculiXCore::get_step_export_data() // gets the export data from core
{
  if (is_topology_adjoint_export())
  {
    double objective_value = 0.0;
    int node_id = -1;
    std::array<double, 3> load{};

    if (!build_topology_adjoint_load(job_export_context.topology_id, job_export_context.iteration_id, objective_value, node_id, load))
    {
      return "";
    }

    if (objective_value == 0.0)
    {
      return "";
    }

    // Accumulated prescribed DOFs for the supported OP=MOD case.
    std::set<std::pair<int, int>> boundaries;

    const auto collect_bc = [&](int bc_id) -> bool
    {
      const int index = bcsdisplacements->get_bcs_data_id_from_bc_id(bc_id);

      if (index < 0 || bcsdisplacements->bcs_data[index].size() < 10)
      {
        PRINT_ERROR("Missing displacement BC %d.\n", bc_id);
        return false;
      }

      if (bcsdisplacements->bcs_data[index][1] != 0)
      {
        PRINT_ERROR("Adjoint export does not yet support OP=NEW in displacement BC %d.\n", bc_id);
        return false;
      }

      BCEntityHandle handle;
      NodesetHandle nodeset;
      std::vector<MeshExportBCData> attributes;

      if (!me_iface->get_bc_handle(BCTYPE_DISPLACEMENT, bc_id, handle) || !me_iface->get_bc_nodeset(handle, nodeset) || !me_iface->get_bc_attributes(handle, attributes))
      {
        PRINT_ERROR("Unable to query displacement BC %d.\n", bc_id);
        return false;
      }

      const int nodeset_id = me_iface->id_from_handle(nodeset);

      if (nodeset_id <= 0 || CubitInterface::get_nodeset_nodes_inclusive(nodeset_id).empty())
      {
        PRINT_ERROR("Displacement BC %d has a missing or empty nodeset.\n", bc_id);
        return false;
      }

      for (const auto& attribute : attributes)
      {
        if (attribute.first < 0 || attribute.first >= 6 || !std::isfinite(attribute.second))
        {
          PRINT_ERROR("Invalid displacement attributes for BC %d.\n", bc_id);
          return false;
        }

        boundaries.emplace(nodeset_id, attribute.first + 1);
      }

      return true;
    };

    for (const auto& bc : hbcs->bcs_data)
    {
      if (bc.size() < 3 || bc[1] != 1 || !collect_bc(bc[2]))
      {
        PRINT_ERROR("Unable to collect model-level adjoint boundary conditions.\n");
        return "";
      }
    }

    for (const auto& step : steps->steps_data)
    {
      for (int index : steps->get_bc_data_ids_from_bcs_id(step[6]))
      {
        if (index < 0 || static_cast<std::size_t>(index) >= steps->bcs_data.size())
        {
          PRINT_ERROR("Invalid step boundary assignment.\n");
          return "";
        }

        const auto& bc = steps->bcs_data[index];

        if (bc.size() < 3 || bc[1] != 1 || !collect_bc(bc[2]))
        {
          PRINT_ERROR("Unable to collect adjoint boundary conditions for step %d.\n", step[0]);
          return "";
        }
      }
    }

    if (!steps || steps->steps_data.empty())
    {
      PRINT_ERROR("No analysis step is available for the adjoint solve.\n");
      return "";
    }

    const auto& final_step = steps->steps_data.back();

    if (final_step.size() < 9)
    {
      PRINT_ERROR("Invalid final analysis step data.\n");
      return "";
    }

    const std::string step_definition = steps->get_step_export(final_step[0]);

    if (step_definition.empty())
    {
      PRINT_ERROR("Unable to export the final analysis step definition.\n");
      return "";
    }

    std::ostringstream output;
    output << "** Topology adjoint calculation\n";
    output << step_definition;

    if (step_definition.back() != '\n')
    {
      output << "\n";
    }

    if (!boundaries.empty())
    {
      output << "*BOUNDARY\n";

      for (const auto& boundary : boundaries)
      {
        const std::string nodeset_name = get_nodeset_name(boundary.first);

        if (nodeset_name.empty())
        {
          PRINT_ERROR("Missing adjoint boundary nodeset name.\n");
          return "";
        }

        output << nodeset_name << "," << boundary.second << "," << boundary.second << ",0.000000\n";
      }
    }

    output << "*CLOAD\n";

    for (std::size_t axis = 0; axis < load.size(); ++axis)
    {
      if (load[axis] != 0.0)
      {
        output << node_id << "," << axis + 1 << "," << to_string_scientific(load[axis]) << "\n";
      }
    }

    if (!fieldoutputs)
    {
      PRINT_ERROR("Field output core is not initialized.\n");
      return "";
    }

    if (!historyoutputs)
    {
      PRINT_ERROR("History output core is not initialized.\n");
      return "";
    }

    for (int index : steps->get_historyoutput_data_ids_from_historyoutputs_id(final_step[7]))
    {
      if (index < 0 || static_cast<std::size_t>(index) >= steps->historyoutputs_data.size() || steps->historyoutputs_data[index].size() < 2)
      {
        PRINT_ERROR("Invalid history output assignment for the final analysis step.\n");
        return "";
      }

      const int output_id = steps->historyoutputs_data[index][1];

      if (historyoutputs->get_outputs_data_id_from_output_id(output_id) < 0)
      {
        PRINT_ERROR("History output %d no longer exists.\n", output_id);
        return "";
      }

      const std::string data = historyoutputs->get_output_export(output_id);

      if (data.empty())
      {
        PRINT_ERROR("Unable to export history output %d.\n", output_id);
        return "";
      }

      output << data;

      if (data.back() != '\n')
      {
        output << "\n";
      }
    }

    for (int index : steps->get_fieldoutput_data_ids_from_fieldoutputs_id(final_step[8]))
    {
      if (index < 0 || static_cast<std::size_t>(index) >= steps->fieldoutputs_data.size() || steps->fieldoutputs_data[index].size() < 2)
      {
        PRINT_ERROR("Invalid field output assignment for the final analysis step.\n");
        return "";
      }

      const int output_id = steps->fieldoutputs_data[index][1];

      if (fieldoutputs->get_outputs_data_id_from_output_id(output_id) < 0)
      {
        PRINT_ERROR("Field output %d no longer exists.\n", output_id);
        return "";
      }

      const std::string data = fieldoutputs->get_output_export(output_id);

      if (data.empty())
      {
        PRINT_ERROR("Unable to export field output %d.\n", output_id);
        return "";
      }

      output << data;

      if (data.back() != '\n')
      {
        output << "\n";
      }
    }
    output << "*END STEP\n";

    return output.str();
  }
  std::vector<std::string> steps_export_list;
  steps_export_list.push_back("********************************** S T E P S ****************************");
  std::string str_temp;
  std::vector<std::vector<std::string>> temp_list;
  std::string log;
  std::vector<int> sub_data_ids;
  std::string command;
  int bc_set_id=-1;
  BCSetHandle bc_set;
  std::vector<BCSetHandle> bc_sets;
  NodesetHandle nodeset;
  SidesetHandle sideset;
  std::vector<BCEntityHandle> bc_handles;
  std::vector<MeshExportBCData> bc_attribs; 
  std::vector<std::string> customline;

  //loop over all steps
  for (size_t i = 0; i < steps->steps_data.size(); i++)
  { 
    if (i==0)
    { 
      log = "Creating BCSet for exporting Steps.\n";
      PRINT_INFO("%s", log.c_str());
      if (me_iface->get_bcset_list(bc_sets))
      {
        //bc_set_id = me_iface->id_from_handle(bc_sets[bc_sets.size()-1]);
        bc_set_id = bc_sets.size();     
        me_iface->create_default_bcset(bc_set_id,true,true,true,bc_set);
      }else{
        me_iface->create_default_bcset(2,true,true,true,bc_set);
      }
      bc_set_id = me_iface->id_from_handle(bc_set);
    }
    str_temp = steps->get_step_export(steps->steps_data[i][0]);
    steps_export_list.push_back(str_temp);        
    // Loads
    me_iface->get_bc_loads(bc_set, bc_handles);
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i][5]);
    for (size_t ii = 0; ii < bc_handles.size(); ii++)
    {  
      me_iface->get_bc_attributes(bc_handles[ii],bc_attribs);
      for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
      { 
        // FORCE CLOAD
        if ((get_bc_fea_type(bc_attribs)==5) && (steps->loads_data[sub_data_ids[iii]][1]==1))
        {
          if (steps->loads_data[sub_data_ids[iii]][2]==me_iface->id_from_handle(bc_handles[ii]))
          {
            // CUSTOMLINE START
            customline = customlines->get_customline_data("BEFORE","FORCE",steps->loads_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
            // CUSTOMLINE END
            
            me_iface->get_bc_nodeset(bc_handles[ii],nodeset);
            str_temp = "*CLOAD";
            str_temp.append(loadsforces->get_load_parameter_export(steps->loads_data[sub_data_ids[iii]][2]));
            steps_export_list.push_back(str_temp);
            for (size_t iv = 1; iv < 4; iv++)
            {
              //check if zero component
              if (bc_attribs[iv].second*bc_attribs[0].second != 0.0)
              {
                str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + std::to_string(iv) + "," + to_string_scientific(bc_attribs[iv].second*bc_attribs[0].second);
                steps_export_list.push_back(str_temp);
              }
            }
            for (size_t iv = 1; iv < 4; iv++)
            {
              if (bc_attribs[iv+4].second*bc_attribs[4].second != 0.0)
              {
                str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + std::to_string(iv+3) + "," + to_string_scientific(bc_attribs[iv+4].second*bc_attribs[4].second);
                steps_export_list.push_back(str_temp);
              }
            }
            // CUSTOMLINE START
            customline = customlines->get_customline_data("AFTER","FORCE",steps->loads_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
            // CUSTOMLINE END
          }  
        }
        // PRESSURE DLOAD 
        if ((get_bc_fea_type(bc_attribs)==6) && (steps->loads_data[sub_data_ids[iii]][1]==2))
        {
          if (steps->loads_data[sub_data_ids[iii]][2]==me_iface->id_from_handle(bc_handles[ii]))
          {
            // CUSTOMLINE START
            customline = customlines->get_customline_data("BEFORE","PRESSURE",steps->loads_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
            // CUSTOMLINE END
            
            me_iface->get_bc_sideset(bc_handles[ii],sideset);
            temp_list = get_sideset_face(me_iface->id_from_handle(sideset));
            int element_type = std::stoi(temp_list[0][3]);
            // check if shell element
            if ((element_type>=15) && (element_type<=22))
            {
              for (size_t iv = 0; iv < temp_list.size(); iv++)
              {              
                str_temp = "*DLOAD";
                str_temp.append(loadspressures->get_load_parameter_export(steps->loads_data[sub_data_ids[iii]][2]));
                steps_export_list.push_back(str_temp);
                // check if edge load or not
                if (std::stoi(temp_list[0][2])>2)
                {
                  int edge = std::stoi(temp_list[iv][2])-2;
                  str_temp = temp_list[iv][1] + ",EDNOR" + std::to_string(edge) + "," + to_string_scientific(bc_attribs[0].second);
                }else{
                  // check direction of load, invert +- if necessary
                  if (std::stoi(temp_list[0][2])==1) // negativ normal direction->invert load
                  {
                    str_temp = temp_list[iv][1] + ",P," + to_string_scientific(bc_attribs[0].second*(-1));
                  }else{
                    str_temp = temp_list[iv][1] + ",P," + to_string_scientific(bc_attribs[0].second);
                  }
                }
                steps_export_list.push_back(str_temp);
              }
            }else{
              for (size_t iv = 0; iv < temp_list.size(); iv++)
              {              
                str_temp = "*DLOAD";
                str_temp.append(loadspressures->get_load_parameter_export(steps->loads_data[sub_data_ids[iii]][2]));
                steps_export_list.push_back(str_temp);

                str_temp = temp_list[iv][1] + ",P" + temp_list[iv][2] + "," + to_string_scientific(bc_attribs[0].second);
                steps_export_list.push_back(str_temp);
              }
            }
            // CUSTOMLINE START
            customline = customlines->get_customline_data("AFTER","PRESSURE",steps->loads_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
          }  
        }
        // HEATFLUX DFLUX
        if ((get_bc_fea_type(bc_attribs)==7) && (steps->loads_data[sub_data_ids[iii]][1]==3))
        {
          if (steps->loads_data[sub_data_ids[iii]][2]==me_iface->id_from_handle(bc_handles[ii]))
          {
            // CUSTOMLINE START
            customline = customlines->get_customline_data("BEFORE","HEATFLUX",steps->loads_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
            me_iface->get_bc_sideset(bc_handles[ii],sideset);
            temp_list = get_sideset_face(me_iface->id_from_handle(sideset));
            for (size_t iv = 0; iv < temp_list.size(); iv++)
            {              
              str_temp = "*DFLUX";
              str_temp.append(loadsheatfluxes->get_load_parameter_export(steps->loads_data[sub_data_ids[iii]][2]));
              steps_export_list.push_back(str_temp);

              str_temp = temp_list[iv][1] + ",S" + temp_list[iv][2] + "," + to_string_scientific(bc_attribs[0].second);
              steps_export_list.push_back(str_temp);
            }
            // CUSTOMLINE START
            customline = customlines->get_customline_data("AFTER","HEATFLUX",steps->loads_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
          }  
        }
      }
      bc_attribs.clear();
    }
    // GRAVITY DLOAD
    for (size_t ii = 0; ii < loadsgravity->loads_data.size(); ii++)
    {  
      for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
      { 
        if ((steps->loads_data[sub_data_ids[iii]][1]==4) && (steps->loads_data[sub_data_ids[iii]][2]==loadsgravity->loads_data[ii][0]))
        {
          // CUSTOMLINE START
          customline = customlines->get_customline_data("BEFORE","GRAVITY",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END

          str_temp = loadsgravity->get_load_export(steps->loads_data[sub_data_ids[iii]][2]);
          steps_export_list.push_back(str_temp);

          // CUSTOMLINE START
          customline = customlines->get_customline_data("AFTER","GRAVITY",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
        }
      }
    }
    // CENTRIFUGAL DLOAD
    for (size_t ii = 0; ii < loadscentrifugal->loads_data.size(); ii++)
    {  
      for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
      { 
        if ((steps->loads_data[sub_data_ids[iii]][1]==5) && (steps->loads_data[sub_data_ids[iii]][2]==loadscentrifugal->loads_data[ii][0]))
        {
          // CUSTOMLINE START
          customline = customlines->get_customline_data("BEFORE","CENTRIFUGAL",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END

          str_temp = loadscentrifugal->get_load_export(steps->loads_data[sub_data_ids[iii]][2]);
          steps_export_list.push_back(str_temp);

          // CUSTOMLINE START
          customline = customlines->get_customline_data("AFTER","CENTRIFUGAL",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
        }
      }
    }
    // TRAJECTORY WORKS DIFFERENT
    // FILM
    for (size_t ii = 0; ii < loadsfilm->loads_data.size(); ii++)
    {  
      for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
      { 
        if ((steps->loads_data[sub_data_ids[iii]][1]==7) && (steps->loads_data[sub_data_ids[iii]][2]==loadsfilm->loads_data[ii][0]))
        {
          // CUSTOMLINE START
          customline = customlines->get_customline_data("BEFORE","FILM",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END          
          
          temp_list = get_sideset_face(loadsfilm->loads_data[ii][4]);
          for (size_t iv = 0; iv < temp_list.size(); iv++)
          {              
            //get first line
            int load_data_id = loadsfilm->get_loads_data_id_from_load_id(loadsfilm->loads_data[ii][0]);
            str_temp = loadsfilm->get_load_export(loadsfilm->loads_data[ii][0]);
            steps_export_list.push_back(str_temp);
            
            int sub_data_id = loadsfilm->get_temperature_data_id_from_temperature_id(loadsfilm->loads_data[load_data_id][5]);
            std::string temperature = loadsfilm->temperature_data[sub_data_id][1];
            
            sub_data_id = loadsfilm->get_coefficient_data_id_from_coefficient_id(loadsfilm->loads_data[load_data_id][6]);
            std::string coefficient = loadsfilm->coefficient_data[sub_data_id][1];

            str_temp = temp_list[iv][1] + ",F" + temp_list[iv][2] + "," + temperature + "," + coefficient;
            steps_export_list.push_back(str_temp);
          }

          // CUSTOMLINE START
          customline = customlines->get_customline_data("AFTER","FILM",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
        }
      }
    }
    // RADIATION
    for (size_t ii = 0; ii < loadsradiation->loads_data.size(); ii++)
    {  
      for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
      { 
        if ((steps->loads_data[sub_data_ids[iii]][1]==8) && (steps->loads_data[sub_data_ids[iii]][2]==loadsradiation->loads_data[ii][0]))
        {
          // CUSTOMLINE START
          customline = customlines->get_customline_data("BEFORE","RADIATION",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END          
          temp_list = get_sideset_face(loadsradiation->loads_data[ii][4]);
          for (size_t iv = 0; iv < temp_list.size(); iv++)
          {              
            //get first line
            int load_data_id = loadsradiation->get_loads_data_id_from_load_id(loadsradiation->loads_data[ii][0]);
            str_temp = loadsradiation->get_load_export(loadsradiation->loads_data[ii][0]);
            steps_export_list.push_back(str_temp);
  
            int sub_data_id = loadsradiation->get_temperature_data_id_from_temperature_id(loadsradiation->loads_data[load_data_id][5]);
            std::string temperature = loadsradiation->temperature_data[sub_data_id][1];
            
            sub_data_id = loadsradiation->get_emissivity_data_id_from_emissivity_id(loadsradiation->loads_data[load_data_id][6]);
            std::string emissivity = loadsradiation->emissivity_data[sub_data_id][1];

            if (loadsradiation->loads_data[load_data_id][11]==1)
            {
              str_temp = temp_list[iv][1] + ",R" + temp_list[iv][2] + "CR," + temperature + "," + emissivity;
            }else{
              str_temp = temp_list[iv][1] + ",R" + temp_list[iv][2] + "," + temperature + "," + emissivity;
            }
            steps_export_list.push_back(str_temp);
          }

          // CUSTOMLINE START
          customline = customlines->get_customline_data("AFTER","RADIATION",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
        }
      }
    }
    // SURFACE TRACTION
    for (size_t ii = 0; ii < loadssurfacetraction->loads_data.size(); ii++)
    {  
      for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
      { 
        if ((steps->loads_data[sub_data_ids[iii]][1]==9) && (steps->loads_data[sub_data_ids[iii]][2]==loadssurfacetraction->loads_data[ii][0]))
        {
          // CUSTOMLINE START
          customline = customlines->get_customline_data("BEFORE","SURFACETRACTION",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END

          str_temp = loadssurfacetraction->get_load_export(steps->loads_data[sub_data_ids[iii]][2]);
          steps_export_list.push_back(str_temp);

          // CUSTOMLINE START
          customline = customlines->get_customline_data("AFTER","SURFACETRACTION",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
        }
      }
    }
    // SURFACE TRACTION
    for (size_t ii = 0; ii < loadsbodyheatflux->loads_data.size(); ii++)
    {  
      for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
      { 
        if ((steps->loads_data[sub_data_ids[iii]][1]==10) && (steps->loads_data[sub_data_ids[iii]][2]==loadsbodyheatflux->loads_data[ii][0]))
        {
          // CUSTOMLINE START
          customline = customlines->get_customline_data("BEFORE","BODYHEATFLUX",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END

          str_temp = loadsbodyheatflux->get_load_export(steps->loads_data[sub_data_ids[iii]][2]);
          steps_export_list.push_back(str_temp);

          // CUSTOMLINE START
          customline = customlines->get_customline_data("AFTER","BODYHEATFLUX",steps->loads_data[sub_data_ids[iii]][2]);
          for (size_t icl = 0; icl < customline.size(); icl++)
          {
            steps_export_list.push_back(customline[icl]);
          }
          // CUSTOMLINE END
        }
      }
    }
    // BCs
    me_iface->get_bc_restraints(bc_set, bc_handles);
    sub_data_ids = steps->get_bc_data_ids_from_bcs_id(steps->steps_data[i][6]);
    for (size_t ii = 0; ii < bc_handles.size(); ii++)
    {  
      me_iface->get_bc_attributes(bc_handles[ii],bc_attribs);
      for (size_t iii = 0; iii < sub_data_ids.size(); iii++)
      { 
        // DISPLACEMENT
        if ((get_bc_fea_type(bc_attribs)==1) && (steps->bcs_data[sub_data_ids[iii]][1]==1))
        {
          if (steps->bcs_data[sub_data_ids[iii]][2]==me_iface->id_from_handle(bc_handles[ii]))
          { 
            // CUSTOMLINE START
            customline = customlines->get_customline_data("BEFORE","DISPLACEMENT",steps->bcs_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
            // CUSTOMLINE END
            me_iface->get_bc_nodeset(bc_handles[ii],nodeset);
            str_temp = "*BOUNDARY";
            str_temp.append(bcsdisplacements->get_bc_parameter_export(steps->bcs_data[sub_data_ids[iii]][2]));
            steps_export_list.push_back(str_temp);
            for (size_t iv = 0; iv < bc_attribs.size(); iv++)
            { 
              //SKIP IF FIXED
              if (!bcsdisplacements->check_dof_fixed(steps->bcs_data[sub_data_ids[iii]][2],bc_attribs[iv].first+1))
              {
                str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + std::to_string(bc_attribs[iv].first+1) + "," + std::to_string(bc_attribs[iv].first+1) + "," + to_string_scientific(bc_attribs[iv].second);
                steps_export_list.push_back(str_temp);
              }
            }
            //FIXED DOFS
            if (bcsdisplacements->check_fixed(steps->bcs_data[sub_data_ids[iii]][2]))
            {
              str_temp = "*BOUNDARY";
              str_temp.append(bcsdisplacements->get_bc_parameter_fixed_export(steps->bcs_data[sub_data_ids[iii]][2]));
              steps_export_list.push_back(str_temp);
              for (size_t iv = 0; iv < bc_attribs.size(); iv++)
              { 
                //SKIP IF NOT FIXED
                if (bcsdisplacements->check_dof_fixed(steps->bcs_data[sub_data_ids[iii]][2],bc_attribs[iv].first+1))
                {
                  str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + std::to_string(bc_attribs[iv].first+1) + "," + std::to_string(bc_attribs[iv].first+1);
                  steps_export_list.push_back(str_temp);
                }
              }
            }
            // CUSTOMLINE START
            customline = customlines->get_customline_data("AFTER","DISPLACEMENT",steps->bcs_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
            // CUSTOMLINE END
          }  
        }
        // TEMPERATURE
        if ((get_bc_fea_type(bc_attribs)==4) && (steps->bcs_data[sub_data_ids[iii]][1]==2))
        {
          if (steps->bcs_data[sub_data_ids[iii]][2]==me_iface->id_from_handle(bc_handles[ii]))
          {
            // CUSTOMLINE START
            customline = customlines->get_customline_data("BEFORE","TEMPERATURE",steps->bcs_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
            // CUSTOMLINE END
            me_iface->get_bc_nodeset(bc_handles[ii],nodeset);
            if (bcstemperatures->get_bc_keyword_type(steps->bcs_data[sub_data_ids[iii]][2]) == 0) // *BOUNDARY
            {
              str_temp = "*BOUNDARY";
            }else if (bcstemperatures->get_bc_keyword_type(steps->bcs_data[sub_data_ids[iii]][2]) == 1) // *TEMPERATURE
            {
              str_temp = "*TEMPERATURE";
            }else{
              str_temp = "*SOMETHING WENT WRONG";
            }
            str_temp.append(bcstemperatures->get_bc_parameter_export(steps->bcs_data[sub_data_ids[iii]][2]));
            steps_export_list.push_back(str_temp);            
            if (bcstemperatures->get_bc_keyword_type(steps->bcs_data[sub_data_ids[iii]][2]) == 0) // *BOUNDARY
            {
              for (size_t iv = 0; iv < bc_attribs.size(); iv++)
              { 
                str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + ",11,11," + to_string_scientific(bc_attribs[iv].second);
                steps_export_list.push_back(str_temp);
              }
            }else if (bcstemperatures->get_bc_keyword_type(steps->bcs_data[sub_data_ids[iii]][2]) == 1) // *TEMPERATURE
            {
              for (size_t iv = 0; iv < bc_attribs.size(); iv++)
              { 
                str_temp = get_nodeset_name(me_iface->id_from_handle(nodeset)) + "," + to_string_scientific(bc_attribs[iv].second);
                steps_export_list.push_back(str_temp);
              }
            }
            // CUSTOMLINE START
            customline = customlines->get_customline_data("AFTER","TEMPERATURE",steps->bcs_data[sub_data_ids[iii]][2]);
            for (size_t icl = 0; icl < customline.size(); icl++)
            {
              steps_export_list.push_back(customline[icl]);
            }
            // CUSTOMLINE END
          }  
        }
      }
      bc_attribs.clear();
    }
    // History Outputs
    sub_data_ids = steps->get_historyoutput_data_ids_from_historyoutputs_id(steps->steps_data[i][7]);

    for (size_t ii = 0; ii < sub_data_ids.size(); ii++)
    {
      str_temp = historyoutputs->get_output_export(steps->historyoutputs_data[sub_data_ids[ii]][1]);
      steps_export_list.push_back(str_temp);
    }
    
    // Field Outputs
    sub_data_ids = steps->get_fieldoutput_data_ids_from_fieldoutputs_id(steps->steps_data[i][8]);

    for (size_t ii = 0; ii < sub_data_ids.size(); ii++)
    {
      str_temp = fieldoutputs->get_output_export(steps->fieldoutputs_data[sub_data_ids[ii]][1]);
      steps_export_list.push_back(str_temp);
    }
    
    // CUSTOMLINE START
    customline = customlines->get_customline_data("BEFORE","STEP_END",steps->steps_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      steps_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END
    str_temp = "*END STEP";
    steps_export_list.push_back(str_temp);
    // CUSTOMLINE START
    customline = customlines->get_customline_data("AFTER","STEP_END",steps->steps_data[i][0]);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      steps_export_list.push_back(customline[icl]);
    }
    // CUSTOMLINE END
  }
  
  if (bc_set_id!=-1)
  {
    log = "Deleting BCSet Steps Export.\n";
    PRINT_INFO("%s", log.c_str());
    command = "delete bcset " + std::to_string(bc_set_id);
    CubitInterface::cmd(command.c_str());
  }

  std::string step_export;

  for (size_t i = 0; i < steps_export_list.size(); i++)
  {
    step_export.append(steps_export_list[i] + "\n");
  }

  // clear export storage data
  sideset_face_data.clear();

  return step_export;
}

std::vector<std::vector<std::string>> CalculiXCore::get_blocks_tree_data()
{ 
  std::vector<std::vector<std::string>> blocks_tree_data;
    
  for (size_t i = 0; i < cb->blocks_data.size(); i++)
  {
    std::vector<std::string> blocks_tree_data_set;
    std::string block_name;

    BlockHandle block;
    me_iface->get_block_handle(cb->blocks_data[i][0], block);
    block_name = me_iface->name_from_handle(block);
    
    if (block_name == "")
    {
      block_name = "Block_" + std::to_string(cb->blocks_data[i][0]);
    }
    
    blocks_tree_data_set.push_back(std::to_string(cb->blocks_data[i][0])); //block_id
    blocks_tree_data_set.push_back(block_name); //block_name
    blocks_tree_data_set.push_back(cb->get_ccx_element_type_name(cb->blocks_data[i][0]));//ccx_element_type
    blocks_tree_data.push_back(blocks_tree_data_set);
  }

  return blocks_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_nodeset_tree_data()
{ 
  std::vector<std::vector<std::string>> nodeset_tree_data;

  // Get the list of nodesets
  std::vector<int> nodesets;
  nodesets = CubitInterface::get_nodeset_id_list();
  
  // loop over the nodesets
  for (size_t i = 0; i < nodesets.size(); i++)
  {
    std::vector<std::string> nodeset_tree_data_set;
    std::string nodeset_name;

    NodesetHandle nodeset;
    me_iface->get_nodeset_handle(nodesets[i],nodeset);
    
    nodeset_name = me_iface->get_nodeset_name(nodeset);
    
    if (nodeset_name == "")
    {
      nodeset_name = "Nodeset_" + std::to_string(me_iface->id_from_handle(nodeset));
    }

    nodeset_tree_data_set.push_back(std::to_string(me_iface->id_from_handle(nodeset))); //nodeset_id
    nodeset_tree_data_set.push_back(nodeset_name); //nodeset_name
    nodeset_tree_data.push_back(nodeset_tree_data_set);
  }

  return nodeset_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_sideset_tree_data()
{ 
  std::vector<std::vector<std::string>> sideset_tree_data;

  // Get the list of sidesets
  std::vector<int> sidesets;
  sidesets = CubitInterface::get_sideset_id_list();
  
  // loop over the sidesets
  for (size_t i = 0; i < sidesets.size(); i++)
  {
    std::vector<std::string> sideset_tree_data_set;
    std::string sideset_name;

    SidesetHandle sideset;
    me_iface->get_sideset_handle(sidesets[i],sideset);
    
    sideset_name = me_iface->get_sideset_name(sideset);
    
    if (sideset_name == "")
    {
      sideset_name = "Sideset_" + std::to_string(me_iface->id_from_handle(sideset));
    }

    sideset_tree_data_set.push_back(std::to_string(me_iface->id_from_handle(sideset))); //sideset_id
    sideset_tree_data_set.push_back(sideset_name); //sideset_name
    sideset_tree_data.push_back(sideset_tree_data_set);
  }

  return sideset_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_material_tree_data()
{ 
  std::vector<std::vector<std::string>> material_tree_data;
  // Get the list of materials
  std::vector<std::string> material_name_list;
  material_name_list = CubitInterface::get_material_name_list();
  //loop over all materials
  for (size_t i = 0; i < material_name_list.size(); i++)
  { 
    MaterialInterface::Material material;
    MaterialInterface::PropertyGroup group;
    std::string group_name;
    std::vector<std::string> group_list;
    material = mat_iface->get_material(material_name_list[i]);
    group = mat_iface->get_material_property_group(material);
    group_name = mat_iface->get_group_name(group);
    group_list = mat->get_group_list();

    if (std::find(group_list.begin(), group_list.end(), group_name) != group_list.end())
    {
      std::vector<std::string> material_tree_data_set;
      material_tree_data_set.push_back(std::to_string(mat_iface->get_material_id(material))); //material_id
      material_tree_data_set.push_back(material_name_list[i]); //material_name
      material_tree_data.push_back(material_tree_data_set);
    }
  }
  return material_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_material_group_properties()
{ 
  return mat->group_properties;
}

std::vector<std::vector<std::string>> CalculiXCore::get_materiallibrary_tree_data()
{ 
  return mat_library->get_materiallibrary_tree_data();
}

std::vector<std::vector<std::string>> CalculiXCore::get_materiallibrary_material_properties(std::string name, std::string group)
{ 
  return mat_library->get_materiallibrary_material_properties(name, group);
}

std::vector<std::vector<double>> CalculiXCore::get_materiallibrary_material_values(std::string name, std::string group, std::string property)
{
  return mat_library->get_materiallibrary_material_values(name, group, property);
}

std::vector<std::vector<std::string>> CalculiXCore::get_sections_tree_data()
{ 
  std::vector<std::vector<std::string>> sections_tree_data;
  
  for (size_t i = 0; i < sections->sections_data.size(); i++)
  {
    std::vector<std::string> sections_tree_data_set;
    std::string section_name;
    std::string block_name;
    std::string material_name;
    int sub_section_data_id;

    if (sections->sections_data[i][1] == 1)
    {
      sub_section_data_id = sections->get_solid_section_data_id_from_solid_section_id(sections->sections_data[i][2]);
      material_name = this->get_material_name(std::stoi(sections->solid_section_data[sub_section_data_id][2]));
      block_name = this->get_block_name(std::stoi(sections->solid_section_data[sub_section_data_id][1]));
      section_name = "SOLID (" + block_name + "|" + material_name + ")";
    } else if (sections->sections_data[i][1] == 2)
    {
      sub_section_data_id = sections->get_shell_section_data_id_from_shell_section_id(sections->sections_data[i][2]);
      material_name = this->get_material_name(std::stoi(sections->shell_section_data[sub_section_data_id][2]));
      block_name = this->get_block_name(std::stoi(sections->shell_section_data[sub_section_data_id][1]));
      section_name = "SHELL (" + block_name + "|" + material_name + ")";
    } else if (sections->sections_data[i][1] == 3)
    {
      sub_section_data_id = sections->get_beam_section_data_id_from_beam_section_id(sections->sections_data[i][2]);
      material_name = this->get_material_name(std::stoi(sections->beam_section_data[sub_section_data_id][2]));
      block_name = this->get_block_name(std::stoi(sections->beam_section_data[sub_section_data_id][1]));
      section_name = "BEAM (" + sections->beam_section_data[sub_section_data_id][3] + "|" + block_name + "|" + material_name + ")";
    } else if (sections->sections_data[i][1] == 4)
    {
      sub_section_data_id = sections->get_membrane_section_data_id_from_membrane_section_id(sections->sections_data[i][2]);
      material_name = this->get_material_name(std::stoi(sections->membrane_section_data[sub_section_data_id][2]));
      block_name = this->get_block_name(std::stoi(sections->membrane_section_data[sub_section_data_id][1]));
      section_name = "MEMBRANE (" + block_name + "|" + material_name + ")";
    }
    
    sections_tree_data_set.push_back(std::to_string(sections->sections_data[i][0])); //section_id
    sections_tree_data_set.push_back(section_name); //section_name
    sections_tree_data.push_back(sections_tree_data_set);
  }

  return sections_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_constraints_tree_data()
{ 
  std::vector<std::vector<std::string>> constraints_tree_data;
  
  for (size_t i = 0; i < constraints->constraints_data.size(); i++)
  {
    std::vector<std::string> constraints_tree_data_set;
    std::string constraint_name;
    std::string entity_name;
    std::string entity_id;
    std::string vertex_ref;
    std::string vertex_rot;

    int sub_constraint_data_id;

    if (constraints->constraints_data[i][1] == 1)
    {
      sub_constraint_data_id = constraints->get_rigidbody_constraint_data_id_from_rigidbody_constraint_id(constraints->constraints_data[i][2]);
      entity_name = "";
      if (constraints->rigidbody_constraint_data[sub_constraint_data_id][1]=="1")
      {
        entity_name = "Nodeset ";
      } else if (constraints->rigidbody_constraint_data[sub_constraint_data_id][1]=="2")
      {
        entity_name = "Block ";
      }
      
      entity_id = constraints->rigidbody_constraint_data[sub_constraint_data_id][2];
      vertex_ref = constraints->rigidbody_constraint_data[sub_constraint_data_id][3];
      vertex_rot = constraints->rigidbody_constraint_data[sub_constraint_data_id][4];
      constraint_name = "Rigid Body ("+ entity_name + entity_id + "| v_ref " + vertex_ref + "| v_rot " + vertex_rot + ")";
    } else if (constraints->constraints_data[i][1] == 2)
    {
      sub_constraint_data_id = constraints->get_tie_constraint_data_id_from_tie_constraint_id(constraints->constraints_data[i][2]);
      
      constraint_name = "TIE (" + constraints->tie_constraint_data[sub_constraint_data_id][1] + ")";
    } else if (constraints->constraints_data[i][1] == 3)
    {
      sub_constraint_data_id = constraints->get_equation_constraint_data_id_from_equation_constraint_id(constraints->constraints_data[i][2]);
      
      constraint_name = "EQUATION (" + constraints->equation_constraint_data[sub_constraint_data_id][1] + ")";
    } else if (constraints->constraints_data[i][1] == 4)
    {
      sub_constraint_data_id = constraints->get_equation_group_constraint_data_id_from_equation_group_constraint_id(constraints->constraints_data[i][2]);
      
      constraint_name = "EQUATION GROUP (" + constraints->equation_group_constraint_data[sub_constraint_data_id][1] + ")";
    }
    
    constraints_tree_data_set.push_back(std::to_string(constraints->constraints_data[i][0])); //constraint_id
    constraints_tree_data_set.push_back(constraint_name); //constraint_name
    constraints_tree_data.push_back(constraints_tree_data_set);
  }

  return constraints_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_surfaceinteractions_tree_data()
{ 
  std::vector<std::vector<std::string>> surfaceinteractions_tree_data;
  
  for (size_t i = 0; i < surfaceinteractions->surfaceinteractions_data.size(); i++)
  {
    std::vector<std::string> surfaceinteractions_tree_data_set;
    std::string surfaceinteraction_name;
    int surfaceinteraction_name_id;

    surfaceinteraction_name_id = surfaceinteractions->get_surfaceinteraction_name_data_id_from_surfaceinteraction_name_id(surfaceinteractions->surfaceinteractions_data[i][1]);
    surfaceinteraction_name = surfaceinteractions->surfaceinteraction_name_data[surfaceinteraction_name_id][1];
    
    surfaceinteractions_tree_data_set.push_back(std::to_string(surfaceinteractions->surfaceinteractions_data[i][0])); //surfaceinteraction_id
    surfaceinteractions_tree_data_set.push_back(surfaceinteraction_name); //surfaceinteraction_name
    surfaceinteractions_tree_data.push_back(surfaceinteractions_tree_data_set);
  }

  return surfaceinteractions_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_contactpairs_tree_data()
{ 
  std::vector<std::vector<std::string>> contactpairs_tree_data;
  
  for (size_t i = 0; i < contactpairs->contactpairs_data.size(); i++)
  {
    std::vector<std::string> contactpairs_tree_data_set;
    std::string contactpair_name;
    
    contactpair_name = "Master: ";
    contactpair_name.append(this->get_sideset_name(contactpairs->contactpairs_data[i][3]));
    contactpair_name.append(" | Slave: ");
    contactpair_name.append(this->get_sideset_name(contactpairs->contactpairs_data[i][4]));

    contactpairs_tree_data_set.push_back(std::to_string(contactpairs->contactpairs_data[i][0])); //contactpair_id
    contactpairs_tree_data_set.push_back(contactpair_name); //contactpair_name
    contactpairs_tree_data.push_back(contactpairs_tree_data_set);
  }

  return contactpairs_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_amplitudes_tree_data()
{ 
  std::vector<std::vector<std::string>> amplitudes_tree_data;
  
  for (size_t i = 0; i < amplitudes->amplitudes_data.size(); i++)
  {
    std::vector<std::string> amplitudes_tree_data_set;
    std::string amplitude_name;
    int amplitude_name_id;

    amplitude_name_id = amplitudes->get_name_amplitude_data_id_from_name_amplitude_id(amplitudes->amplitudes_data[i][1]);
    amplitude_name = amplitudes->name_amplitude_data[amplitude_name_id][1];
    
    amplitudes_tree_data_set.push_back(std::to_string(amplitudes->amplitudes_data[i][0])); //amplitude_id
    amplitudes_tree_data_set.push_back(amplitude_name); //amplitude_name
    amplitudes_tree_data.push_back(amplitudes_tree_data_set);
  }

  return amplitudes_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_orientations_tree_data()
{ 
  std::vector<std::vector<std::string>> orientations_tree_data;
  
  for (size_t i = 0; i < orientations->orientations_data.size(); i++)
  {
    std::vector<std::string> orientations_tree_data_set;
    std::string name;
    int name_data_id;

    name_data_id = orientations->get_name_data_id_from_name_id(orientations->orientations_data[i][1]);
    name = orientations->name_data[name_data_id][1];
    
    orientations_tree_data_set.push_back(std::to_string(orientations->orientations_data[i][0])); //orientation_id
    orientations_tree_data_set.push_back(name); //name
    orientations_tree_data.push_back(orientations_tree_data_set);
  }

  return orientations_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_damping_tree_data()
{ 
  std::vector<std::vector<std::string>> damping_tree_data;
  
  if (damping->damping_data.size()==2)
  {
    std::vector<std::string> damping_tree_data_set;
    std::string name;
    name = "alpha = " + damping->damping_data[0];
    damping_tree_data_set.push_back("1"); //damping_id
    damping_tree_data_set.push_back(name); //name
    damping_tree_data.push_back(damping_tree_data_set);
    damping_tree_data_set.clear();

    name = "beta = " + damping->damping_data[1];
    damping_tree_data_set.push_back("2"); //damping_id
    damping_tree_data_set.push_back(name); //name
    damping_tree_data.push_back(damping_tree_data_set);
  }
  
  return damping_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_physicalconstants_tree_data()
{ 
  std::vector<std::vector<std::string>> physicalconstants_tree_data;
  
  if (physicalconstants->physicalconstants_data.size()==3)
  {
    std::vector<std::string> physicalconstants_tree_data_set;
    std::string name;
    name = "ABSOLUTE ZERO = " + physicalconstants->physicalconstants_data[0];
    physicalconstants_tree_data_set.push_back("1"); //physicalconstants_id
    physicalconstants_tree_data_set.push_back(name); //name
    physicalconstants_tree_data.push_back(physicalconstants_tree_data_set);
    physicalconstants_tree_data_set.clear();

    name = "STEFAN BOLTZMANN = " + physicalconstants->physicalconstants_data[1];
    physicalconstants_tree_data_set.push_back("2"); //physicalconstants_id
    physicalconstants_tree_data_set.push_back(name); //name
    physicalconstants_tree_data.push_back(physicalconstants_tree_data_set);
    physicalconstants_tree_data_set.clear();

    name = "NEWTON GRAVITY = " + physicalconstants->physicalconstants_data[2];
    physicalconstants_tree_data_set.push_back("3"); //physicalconstants_id
    physicalconstants_tree_data_set.push_back(name); //name
    physicalconstants_tree_data.push_back(physicalconstants_tree_data_set);
    physicalconstants_tree_data_set.clear();
  }
  
  return physicalconstants_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsforces_tree_data()
{ 
  std::vector<std::vector<std::string>> loadsforces_tree_data;
  
  for (size_t i = 0; i < loadsforces->loads_data.size(); i++)
  {
    std::vector<std::string> loadsforces_tree_data_set;
    std::string name;
    
    name = CubitInterface::get_bc_name(CI_BCTYPE_FORCE,loadsforces->loads_data[i][0]);
    if (name == "")
    {
      name = "Force_" + std::to_string(loadsforces->loads_data[i][0]);
    }

    loadsforces_tree_data_set.push_back(std::to_string(loadsforces->loads_data[i][0])); //load_id
    loadsforces_tree_data_set.push_back(name); 
    loadsforces_tree_data.push_back(loadsforces_tree_data_set);
  }
  return loadsforces_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadspressures_tree_data()
{ 
  std::vector<std::vector<std::string>> loadspressures_tree_data;
  
  for (size_t i = 0; i < loadspressures->loads_data.size(); i++)
  {
    std::vector<std::string> loadspressures_tree_data_set;
    std::string name;
    
    name = CubitInterface::get_bc_name(CI_BCTYPE_PRESSURE,loadspressures->loads_data[i][0]);
    if (name == "")
    {
      name = "Pressure_" + std::to_string(loadspressures->loads_data[i][0]);
    }
    
    loadspressures_tree_data_set.push_back(std::to_string(loadspressures->loads_data[i][0])); //load_id
    loadspressures_tree_data_set.push_back(name); 
    loadspressures_tree_data.push_back(loadspressures_tree_data_set);
  }
  return loadspressures_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsheatfluxes_tree_data()
{ 
  std::vector<std::vector<std::string>> loadsheatfluxes_tree_data;
  
  for (size_t i = 0; i < loadsheatfluxes->loads_data.size(); i++)
  {
    std::vector<std::string> loadsheatfluxes_tree_data_set;
    std::string name;
    
    name = CubitInterface::get_bc_name(CI_BCTYPE_HEATFLUX,loadsheatfluxes->loads_data[i][0]);
    if (name == "")
    {
      name = "Heatflux_" + std::to_string(loadsheatfluxes->loads_data[i][0]);
    }
    
    loadsheatfluxes_tree_data_set.push_back(std::to_string(loadsheatfluxes->loads_data[i][0])); //load_id
    loadsheatfluxes_tree_data_set.push_back(name); 
    loadsheatfluxes_tree_data.push_back(loadsheatfluxes_tree_data_set);
  }
  return loadsheatfluxes_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsgravity_tree_data()
{ 
  std::vector<std::vector<std::string>> loadsgravity_tree_data;
  
  for (size_t i = 0; i < loadsgravity->loads_data.size(); i++)
  {
    std::vector<std::string> loadsgravity_tree_data_set;
    std::string name;
    
    int subdata_id = loadsgravity->get_name_data_id_from_name_id(loadsgravity->loads_data[i][7]);
    if ((subdata_id!=-1)&&(loadsgravity->name_data[subdata_id][1]!=""))
    {
      name = loadsgravity->name_data[subdata_id][1];
    }else{
      name = "Gravity_" + std::to_string(loadsgravity->loads_data[i][0]);
    }
    
    loadsgravity_tree_data_set.push_back(std::to_string(loadsgravity->loads_data[i][0])); //load_id
    loadsgravity_tree_data_set.push_back(name); 
    loadsgravity_tree_data.push_back(loadsgravity_tree_data_set);
  }
  return loadsgravity_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadscentrifugal_tree_data()
{ 
  std::vector<std::vector<std::string>> loadscentrifugal_tree_data;
  
  for (size_t i = 0; i < loadscentrifugal->loads_data.size(); i++)
  {
    std::vector<std::string> loadscentrifugal_tree_data_set;
    std::string name;
    
    int subdata_id = loadscentrifugal->get_name_data_id_from_name_id(loadscentrifugal->loads_data[i][8]);
    if ((subdata_id!=-1)&&(loadscentrifugal->name_data[subdata_id][1]!=""))
    {
      name = loadscentrifugal->name_data[subdata_id][1];
    }else{
      name = "Centrifugal_" + std::to_string(loadscentrifugal->loads_data[i][0]);
    }
        
    loadscentrifugal_tree_data_set.push_back(std::to_string(loadscentrifugal->loads_data[i][0])); //load_id
    loadscentrifugal_tree_data_set.push_back(name); 
    loadscentrifugal_tree_data.push_back(loadscentrifugal_tree_data_set);
  }
  return loadscentrifugal_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadstrajectory_tree_data()
{ 
  std::vector<std::vector<std::string>> loadstrajectory_tree_data;
  
  for (size_t i = 0; i < loadstrajectory->loads_data.size(); i++)
  {
    std::vector<std::string> loadstrajectory_tree_data_set;
    std::string name;
    
    name = loadstrajectory->get_name_from_load_id(loadstrajectory->loads_data[i][0]);

    if (name=="")
    {
      name = "Trajectory_" + std::to_string(loadstrajectory->loads_data[i][0]);
    }
    
    loadstrajectory_tree_data_set.push_back(std::to_string(loadstrajectory->loads_data[i][0])); //load_id
    loadstrajectory_tree_data_set.push_back(name); 
    loadstrajectory_tree_data.push_back(loadstrajectory_tree_data_set);
  }
  return loadstrajectory_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsfilm_tree_data()
{ 
  std::vector<std::vector<std::string>> loadsfilm_tree_data;
  
  for (size_t i = 0; i < loadsfilm->loads_data.size(); i++)
  {
    std::vector<std::string> loadsfilm_tree_data_set;
    std::string name;
    
    int subdata_id = loadsfilm->get_name_data_id_from_name_id(loadsfilm->loads_data[i][9]);
    if ((subdata_id!=-1)&&(loadsfilm->name_data[subdata_id][1]!=""))
    {
      name = loadsfilm->name_data[subdata_id][1];
    }else{
      name = "Film_" + std::to_string(loadsfilm->loads_data[i][0]);
    }
    
    loadsfilm_tree_data_set.push_back(std::to_string(loadsfilm->loads_data[i][0])); //load_id
    loadsfilm_tree_data_set.push_back(name); 
    loadsfilm_tree_data.push_back(loadsfilm_tree_data_set);
  }
  return loadsfilm_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsradiation_tree_data()
{ 
  std::vector<std::vector<std::string>> loadsradiation_tree_data;
  
  for (size_t i = 0; i < loadsradiation->loads_data.size(); i++)
  {
    std::vector<std::string> loadsradiation_tree_data_set;
    std::string name;
    
    int subdata_id = loadsradiation->get_name_data_id_from_name_id(loadsradiation->loads_data[i][9]);
    if ((subdata_id!=-1)&&(loadsradiation->name_data[subdata_id][1]!=""))
    {
      name = loadsradiation->name_data[subdata_id][1];
    }else{
      name = "Radiation_" + std::to_string(loadsradiation->loads_data[i][0]);
    }
    
    loadsradiation_tree_data_set.push_back(std::to_string(loadsradiation->loads_data[i][0])); //load_id
    loadsradiation_tree_data_set.push_back(name); 
    loadsradiation_tree_data.push_back(loadsradiation_tree_data_set);
  }
  return loadsradiation_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadssurfacetraction_tree_data()
{ 
  std::vector<std::vector<std::string>> loadssurfacetraction_tree_data;
  
  for (size_t i = 0; i < loadssurfacetraction->loads_data.size(); i++)
  {
    std::vector<std::string> loadssurfacetraction_tree_data_set;
    std::string name;
    
    int subdata_id = loadssurfacetraction->get_name_data_id_from_name_id(loadssurfacetraction->loads_data[i][6]);
    if ((subdata_id!=-1)&&(loadssurfacetraction->name_data[subdata_id][1]!=""))
    {
      name = loadssurfacetraction->name_data[subdata_id][1];
    }else{
      name = "SurfaceTraction_" + std::to_string(loadssurfacetraction->loads_data[i][0]);
    }
    
    loadssurfacetraction_tree_data_set.push_back(std::to_string(loadssurfacetraction->loads_data[i][0])); //load_id
    loadssurfacetraction_tree_data_set.push_back(name); 
    loadssurfacetraction_tree_data.push_back(loadssurfacetraction_tree_data_set);
  }
  return loadssurfacetraction_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_loadsbodyheatflux_tree_data()
{ 
  std::vector<std::vector<std::string>> loadsbodyheatflux_tree_data;
  
  for (size_t i = 0; i < loadsbodyheatflux->loads_data.size(); i++)
  {
    std::vector<std::string> loadsbodyheatflux_tree_data_set;
    std::string name;
    
    int subdata_id = loadsbodyheatflux->get_name_data_id_from_name_id(loadsbodyheatflux->loads_data[i][6]);
    if ((subdata_id!=-1)&&(loadsbodyheatflux->name_data[subdata_id][1]!=""))
    {
      name = loadsbodyheatflux->name_data[subdata_id][1];
    }else{
      name = "BodyHeatflux_" + std::to_string(loadsbodyheatflux->loads_data[i][0]);
    }
    
    loadsbodyheatflux_tree_data_set.push_back(std::to_string(loadsbodyheatflux->loads_data[i][0])); //load_id
    loadsbodyheatflux_tree_data_set.push_back(name); 
    loadsbodyheatflux_tree_data.push_back(loadsbodyheatflux_tree_data_set);
  }
  return loadsbodyheatflux_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_bcsdisplacements_tree_data()
{ 
  std::vector<std::vector<std::string>> bcsdisplacements_tree_data;
  
  for (size_t i = 0; i < bcsdisplacements->bcs_data.size(); i++)
  {
    std::vector<std::string> bcsdisplacements_tree_data_set;
    std::string name;
    
    name = CubitInterface::get_bc_name(CI_BCTYPE_DISPLACEMENT,bcsdisplacements->bcs_data[i][0]);
    if (name == "")
    {
      name = "Displacement_" + std::to_string(bcsdisplacements->bcs_data[i][0]);
    }
    
    bcsdisplacements_tree_data_set.push_back(std::to_string(bcsdisplacements->bcs_data[i][0])); //bc_id
    bcsdisplacements_tree_data_set.push_back(name); 
    bcsdisplacements_tree_data.push_back(bcsdisplacements_tree_data_set);
  }
  return bcsdisplacements_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_bcstemperatures_tree_data()
{ 
  std::vector<std::vector<std::string>> bcstemperatures_tree_data;
  
  for (size_t i = 0; i < bcstemperatures->bcs_data.size(); i++)
  {
    std::vector<std::string> bcstemperatures_tree_data_set;
    std::string name;
    
    name = CubitInterface::get_bc_name(CI_BCTYPE_TEMPERATURE,bcstemperatures->bcs_data[i][0]);
    if (name == "")
    {
      name = "Temperature_" + std::to_string(bcstemperatures->bcs_data[i][0]);
    }
    
    bcstemperatures_tree_data_set.push_back(std::to_string(bcstemperatures->bcs_data[i][0])); //bc_id
    bcstemperatures_tree_data_set.push_back(name); 
    bcstemperatures_tree_data.push_back(bcstemperatures_tree_data_set);
  }
  return bcstemperatures_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_historyoutputs_tree_data()
{ 
  std::vector<std::vector<std::string>> outputs_tree_data;
  
  for (size_t i = 0; i < historyoutputs->outputs_data.size(); i++)
  {
    std::vector<std::string> outputs_tree_data_set;
    std::string output_name;
    int output_name_id;

    output_name_id = historyoutputs->get_name_data_id_from_name_id(historyoutputs->outputs_data[i][1]);
    output_name = historyoutputs->name_data[output_name_id][1];
    
    if (historyoutputs->outputs_data[i][2]==1)
    {
      output_name = output_name + " (node)";
    }else if (historyoutputs->outputs_data[i][2]==2)
    {
      output_name = output_name + " (element)";
    }else if (historyoutputs->outputs_data[i][2]==3)
    {
      output_name = output_name + " (contact)";
    }else if (historyoutputs->outputs_data[i][2]==4)
    {
      output_name = output_name + " (section)";
    }

    outputs_tree_data_set.push_back(std::to_string(historyoutputs->outputs_data[i][0])); //output_id
    outputs_tree_data_set.push_back(output_name); //output_name
    outputs_tree_data.push_back(outputs_tree_data_set);
  }

  return outputs_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_fieldoutputs_tree_data()
{ 
  std::vector<std::vector<std::string>> outputs_tree_data;
  
  for (size_t i = 0; i < fieldoutputs->outputs_data.size(); i++)
  {
    std::vector<std::string> outputs_tree_data_set;
    std::string output_name;
    int output_name_id;

    output_name_id = fieldoutputs->get_name_data_id_from_name_id(fieldoutputs->outputs_data[i][1]);
    output_name = fieldoutputs->name_data[output_name_id][1];
    
    if (fieldoutputs->outputs_data[i][2]==1)
    {
      output_name = output_name + " (node)";
    }else if (fieldoutputs->outputs_data[i][2]==2)
    {
      output_name = output_name + " (element)";
    }else if (fieldoutputs->outputs_data[i][2]==3)
    {
      output_name = output_name + " (contact)";
    }

    outputs_tree_data_set.push_back(std::to_string(fieldoutputs->outputs_data[i][0])); //output_id
    outputs_tree_data_set.push_back(output_name); //output_name
    outputs_tree_data.push_back(outputs_tree_data_set);
  }

  return outputs_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_initialconditions_tree_data()
{ 
  std::vector<std::vector<std::string>> initialconditions_tree_data;
  
  for (size_t i = 0; i < initialconditions->initialconditions_data.size(); i++)
  {
    std::vector<std::string> initialconditions_tree_data_set;
    std::string initialcondition_name;
    int sub_id;
    std::string bc_id;

    if (initialconditions->initialconditions_data[i][1]==1)
    {
      sub_id = initialconditions->get_displacement_data_id_from_displacement_id(initialconditions->initialconditions_data[i][2]);
      bc_id = initialconditions->displacement_data[sub_id][1];
      if (bc_id == "")
      {
        initialcondition_name = "Displacement ID not set!";
      } else {
        if (check_bc_exists(std::stoi(bc_id),4))
        {
          initialcondition_name = CubitInterface::get_bc_name(CI_BCTYPE_DISPLACEMENT,std::stoi(bc_id));
        } else 
        {
          initialcondition_name = "Displacement ID " + bc_id + " doesn't exist!";
        }
      }
      if (initialcondition_name == "")
      {
        initialcondition_name = "Displacement_" + initialconditions->displacement_data[sub_id][1];
      }
      initialcondition_name = initialcondition_name + " (Displacement)";
    }else if (initialconditions->initialconditions_data[i][1]==2)
    { 
      sub_id = initialconditions->get_temperature_data_id_from_temperature_id(initialconditions->initialconditions_data[i][2]);
      bc_id = initialconditions->temperature_data[sub_id][1];
      if (bc_id == "")
      {
        initialcondition_name = "Temperature ID not set";
      }else {
        if (check_bc_exists(std::stoi(bc_id),5))
        {
          initialcondition_name = CubitInterface::get_bc_name(CI_BCTYPE_TEMPERATURE,std::stoi(bc_id));
        } else 
        {
          initialcondition_name = "Temperature ID " + bc_id + " doesn't exist!";
        }
      }
      if (initialcondition_name == "")
      {
        initialcondition_name = "Temperature_" + initialconditions->temperature_data[sub_id][1];
      }
      initialcondition_name = initialcondition_name + " (Temperature)";
      
    }else if (initialconditions->initialconditions_data[i][1]==3)
    { 
      sub_id = initialconditions->initialconditions_data[i][2];
      
      initialcondition_name = "Stress_" + std::to_string(sub_id);
    }

    initialconditions_tree_data_set.push_back(std::to_string(initialconditions->initialconditions_data[i][0])); //initialcondition_id
    initialconditions_tree_data_set.push_back(initialcondition_name); //initialcondition_name
    initialconditions_tree_data.push_back(initialconditions_tree_data_set);
  }

  return initialconditions_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_hbcsdisplacements_tree_data()
{ 
  std::vector<std::vector<std::string>> bcsdisplacements_tree_data;
  std::vector<int> bcs_ids;
  
  bcs_ids = hbcs->get_bc_data_ids_from_bcs_id(0);

  for (size_t i = 0; i < bcs_ids.size(); i++)
  {
    std::vector<std::string> bcsdisplacements_tree_data_set;
    std::string name;
    if (hbcs->bcs_data[bcs_ids[i]][1]==1)
    {
      name = CubitInterface::get_bc_name(CI_BCTYPE_DISPLACEMENT,hbcs->bcs_data[bcs_ids[i]][2]);
      if (name == "")
      {
        name = "Displacement_" + std::to_string(hbcs->bcs_data[bcs_ids[i]][2]);
      }
      bcsdisplacements_tree_data_set.push_back(std::to_string(hbcs->bcs_data[bcs_ids[i]][2])); //bc_id
      bcsdisplacements_tree_data_set.push_back(name); 
      bcsdisplacements_tree_data.push_back(bcsdisplacements_tree_data_set);  
    }
  }
  return bcsdisplacements_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_hbcstemperatures_tree_data()
{ 
  std::vector<std::vector<std::string>> bcstemperatures_tree_data;
  std::vector<int> bcs_ids;
  
  bcs_ids = hbcs->get_bc_data_ids_from_bcs_id(0);

  for (size_t i = 0; i < bcs_ids.size(); i++)
  {
    std::vector<std::string> bcstemperatures_tree_data_set;
    std::string name;
    if (hbcs->bcs_data[bcs_ids[i]][1]==2)
    {
      name = CubitInterface::get_bc_name(CI_BCTYPE_TEMPERATURE,hbcs->bcs_data[bcs_ids[i]][2]);
      if (name == "")
      {
        name = "Temperature_" + std::to_string(hbcs->bcs_data[bcs_ids[i]][2]);
      }
      bcstemperatures_tree_data_set.push_back(std::to_string(hbcs->bcs_data[bcs_ids[i]][2])); //bc_id
      bcstemperatures_tree_data_set.push_back(name); 
      bcstemperatures_tree_data.push_back(bcstemperatures_tree_data_set);  
    }
  }
  return bcstemperatures_tree_data;
}

std::vector<int> CalculiXCore::get_steps_ids()
{
  std::vector<int> steps_ids;
  for (size_t i = 0; i < steps->steps_data.size(); i++)
  {
    steps_ids.push_back(steps->steps_data[i][0]);
  }
  return steps_ids;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_tree_data()
{ 
  std::vector<std::vector<std::string>> steps_tree_data;
  
  for (size_t i = 0; i < steps->steps_data.size(); i++)
  {
    std::vector<std::string> steps_tree_data_set;
    std::string step_name;
    int step_name_id;

    step_name_id = steps->get_name_data_id_from_name_id(steps->steps_data[i][1]);
    step_name = steps->name_data[step_name_id][1];
    
    if (steps->steps_data[i][3]==1)
    {
      step_name = step_name + " (NO ANALYSIS)";
    }else if (steps->steps_data[i][3]==2)
    {
      step_name = step_name + " (STATIC)";
    }else if (steps->steps_data[i][3]==3)
    {
      step_name = step_name + " (FREQUENCY)";
    }else if (steps->steps_data[i][3]==4)
    {
      step_name = step_name + " (BUCKLE)";
    }else if (steps->steps_data[i][3]==5)
    {
      step_name = step_name + " (HEAT TRANSFER)";
    }else if (steps->steps_data[i][3]==6)
    {
      step_name = step_name + " (COUPLED TEMPERATURE-DISPLACEMENT)";
    }else if (steps->steps_data[i][3]==7)
    {
      step_name = step_name + " (UNCOUPLED TEMPERATURE-DISPLACEMENT)";
    }else if (steps->steps_data[i][3]==8)
    {
      step_name = step_name + " (DYNAMIC)";
    }else if (steps->steps_data[i][3]==9)
    {
      step_name = step_name + " (MODAL DYNAMIC)";
    }else if (steps->steps_data[i][3]==10)
    {
      step_name = step_name + " (STEADY STATE DYNAMICS)";
    }else if (steps->steps_data[i][3]==11)
    {
      step_name = step_name + " (COMPLEX DYNAMIC)";
    }

    steps_tree_data_set.push_back(std::to_string(steps->steps_data[i][0])); //step_id
    steps_tree_data_set.push_back(step_name); //step_name
    steps_tree_data.push_back(steps_tree_data_set);
  }

  return steps_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsforces_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadsforces_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadsforces_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadsforces_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==1)
    {
      name = CubitInterface::get_bc_name(CI_BCTYPE_FORCE,steps->loads_data[loads_ids[i]][2]);
      if (name == "")
      {
        name = "Force_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
      loadsforces_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadsforces_tree_data_set.push_back(name); 
      loadsforces_tree_data.push_back(loadsforces_tree_data_set);  
    }
  }
  return loadsforces_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadspressures_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadspressures_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadspressures_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadspressures_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==2)
    {
      name = CubitInterface::get_bc_name(CI_BCTYPE_PRESSURE,steps->loads_data[loads_ids[i]][2]);
      if (name == "")
      {
        name = "Pressure_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
      loadspressures_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadspressures_tree_data_set.push_back(name); 
      loadspressures_tree_data.push_back(loadspressures_tree_data_set);  
    }
  }
  return loadspressures_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsheatfluxes_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadsheatfluxes_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadsheatfluxes_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadsheatfluxes_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==3)
    {
      name = CubitInterface::get_bc_name(CI_BCTYPE_HEATFLUX,steps->loads_data[loads_ids[i]][2]);
      if (name == "")
      {
        name = "Heatflux_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
      loadsheatfluxes_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadsheatfluxes_tree_data_set.push_back(name); 
      loadsheatfluxes_tree_data.push_back(loadsheatfluxes_tree_data_set);  
    }
  }
  return loadsheatfluxes_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsgravity_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadsgravity_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadsgravity_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadsgravity_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==4)
    { 
      int loaddata_id = loadsgravity->get_loads_data_id_from_load_id(steps->loads_data[loads_ids[i]][2]);
      if (loaddata_id!=-1)
      {
        int subdata_id = loadsgravity->get_name_data_id_from_name_id(loadsgravity->loads_data[loaddata_id][7]);
        if ((subdata_id!=-1)&&(loadsgravity->name_data[subdata_id][1]!=""))
        {
          name = loadsgravity->name_data[subdata_id][1];
        }else{
          name = "Gravity_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
        }
      }else{
        name = "Gravity_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
    
      loadsgravity_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadsgravity_tree_data_set.push_back(name); 
      loadsgravity_tree_data.push_back(loadsgravity_tree_data_set);  
    }
  }
  return loadsgravity_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadscentrifugal_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadscentrifugal_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadscentrifugal_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadscentrifugal_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==5)
    { 
      int loaddata_id = loadscentrifugal->get_loads_data_id_from_load_id(steps->loads_data[loads_ids[i]][2]);
      if (loaddata_id!=-1)
      {
        int subdata_id = loadscentrifugal->get_name_data_id_from_name_id(loadscentrifugal->loads_data[loaddata_id][8]);
        if ((subdata_id!=-1)&&(loadscentrifugal->name_data[subdata_id][1]!=""))
        {
          name = loadscentrifugal->name_data[subdata_id][1];
        }else{
          name = "Centrifugal_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
        }
      }else{
        name = "Centrifugal_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
    
      loadscentrifugal_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadscentrifugal_tree_data_set.push_back(name); 
      loadscentrifugal_tree_data.push_back(loadscentrifugal_tree_data_set);  
    }
  }
  return loadscentrifugal_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadstrajectory_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadstrajectory_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadstrajectory_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadstrajectory_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==6)
    { 
      int loaddata_id = loadstrajectory->get_loads_data_id_from_load_id(steps->loads_data[loads_ids[i]][2]);
      if (loaddata_id!=-1)
      {
        name = loadstrajectory->get_name_from_load_id(steps->loads_data[loads_ids[i]][2]);
        if (name=="")
        {
          name = "Trajectory_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
        }
      }else{
        name = "Trajectory_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
    
      loadstrajectory_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadstrajectory_tree_data_set.push_back(name); 
      loadstrajectory_tree_data.push_back(loadstrajectory_tree_data_set);  
    }
  }
  return loadstrajectory_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsfilm_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadsfilm_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadsfilm_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadsfilm_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==7)
    { 
      int loaddata_id = loadsfilm->get_loads_data_id_from_load_id(steps->loads_data[loads_ids[i]][2]);
      if (loaddata_id!=-1)
      {
        int subdata_id = loadsfilm->get_name_data_id_from_name_id(loadsfilm->loads_data[loaddata_id][9]);
        if ((subdata_id!=-1)&&(loadsfilm->name_data[subdata_id][1]!=""))
        {
          name = loadsfilm->name_data[subdata_id][1];
        }else{
          name = "Film_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
        }
      }else{
        name = "Film_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
    
      loadsfilm_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadsfilm_tree_data_set.push_back(name); 
      loadsfilm_tree_data.push_back(loadsfilm_tree_data_set);  
    }
  }
  return loadsfilm_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsradiation_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadsradiation_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadsradiation_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadsradiation_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==8)
    { 
      int loaddata_id = loadsradiation->get_loads_data_id_from_load_id(steps->loads_data[loads_ids[i]][2]);
      if (loaddata_id!=-1)
      {
        int subdata_id = loadsradiation->get_name_data_id_from_name_id(loadsradiation->loads_data[loaddata_id][9]);
        if ((subdata_id!=-1)&&(loadsradiation->name_data[subdata_id][1]!=""))
        {
          name = loadsradiation->name_data[subdata_id][1];
        }else{
          name = "Radiation_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
        }
      }else{
        name = "Radiation_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
    
      loadsradiation_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadsradiation_tree_data_set.push_back(name); 
      loadsradiation_tree_data.push_back(loadsradiation_tree_data_set);  
    }
  }
  return loadsradiation_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadssurfacetraction_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadssurfacetraction_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadssurfacetraction_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadssurfacetraction_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==9)
    { 
      int loaddata_id = loadssurfacetraction->get_loads_data_id_from_load_id(steps->loads_data[loads_ids[i]][2]);
      if (loaddata_id!=-1)
      {
        int subdata_id = loadssurfacetraction->get_name_data_id_from_name_id(loadssurfacetraction->loads_data[loaddata_id][6]);
        if ((subdata_id!=-1)&&(loadssurfacetraction->name_data[subdata_id][1]!=""))
        {
          name = loadssurfacetraction->name_data[subdata_id][1];
        }else{
          name = "SurfaceTraction_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
        }
      }else{
        name = "SurfaceTraction_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
    
      loadssurfacetraction_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadssurfacetraction_tree_data_set.push_back(name); 
      loadssurfacetraction_tree_data.push_back(loadssurfacetraction_tree_data_set);  
    }
  }
  return loadssurfacetraction_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsbodyheatflux_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> loadsbodyheatflux_tree_data;
  int step_data_id;
  std::vector<int> loads_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return loadsbodyheatflux_tree_data;
  }
  loads_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[step_data_id][5]);

  for (size_t i = 0; i < loads_ids.size(); i++)
  {
    std::vector<std::string> loadsbodyheatflux_tree_data_set;
    std::string name;
    if (steps->loads_data[loads_ids[i]][1]==10)
    { 
      int loaddata_id = loadsbodyheatflux->get_loads_data_id_from_load_id(steps->loads_data[loads_ids[i]][2]);
      if (loaddata_id!=-1)
      {
        int subdata_id = loadsbodyheatflux->get_name_data_id_from_name_id(loadsbodyheatflux->loads_data[loaddata_id][6]);
        if ((subdata_id!=-1)&&(loadsbodyheatflux->name_data[subdata_id][1]!=""))
        {
          name = loadsbodyheatflux->name_data[subdata_id][1];
        }else{
          name = "BodyHeatflux_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
        }
      }else{
        name = "BodyHeatflux_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
      }
    
      loadsbodyheatflux_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadsbodyheatflux_tree_data_set.push_back(name); 
      loadsbodyheatflux_tree_data.push_back(loadsbodyheatflux_tree_data_set);  
    }
  }
  return loadsbodyheatflux_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_bcsdisplacements_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> bcsdisplacements_tree_data;
  int step_data_id;
  std::vector<int> bcs_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return bcsdisplacements_tree_data;
  }
  bcs_ids = steps->get_bc_data_ids_from_bcs_id(steps->steps_data[step_data_id][6]);

  for (size_t i = 0; i < bcs_ids.size(); i++)
  {
    std::vector<std::string> bcsdisplacements_tree_data_set;
    std::string name;
    if (steps->bcs_data[bcs_ids[i]][1]==1)
    {
      name = CubitInterface::get_bc_name(CI_BCTYPE_DISPLACEMENT,steps->bcs_data[bcs_ids[i]][2]);
      if (name == "")
      {
        name = "Displacement_" + std::to_string(steps->bcs_data[bcs_ids[i]][2]);
      }
      bcsdisplacements_tree_data_set.push_back(std::to_string(steps->bcs_data[bcs_ids[i]][2])); //bc_id
      bcsdisplacements_tree_data_set.push_back(name); 
      bcsdisplacements_tree_data.push_back(bcsdisplacements_tree_data_set);  
    }
  }
  return bcsdisplacements_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_bcstemperatures_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> bcstemperatures_tree_data;
  int step_data_id;
  std::vector<int> bcs_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return bcstemperatures_tree_data;
  }
  bcs_ids = steps->get_bc_data_ids_from_bcs_id(steps->steps_data[step_data_id][6]);

  for (size_t i = 0; i < bcs_ids.size(); i++)
  {
    std::vector<std::string> bcstemperatures_tree_data_set;
    std::string name;
    if (steps->bcs_data[bcs_ids[i]][1]==2)
    {
      name = CubitInterface::get_bc_name(CI_BCTYPE_TEMPERATURE,steps->bcs_data[bcs_ids[i]][2]);
      if (name == "")
      {
        name = "Temperature_" + std::to_string(steps->bcs_data[bcs_ids[i]][2]);
      }
      bcstemperatures_tree_data_set.push_back(std::to_string(steps->bcs_data[bcs_ids[i]][2])); //bc_id
      bcstemperatures_tree_data_set.push_back(name); 
      bcstemperatures_tree_data.push_back(bcstemperatures_tree_data_set);  
    }
  }
  return bcstemperatures_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_historyoutputs_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> outputs_tree_data;
  int step_data_id;
  std::vector<int> historyoutputs_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return outputs_tree_data;
  }
  historyoutputs_ids = steps->get_historyoutput_data_ids_from_historyoutputs_id(steps->steps_data[step_data_id][7]);
  
  for (size_t i = 0; i < historyoutputs_ids.size(); i++)
  {
    int output_id;
    int output_data_id;
    std::vector<std::string> outputs_tree_data_set;
    std::string output_name;
    int output_name_id;
    output_id = steps->historyoutputs_data[historyoutputs_ids[i]][1];
    output_data_id = historyoutputs->get_outputs_data_id_from_output_id(output_id);

    output_name_id = historyoutputs->get_name_data_id_from_name_id(historyoutputs->outputs_data[output_data_id][1]);
    output_name = historyoutputs->name_data[output_name_id][1];
    
    if (historyoutputs->outputs_data[output_data_id][2]==1)
    {
      output_name = output_name + " (node)";
    }else if (historyoutputs->outputs_data[output_data_id][2]==2)
    {
      output_name = output_name + " (element)";
    }else if (historyoutputs->outputs_data[output_data_id][2]==3)
    {
      output_name = output_name + " (contact)";
    }else if (historyoutputs->outputs_data[output_data_id][2]==4)
    {
      output_name = output_name + " (section)";
    }

    outputs_tree_data_set.push_back(std::to_string(historyoutputs->outputs_data[output_data_id][0])); //output_id
    outputs_tree_data_set.push_back(output_name); //output_name
    outputs_tree_data.push_back(outputs_tree_data_set);
  }

  return outputs_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_fieldoutputs_tree_data(int step_id)
{ 
  std::vector<std::vector<std::string>> outputs_tree_data;
  int step_data_id;
  std::vector<int> fieldoutputs_ids;
  step_data_id = steps->get_steps_data_id_from_step_id(step_id);
  if (step_data_id==-1)
  {
    return outputs_tree_data;
  }
  fieldoutputs_ids = steps->get_fieldoutput_data_ids_from_fieldoutputs_id(steps->steps_data[step_data_id][8]);
  
  for (size_t i = 0; i < fieldoutputs_ids.size(); i++)
  {
    int output_id;
    int output_data_id;
    std::vector<std::string> outputs_tree_data_set;
    std::string output_name;
    int output_name_id;
    output_id = steps->fieldoutputs_data[fieldoutputs_ids[i]][1];
    output_data_id = fieldoutputs->get_outputs_data_id_from_output_id(output_id);

    output_name_id = fieldoutputs->get_name_data_id_from_name_id(fieldoutputs->outputs_data[output_data_id][1]);
    output_name = fieldoutputs->name_data[output_name_id][1];
    
    if (fieldoutputs->outputs_data[output_data_id][2]==1)
    {
      output_name = output_name + " (node)";
    }else if (fieldoutputs->outputs_data[output_data_id][2]==2)
    {
      output_name = output_name + " (element)";
    }else if (fieldoutputs->outputs_data[output_data_id][2]==3)
    {
      output_name = output_name + " (contact)";
    }

    outputs_tree_data_set.push_back(std::to_string(fieldoutputs->outputs_data[output_data_id][0])); //output_id
    outputs_tree_data_set.push_back(output_name); //output_name
    outputs_tree_data.push_back(outputs_tree_data_set);
  }

  return outputs_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_customlines_tree_data()
{ 
  std::vector<std::vector<std::string>> customlines_tree_data;
  
  for (size_t i = 0; i < customlines->customlines_data.size(); i++)
  {
    std::vector<std::string> customlines_tree_data_set;
    
    customlines_tree_data_set.push_back(customlines->customlines_data[i][0]); //customline_id
    customlines_tree_data_set.push_back(customlines->customlines_data[i][1] + " (" + customlines->customlines_data[i][2] + " " + customlines->customlines_data[i][3] + " " + customlines->customlines_data[i][4] + ")"); //customline_name
    customlines_tree_data.push_back(customlines_tree_data_set);
  }

  return customlines_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_jobs_tree_data()
{ 
  std::vector<std::vector<std::string>> jobs_tree_data;
  
  for (size_t i = 0; i < jobs->jobs_data.size(); i++)
  {
    std::vector<std::string> jobs_tree_data_set;
    
    jobs_tree_data_set.push_back(jobs->jobs_data[i][0]); //job_id
    jobs_tree_data_set.push_back(jobs->jobs_data[i][1]); //job_name
    jobs_tree_data.push_back(jobs_tree_data_set);
  }

  return jobs_tree_data;
}

std::vector<std::vector<std::string>> CalculiXCore::get_topology_tree_data()
{
  if (!topology)
  {
    return {};
  }
  return topology->get_tree_data();
}

std::vector<int> CalculiXCore::parser(std::string parse_type, std::string parse_string)
{
  std::vector<int> input_ids;
  
  std::vector<int> all_ids;
  std::vector<int> except_ids;
  std::vector<int> to_ids;

  size_t pos_all = parse_string.find("all");
  size_t pos_except = parse_string.find("except");
  size_t pos_to = parse_string.find("to");

  input_ids = this->extractIntegers(parse_string);

  if (pos_all != std::string::npos)
  {
    if (parse_type=="section")
    {
      for (size_t i = 0; i < sections->sections_data.size(); i++)
      {
        all_ids.push_back(sections->sections_data[i][0]);
      }
    } else if (parse_type=="constraint")
    {
      for (size_t i = 0; i < constraints->constraints_data.size(); i++)
      {
        all_ids.push_back(constraints->constraints_data[i][0]);
      }
    } else if (parse_type=="surfaceinteraction")
    {
      for (size_t i = 0; i < surfaceinteractions->surfaceinteractions_data.size(); i++)
      {
        all_ids.push_back(surfaceinteractions->surfaceinteractions_data[i][0]);
      }
    } else if (parse_type=="contactpair")
    {
      for (size_t i = 0; i < contactpairs->contactpairs_data.size(); i++)
      {
        all_ids.push_back(contactpairs->contactpairs_data[i][0]);
      }
    } else if (parse_type=="amplitude")
    {
      for (size_t i = 0; i < amplitudes->amplitudes_data.size(); i++)
      {
        all_ids.push_back(amplitudes->amplitudes_data[i][0]);
      }
    } else if (parse_type=="orientation")
    {
      for (size_t i = 0; i < orientations->orientations_data.size(); i++)
      {
        all_ids.push_back(orientations->orientations_data[i][0]);
      }
    } else if (parse_type=="loadsgravity")
    {
      for (size_t i = 0; i < loadsgravity->loads_data.size(); i++)
      {
        all_ids.push_back(loadsgravity->loads_data[i][0]);
      }
    } else if (parse_type=="loadscentrifugal")
    {
      for (size_t i = 0; i < loadscentrifugal->loads_data.size(); i++)
      {
        all_ids.push_back(loadscentrifugal->loads_data[i][0]);
      }
    } else if (parse_type=="loadstrajectory")
    {
      for (size_t i = 0; i < loadstrajectory->loads_data.size(); i++)
      {
        all_ids.push_back(loadstrajectory->loads_data[i][0]);
      }
    } else if (parse_type=="loadsfilm")
    {
      for (size_t i = 0; i < loadsfilm->loads_data.size(); i++)
      {
        all_ids.push_back(loadsfilm->loads_data[i][0]);
      }
    } else if (parse_type=="loadsradiation")
    {
      for (size_t i = 0; i < loadsradiation->loads_data.size(); i++)
      {
        all_ids.push_back(loadsradiation->loads_data[i][0]);
      }
    } else if (parse_type=="loadssurfacetraction")
    {
      for (size_t i = 0; i < loadssurfacetraction->loads_data.size(); i++)
      {
        all_ids.push_back(loadssurfacetraction->loads_data[i][0]);
      }
    } else if (parse_type=="loadsbodyheatflux")
    {
      for (size_t i = 0; i < loadsbodyheatflux->loads_data.size(); i++)
      {
        all_ids.push_back(loadsbodyheatflux->loads_data[i][0]);
      }
    } else if (parse_type=="historyoutput")
    {
      for (size_t i = 0; i < historyoutputs->outputs_data.size(); i++)
      {
        all_ids.push_back(historyoutputs->outputs_data[i][0]);
      }
    } else if (parse_type=="fieldoutput")
    {
      for (size_t i = 0; i < fieldoutputs->outputs_data.size(); i++)
      {
        all_ids.push_back(fieldoutputs->outputs_data[i][0]);
      }
    } else if (parse_type=="initialcondition")
    {
      for (size_t i = 0; i < initialconditions->initialconditions_data.size(); i++)
      {
        all_ids.push_back(initialconditions->initialconditions_data[i][0]);
      }
    } else if (parse_type=="step")
    {
      for (size_t i = 0; i < steps->steps_data.size(); i++)
      {
        all_ids.push_back(steps->steps_data[i][0]);
      }
    } else if (parse_type=="job")
    {
      for (size_t i = 0; i < jobs->jobs_data.size(); i++)
      {
        all_ids.push_back(std::stoi(jobs->jobs_data[i][0]));
      }
    } else if (parse_type=="customline")
    {
      for (size_t i = 0; i < customlines->customlines_data.size(); i++)
      {
        all_ids.push_back(std::stoi(customlines->customlines_data[i][0]));
      }
    }
  }

  if (pos_to != std::string::npos)
  { 
    if ((input_ids[0]<=input_ids[1])&&(input_ids[0]>0))
    {  
      for (size_t i = input_ids[0]; i < input_ids[1]+1; i++)
      {
        to_ids.push_back(int(i));
      }
    }
  }

  if ((pos_except != std::string::npos) && (pos_to == std::string::npos))
  { 
    except_ids = input_ids;
  } else if ((pos_except != std::string::npos) && (pos_to != std::string::npos))
  {
    except_ids = to_ids;
  }

  if ((pos_all == std::string::npos) && (pos_to != std::string::npos))
  {
    return to_ids;  
  }

  for (size_t i = 0; i < except_ids.size(); i++)
  {
    all_ids.erase(std::remove(all_ids.begin(), all_ids.end(), except_ids[i]), all_ids.end());
  }
  
  return all_ids;
}

std::vector<int> CalculiXCore::extractIntegers(std::string str)
{
    std::stringstream ss;
    std::vector<int> ids;
 
    /* Storing the whole string into string stream */
    ss << str;
 
    /* Running loop till the end of the stream */
    std::string temp;
    int found;
    while (!ss.eof()) {
        /* extracting word by word from stream */
        ss >> temp;
 
        /* Checking the given word is integer or not */
        if (std::stringstream(temp) >> found){
            ids.push_back(found);
        }
        /* To save from space at the end of string */
        temp = "";
    }
    return ids;
}

bool CalculiXCore::is_checking_jobs() const
{
  return checking_jobs;
}

bool CalculiXCore::set_topology_run_state(int topology_id, TopologyRunState state)
{
  if (!topology)
  {
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    PRINT_ERROR("Topology case %d does not exist.\n", topology_id);
    return false;
  }

  entry->second.run_state = state;
  return true;
}

bool CalculiXCore::get_topology_base_run_info(int topology_id, TopologyRunInfo& information)
{
  information = {};

  if (!topology)
  {
    return false;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    return false;
  }

  information.state = get_topology_run_state(topology_id);

  if (!entry->second.iteration_jobs.empty())
  {
    const auto& last = *entry->second.iteration_jobs.rbegin();
    information.iteration_id = last.first;
    information.job_id = last.second;
  }

  return true;
}

int CalculiXCore::get_topology_active_iteration_job(int topology_id, int iteration_id)
{
  if (!topology || !jobs)
  {
    return -1;
  }

  const auto entry = topology->topology_cases.find(topology_id);

  if (entry == topology->topology_cases.end())
  {
    return -1;
  }

  for (const auto* associations : {&entry->second.iteration_jobs, &entry->second.iteration_adjoint_jobs})
  {
    const auto job = associations->find(iteration_id);

    if (job == associations->end())
    {
      continue;
    }

    const int status = jobs->get_job_status(job->second);

    if (status == 1 || status == 5)
    {
      return job->second;
    }
  }

  return -1;
}

bool CalculiXCore::check_topology_density_convergence(int topology_id, int previous_iteration, int current_iteration, bool& converged)
{
  converged = false;

  if (!topology)
  {
    PRINT_ERROR("Topology core is not initialized.\n");
    return false;
  }

  return topology->check_density_convergence(topology_id, previous_iteration, current_iteration, converged);
}
