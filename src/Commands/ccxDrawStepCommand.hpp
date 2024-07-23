#ifndef CCXDRAWSTEPCOMMAND_HPP
#define CCXDRAWSTEPCOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Draw command lets you draw the loads, bcs and orientations
 */
class ccxDrawStepCommand : public CubitCommand
{
public:
  ccxDrawStepCommand();
  ~ccxDrawStepCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);

  std::vector<int> get_ids_in_tree_data(std::vector<int> in_ids,std::vector<std::vector<std::string>> tree_data);
  std::vector<int> convert_tree_data(std::vector<std::vector<std::string>> tree_data);
};

#endif // CCXDRAWSTEPCOMMAND_HPP