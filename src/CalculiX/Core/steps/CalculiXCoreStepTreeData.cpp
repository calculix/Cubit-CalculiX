#include "Core/CalculiXCore.hpp"
#include "CoreStepsLoadTreeData.hpp"
#include "CoreStepsBCTreeData.hpp"
#include "CoreOutputsTreeData.hpp"

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsforces_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadsforces_tree_data(*steps, *loadsforces, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadspressures_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadspressures_tree_data(*steps, *loadspressures, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsheatfluxes_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadsheatfluxes_tree_data(*steps, *loadsheatfluxes, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsgravity_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadsgravity_tree_data(*steps, *loadsgravity, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadscentrifugal_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadscentrifugal_tree_data(*steps, *loadscentrifugal, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadstrajectory_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadstrajectory_tree_data(*steps, *loadstrajectory, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsfilm_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadsfilm_tree_data(*steps, *loadsfilm, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsradiation_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadsradiation_tree_data(*steps, *loadsradiation, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadssurfacetraction_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadssurfacetraction_tree_data(*steps, *loadssurfacetraction, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_loadsbodyheatflux_tree_data(int step_id)
{
  return CoreStepsLoadTreeData::get_steps_loadsbodyheatflux_tree_data(*steps, *loadsbodyheatflux, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_bcsdisplacements_tree_data(int step_id)
{
  return CoreStepsBCTreeData::get_steps_bcsdisplacements_tree_data(*steps, *bcsdisplacements, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_bcstemperatures_tree_data(int step_id)
{
  return CoreStepsBCTreeData::get_steps_bcstemperatures_tree_data(*steps, *bcstemperatures, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_historyoutputs_tree_data(int step_id)
{
  return CoreOutputsTreeData::get_steps_historyoutputs_tree_data(*steps, *historyoutputs, step_id);
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_fieldoutputs_tree_data(int step_id)
{
  return CoreOutputsTreeData::get_steps_fieldoutputs_tree_data(*steps, *fieldoutputs, step_id);
}
