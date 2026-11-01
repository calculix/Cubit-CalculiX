#include "Core/CalculiXCore.hpp"
#include "CoreSteps.hpp"
#include "CoreStepModelChange.hpp"
#include "CoreStepsTreeData.hpp"

bool CalculiXCore::check_step_exists(int step_id)
{
  return steps->check_step_exists(step_id);
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
  return steps->add_loads(step_id, load_type, load_ids);
}

bool CalculiXCore::step_add_bcs(int step_id, int bc_type, std::vector<int> bc_ids)
{
  return steps->add_bcs(step_id, bc_type, bc_ids);
}

bool CalculiXCore::step_add_historyoutputs(int step_id, std::vector<int> historyoutput_ids)
{
  return steps->add_historyoutputs(step_id, historyoutput_ids);
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
  return steps->remove_historyoutputs(step_id, historyoutput_ids);
}

bool CalculiXCore::step_remove_fieldoutputs(int step_id, std::vector<int> fieldoutput_ids)
{
  return steps->remove_fieldoutputs(step_id, fieldoutput_ids);
}

bool CalculiXCore::step_utility_modelchangeelement_dummystep(int step_id, std::vector<int> trajectory_ids)
{
  return CoreStepModelChange::create_dummy_step(*steps, *customlines, *loadstrajectory, step_id, trajectory_ids);
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

std::vector<int> CalculiXCore::get_steps_ids()
{
  return steps->get_steps_ids();
}

std::vector<std::vector<std::string>> CalculiXCore::get_steps_tree_data()
{
  return CoreStepsTreeData::get_steps_tree_data(*steps);
}
