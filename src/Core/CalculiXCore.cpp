#include "CalculiXCore.hpp"
#include <unistd.h>
#include <fstream>
#include <iostream>
#include <sstream>
#include <chrono>
#include <ctime>
#include <algorithm>
#include <cmath>

#include "CubitInterface.hpp"
#include "CubitCoreformInterface.hpp"
#include "CubitMessage.hpp"
#include "MeshExportInterface.hpp"
#include "MaterialInterface.hpp"

#include "CoreBlocks.hpp"
#include "CoreMaterials.hpp"
#include "CoreSections.hpp"
#include "CoreConstraints.hpp"
#include "CoreSurfaceInteractions.hpp"
#include "CoreContactPairs.hpp"
#include "CoreAmplitudes.hpp"
#include "CoreLoadsForces.hpp"
#include "CoreLoadsPressures.hpp"
#include "CoreLoadsHeatfluxes.hpp"
#include "CoreLoadsGravity.hpp"
#include "CoreLoadsCentrifugal.hpp"
#include "CoreBCsDisplacements.hpp"
#include "CoreBCsTemperatures.hpp"
#include "CoreHistoryOutputs.hpp"
#include "CoreFieldOutputs.hpp"
#include "CoreInitialConditions.hpp"
#include "CoreHBCs.hpp"
#include "CoreSteps.hpp"
#include "CoreJobs.hpp"
#include "CoreResults.hpp"
#include "CoreResultsFrd.hpp"
#include "CoreResultsDat.hpp"
#include "CoreTimer.hpp"
#include "CoreCustomLines.hpp"
#include "loadUserOptions.hpp"
#include "CoreDraw.hpp"

#include <Utility/Eigen/Eigenvalues>

CalculiXCore::CalculiXCore():
  cb(NULL),mat(NULL),sections(NULL),constraints(NULL),surfaceinteractions(NULL),
  contactpairs(NULL),amplitudes(NULL),loadsforces(NULL),loadspressures(NULL),loadsheatfluxes(NULL),
  loadsgravity(NULL),loadscentrifugal(NULL),
  bcsdisplacements(NULL),bcstemperatures(NULL), historyoutputs(NULL), fieldoutputs(NULL),
  initialconditions(NULL), hbcs(NULL), steps(NULL),jobs(NULL),results(NULL),timer(NULL),customlines(NULL),
  draw(NULL)
{
  init();
}

CalculiXCore::~CalculiXCore()
{
  if(cb)
    delete cb;
  if(mat)
    delete mat;
  if(sections)
    delete sections;
  if(constraints)
    delete constraints;
  if(surfaceinteractions)
    delete surfaceinteractions;
  if(contactpairs)
    delete contactpairs;
  if(amplitudes)
    delete amplitudes;
  if(loadsforces)
    delete loadsforces;
  if(loadspressures)
    delete loadspressures;
  if(loadsheatfluxes)
    delete loadsheatfluxes;
  if(loadsgravity)
    delete loadsgravity;
  if(loadscentrifugal)
    delete loadscentrifugal;
  if(bcsdisplacements)
    delete bcsdisplacements;
  if(bcstemperatures)
    delete bcstemperatures;
  if(historyoutputs)
    delete historyoutputs;
  if(fieldoutputs)
    delete fieldoutputs;
  if(initialconditions)
    delete initialconditions;
  if(hbcs)
    delete hbcs;
  if(steps)
    delete steps;
  if(jobs)
    delete jobs;
  if(results)
    delete results;
  if(timer)
    delete timer;
  if(customlines)
    delete customlines;
  if(draw)
    delete draw;
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

bool CalculiXCore::init()
{
  me_iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
  me_iface->initialize_export();
  mat_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));

  if(!cb)
    cb = new CoreBlocks;
  
  cb->init();

  if(!mat)
    mat = new CoreMaterials;
  
  mat->init();
  
  if(!sections)
    sections = new CoreSections;
  
  sections->init();

  if(!constraints)
    constraints = new CoreConstraints;
  
  constraints->init();

  if(!surfaceinteractions)
    surfaceinteractions = new CoreSurfaceInteractions;
  
  surfaceinteractions->init();

  if(!contactpairs)
    contactpairs = new CoreContactPairs;
  
  contactpairs->init();

  if(!amplitudes)
    amplitudes = new CoreAmplitudes;
  
  amplitudes->init();
  
  if(!loadsforces)
    loadsforces = new CoreLoadsForces;
  
  loadsforces->init();

  if(!loadspressures)
    loadspressures = new CoreLoadsPressures;
  
  loadspressures->init();

  if(!loadsheatfluxes)
    loadsheatfluxes = new CoreLoadsHeatfluxes;
  
  loadsheatfluxes->init();

  if(!loadsgravity)
    loadsgravity = new CoreLoadsGravity;
  
  loadsgravity->init();

  if(!loadscentrifugal)
    loadscentrifugal = new CoreLoadsCentrifugal;
  
  loadscentrifugal->init();

  if(!bcsdisplacements)
    bcsdisplacements = new CoreBCsDisplacements;
  
  bcsdisplacements->init();

  if(!bcstemperatures)
    bcstemperatures = new CoreBCsTemperatures;
  
  bcstemperatures->init();

  if(!historyoutputs)
    historyoutputs = new CoreHistoryOutputs;
  
  historyoutputs->init();

  if(!fieldoutputs)
    fieldoutputs = new CoreFieldOutputs;
  
  fieldoutputs->init();

  if(!initialconditions)
    initialconditions = new CoreInitialConditions;
  
  initialconditions->init();

  if(!hbcs)
    hbcs = new CoreHBCs;
  
  hbcs->init();

  if(!steps)
    steps = new CoreSteps;
  
  steps->init();

  if(!jobs)
    jobs = new CoreJobs;
  
  jobs->init();

  if(!results)
    results = new CoreResults;
  
  results->init();

  if(!timer)
    timer = new CoreTimer;

  if(!customlines)
    customlines = new CoreCustomLines;
  
  customlines->init();

  if(!draw)
    draw = new CoreDraw;
  
  draw->init();

  if (use_ccx_logfile)
  {
    print_to_log("CalculiXCore Initialization!");
  }

  this->bool_init = true;
  this->bool_init_pythoninterface = false;

  return true;
}

bool CalculiXCore::init_pythoninterface()
{
  // init_pythoninterface(); needs to be initialized after the cubit is fully loaded; 
  // will be initialized with the core timer on its first evocation

  if (!this->bool_init_pythoninterface)
  {
    std::vector<std::string> command;
    command.push_back("#!python");
    command.push_back("import sys");
    command.push_back("sys.path.append('" + ccx_uo.mPathPythonInterface.toStdString() + "')");
    command.push_back("from calculix_pythoninterface import *");
    command.push_back("ccx = CalculiXPythonInterface()");
    command.push_back("#!cubit");
    
    for (size_t i = 0; i < command.size(); i++)
    {
      //CubitInterface::silent_cmd_without_running_journal_lines(command[i].c_str());
      CubitInterface::cubit_or_python_cmd(command[i].c_str());
    }

    this->bool_init_pythoninterface = true;
    return true;
  }

  return false;
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

  return true;
}

bool CalculiXCore::reset()
{
  cb->reset();
  //mat->reset();
  sections->reset();
  constraints->reset();
  surfaceinteractions->reset();
  contactpairs->reset();
  amplitudes->reset();
  loadsforces->reset();
  loadspressures->reset();
  loadsheatfluxes->reset();
  loadsgravity->reset();
  loadscentrifugal->reset();
  bcsdisplacements->reset();
  bcstemperatures->reset();
  historyoutputs->reset();
  fieldoutputs->reset();
  initialconditions->reset();
  hbcs->reset();
  steps->reset();
  jobs->reset();
  results->reset();
  customlines->reset();
  //draw->reset();

  sideset_face_data.clear();
  //print_to_log("RESET");
  //print_to_log(print_data());

  return true;
}

std::string CalculiXCore::autocleanup()
{
  std::string log = "";
  bool print_log = false;
  int sub_data_id;
  std::vector<int> sub_data_ids;
  bool sub_bool;

  std::string output;

  log = "CCX Autocleanup\n";

  // SECTIONS
  //loop over all sections
  for (size_t i = sections->sections_data.size(); i > 0; i--)
  { 
    sub_bool = false;
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
    }
    if (sub_bool)
    {
      print_log = sub_bool;
      sections->delete_section(sections->sections_data[i-1][0]);
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
  sub_data_ids = hbcs->get_bc_data_ids_from_bcs_id(0);
  for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
  {
    // Displacement 
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
    // Temperature
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
    sub_data_ids = steps->get_load_data_ids_from_loads_id(steps->steps_data[i-1][5]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      // Force 
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
      // Pressure
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
      // Heatflux
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
      // Gravity
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
      // Centrifugal
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
    // STEP BCS
    sub_data_ids = steps->get_bc_data_ids_from_bcs_id(steps->steps_data[i-1][6]);
    for (size_t ii = sub_data_ids.size(); ii > 0; ii--)
    {
      // Displacement 
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
      // Temperature
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
  str_return.append(loadsforces->print_data());
  str_return.append(loadspressures->print_data());
  str_return.append(loadsheatfluxes->print_data());
  str_return.append(loadsgravity->print_data());
  str_return.append(loadscentrifugal->print_data());
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

std::string  CalculiXCore::to_string_scientific(double value, int precision)
{
  std::string output;
  std::ostringstream ss;
  ss.precision(precision);
  ss << std::scientific << value;
  output = ss.str();
  return output;
}

double  CalculiXCore::string_scientific_to_double(std::string value, int precision)
{
  double output;
  if (value[0] == '-') // check if negative
  {
    value.replace(0,1, "");
    //check if is INF
    if (value=="INF")
    {
      output = -1.255070e+29;
    }else{
      output = std::stod(value);
      output = -1*output;
    }
  } else {
    //check if is INF
    if (value=="INF")
    {
      output = 1.255070e+29;
    }else{
      output = std::stod(value);
    }
  }
  //apply precision
  output = std::stod(this->to_string_scientific(output,precision));

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

std::string CalculiXCore::get_material_name(int material_id)
{
  std::string material_name;
  MaterialInterface::Material material;
  material = mat_iface->get_material(material_id);
  material_name = mat_iface->get_material_name(material);
  
  return material_name;
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
  std::vector<int> vertex_list = CubitCoreformInterface::get_entities(CubitCoreformInterface::CubitCoreformInterfaceEntityType::VERTEX);
  for (size_t i = 0; i < vertex_list.size(); i++)
  {
    if (vertex_list[i]==vertex_id)
    {
      return true;
    }
  }
  return false;

  /*if (!CubitCoreformInterface::is_entity(CubitCoreformInterface::CubitCoreformInterfaceEntityType::VERTEX,vertex_id))
  {
    return false;
  }
  return true;
  */
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
  } else if ((element_type>=48) && (element_type<=50)) {
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

bool CalculiXCore::create_constraint(std::string constraint_type, std::vector<std::string> options)
{
  return constraints->create_constraint(constraint_type, options);
}

bool CalculiXCore::modify_constraint(std::string constraint_type,int constraint_id, std::vector<std::string> options, std::vector<int> options_marker)
{
  return constraints->modify_constraint(constraint_type, constraint_id, options, options_marker);
}

bool CalculiXCore::delete_constraint(int constraint_id)
{
  return constraints->delete_constraint(constraint_id);
}

bool CalculiXCore::create_constraint_tie_from_cubitcontactpair(std::string name, std::string position_tolerance) // create constraint tie from cubit contact pairs
{
  std::vector<std::string> options;

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

    this->create_constraint("TIE", options);
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

bool CalculiXCore::run_job(int job_id, int option)
{
  return jobs->run_job(job_id,option);
}

bool CalculiXCore::check_jobs()
{
  return jobs->check_jobs();
}

bool CalculiXCore::check_zombie()
{
  return jobs->check_zombie();
}

bool CalculiXCore::wait_job(int job_id)
{
  return jobs->wait_job(job_id);
}

bool CalculiXCore::kill_job(int job_id)
{
  return jobs->kill_job(job_id);
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

int CalculiXCore::convert_result(int job_id, int option, std::vector<int> block_ids, std::vector<int> nodeset_ids, std::vector<int> sideset_ids)
{
  return results->convert_result(job_id, option, block_ids, nodeset_ids, sideset_ids);
}

bool CalculiXCore::project_result(int job_id,int step,int totalincrement,double scale)
{
  return results->project_result(job_id,step,totalincrement,scale);
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
      entities.push_back({"displacement",initialconditions->displacement_data[sub_data_id][1]});
    }else if (initialconditions->initialconditions_data[data_id][1]==2)
    {
      sub_data_id = initialconditions->get_temperature_data_id_from_temperature_id(initialconditions->initialconditions_data[data_id][2]);
      entities.push_back({"temperature",initialconditions->temperature_data[sub_data_id][1]});
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

std::vector<std::vector<double>> CalculiXCore::get_draw_data_for_load_force(int id)
{
  int bc_set_id=-1;
  BCSetHandle bc_set;
  NodesetHandle nodeset;
  std::string command;
  std::vector<BCEntityHandle> bc_handles;
  BCEntityHandle bc_handle;
  std::vector<MeshExportBCData> bc_attribs;
  std::vector<std::vector<double>> draw_data;
  std::vector<int> nodes;

  me_iface->create_default_bcset(0,true,true,true,bc_set);
  bc_set_id = me_iface->id_from_handle(bc_set);

  me_iface->get_bc_loads(bc_set, bc_handles);
  for (size_t i = 0; i < bc_handles.size(); i++)
  {  
    me_iface->get_bc_attributes(bc_handles[i],bc_attribs); 
    if ((get_bc_fea_type(bc_attribs)==5))
    {
      if (id==me_iface->id_from_handle(bc_handles[i]))
      {     
        me_iface->get_bc_nodeset(bc_handles[i],nodeset);
        
        nodes = CubitInterface::get_nodeset_nodes_inclusive(me_iface->id_from_handle(nodeset)); 	
        
        for (size_t ii = 0; ii < nodes.size(); ii++)
        {
          std::array<double,3> coord = CubitInterface::get_nodal_coordinates(nodes[ii]);
          std::vector<double> data;
          
          data.push_back(coord[0]);
          data.push_back(coord[1]);
          data.push_back(coord[2]);
          data.push_back(bc_attribs[1].second*bc_attribs[0].second);
          data.push_back(bc_attribs[2].second*bc_attribs[0].second);
          data.push_back(bc_attribs[3].second*bc_attribs[0].second);
          draw_data.push_back(data);

          data[3] = bc_attribs[5].second*bc_attribs[4].second;
          data[4] = bc_attribs[6].second*bc_attribs[4].second;
          data[5] = bc_attribs[7].second*bc_attribs[4].second;
          draw_data.push_back(data);
        }
      }
    }
  }

  command = "delete bcset " + std::to_string(bc_set_id);
  CubitInterface::silent_cmd_without_running_journal_lines(command.c_str());

  return draw_data;
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

std::vector<int> CalculiXCore::frd_get_node_ids_between_limits(int job_id,int total_increment,std::string result_block_type,std::string result_block_component,double lower_limit,double upper_limit)
{
  //ccx.frd_get_node_ids_between_limits(1,1,"STRESS","MISES",0,2)
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
            //result_block_node_data[0][0][0] node_id
            //result_block_node_data[0][0][1] result_block_node_data_id

             //result_block_data[result block data id][node][component]
            //result_block_data[0][0][0] component...
            int node_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][0];
            int node_data_id = results->frd_data[frd_data_id].result_block_node_data[i][ii][1];
            double value = results->frd_data[frd_data_id].result_block_data[i][node_data_id][component_id];
            if ((value >= lower_limit)&&(value <= upper_limit))
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

QIcon* CalculiXCore::getIcon(std::string name)
{
  QIcon* icon = new QIcon();
  std::string filepath = "";

  filepath = ccx_uo.mPathIcons.toStdString() + name + ".svg";

  if (access(filepath.c_str(), F_OK) == 0) 
  {
    icon = new QIcon(QString::fromStdString(filepath));
  }else{
  }

  return icon;
}

QIcon CalculiXCore::getIcon2(std::string name)
{
  QIcon icon;
  std::string filepath = "";

  filepath = ccx_uo.mPathIcons.toStdString() + name + ".svg";

  if (access(filepath.c_str(), F_OK) == 0) 
  {
    icon = QIcon(QString::fromStdString(filepath));
  }else{
  }

  return icon;
}

std::string CalculiXCore::get_material_export_data() // gets the export data from materials core
{
  return mat->get_material_export();
}

std::string CalculiXCore::get_section_export_data() // gets the export data from sections core
{
  return sections->get_section_export();
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

std::string CalculiXCore::get_initialcondition_export_data() // gets the export data from core
{
  std::vector<std::string> initialconditions_export_list;
  initialconditions_export_list.push_back("********************************** I N I T I A L C O N D I T I O N S ****************************");
  std::string str_temp;
  std::string log;
  int sub_data_id;
  std::string command;
  int bc_set_id=-1;
  BCSetHandle bc_set;
  NodesetHandle nodeset;
  std::vector<BCEntityHandle> bc_handles;
  BCEntityHandle bc_handle;
  std::vector<MeshExportBCData> bc_attribs; 

  //loop over all initialconditions
  for (size_t i = 0; i < initialconditions->initialconditions_data.size(); i++)
  { 
    if (i==0)
    { 
      log = "Creating BCSet for exporting Initial Conditions.\n";
      PRINT_INFO("%s", log.c_str());
      me_iface->create_default_bcset(0,true,true,true,bc_set);
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
  int sub_data_id;
  std::vector<int> sub_data_ids;
  std::string command;
  int bc_set_id=-1;
  BCSetHandle bc_set;
  NodesetHandle nodeset;
  SidesetHandle sideset;
  std::vector<BCEntityHandle> bc_handles;
  BCEntityHandle bc_handle;
  std::vector<MeshExportBCData> bc_attribs; 
  std::vector<std::string> customline;

  log = "Creating BCSet for exporting Homogeneous Boundary Conditions.\n";
  PRINT_INFO("%s", log.c_str());
  me_iface->create_default_bcset(0,true,true,true,bc_set);
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
  std::vector<std::string> steps_export_list;
  steps_export_list.push_back("********************************** S T E P S ****************************");
  std::string str_temp;
  std::vector<std::vector<std::string>> temp_list;
  std::string log;
  int sub_data_id;
  std::vector<int> sub_data_ids;
  std::string command;
  int bc_set_id=-1;
  BCSetHandle bc_set;
  NodesetHandle nodeset;
  SidesetHandle sideset;
  std::vector<BCEntityHandle> bc_handles;
  BCEntityHandle bc_handle;
  std::vector<MeshExportBCData> bc_attribs; 
  std::vector<std::string> customline;

  //loop over all steps
  for (size_t i = 0; i < steps->steps_data.size(); i++)
  { 
    if (i==0)
    { 
      log = "Creating BCSet for exporting Steps.\n";
      PRINT_INFO("%s", log.c_str());
      me_iface->create_default_bcset(0,true,true,true,bc_set);
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
    int contactpair_name_id;

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
    
    name = "Gravity_" + std::to_string(loadsgravity->loads_data[i][0]);
    
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
    
    name = "Centrifugal_" + std::to_string(loadscentrifugal->loads_data[i][0]);
    
    loadscentrifugal_tree_data_set.push_back(std::to_string(loadscentrifugal->loads_data[i][0])); //load_id
    loadscentrifugal_tree_data_set.push_back(name); 
    loadscentrifugal_tree_data.push_back(loadscentrifugal_tree_data_set);
  }
  return loadscentrifugal_tree_data;
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
      name = "Gravity_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
    
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
      name = "Centrifugal_" + std::to_string(steps->loads_data[loads_ids[i]][2]);
    
      loadscentrifugal_tree_data_set.push_back(std::to_string(steps->loads_data[loads_ids[i]][2])); //load_id
      loadscentrifugal_tree_data_set.push_back(name); 
      loadscentrifugal_tree_data.push_back(loadscentrifugal_tree_data_set);  
    }
  }
  return loadscentrifugal_tree_data;
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
        to_ids.push_back(i);
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

bool CalculiXCore::draw_all() // draw all bc and loads
{
  return draw->draw_all();
}