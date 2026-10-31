#include "Core/CalculiXCore.hpp"
#include "CoreIdSelection.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/sections/CoreOrientations.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "Core/amplitudes/CoreAmplitudes.hpp"
#include "Core/bcs/CoreBCsDisplacements.hpp"
#include "Core/bcs/CoreBCsTemperatures.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "Core/steps/CoreInitialConditions.hpp"
#include "Core/steps/CoreSteps.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/loads/CoreLoadsQuery.hpp"

std::vector<int> CalculiXCore::parser(std::string parse_type, std::string parse_string)
{
  if (parse_string.find("all") == std::string::npos)
  {
    return CoreIdSelection::parse(parse_string, {});
  }

  std::vector<int> all_ids;

  if (parse_type == "section")
  {
    all_ids = sections->get_section_ids();
  }
  else if (parse_type == "constraint")
  {
    all_ids = constraints->get_constraint_ids();
  }
  else if (parse_type == "surfaceinteraction")
  {
    all_ids = surfaceinteractions->get_surfaceinteraction_ids();
  }
  else if (parse_type == "contactpair")
  {
    all_ids = contactpairs->get_contactpair_ids();
  }
  else if (parse_type == "amplitude")
  {
    all_ids = amplitudes->get_amplitude_ids();
  }
  else if (parse_type == "orientation")
  {
    all_ids = orientations->get_orientations_ids();
  }
  else if (parse_type == "bcstemperatures")
  {
    all_ids = bcstemperatures->get_bc_ids();
  }
  else if (parse_type == "bcsdisplacements")
  {
    all_ids = bcsdisplacements->get_bc_ids();
  }
  else if (parse_type == "historyoutput")
  {
    all_ids = historyoutputs->get_output_ids();
  }
  else if (parse_type == "fieldoutput")
  {
    all_ids = fieldoutputs->get_output_ids();
  }
  else if (parse_type == "initialcondition")
  {
    all_ids = initialconditions->get_initialcondition_ids();
  }
  else if (parse_type == "step")
  {
    all_ids = steps->get_steps_ids();
  }
  else if (parse_type == "job")
  {
    all_ids = jobs->get_job_ids();
  }
  else if (parse_type == "customline")
  {
    all_ids = customlines->get_customline_ids();
  }
  else
  {
    all_ids = CoreLoadsQuery::get_load_ids(get_load_query_data(), parse_type);
  }

  return CoreIdSelection::parse(parse_string, all_ids);
}
