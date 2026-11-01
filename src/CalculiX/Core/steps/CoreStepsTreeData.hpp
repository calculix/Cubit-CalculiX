#ifndef CORESTEPSTREEDATA_HPP
#define CORESTEPSTREEDATA_HPP

#include <string>
#include <vector>

class CoreSteps;

class CoreStepsTreeData
{
public:
  static std::vector<std::vector<std::string>> get_steps_tree_data(CoreSteps& steps);
};

#endif
