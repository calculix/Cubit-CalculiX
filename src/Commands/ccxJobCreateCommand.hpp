#ifndef CCXJOBCREATECOMMAND_HPP
#define CCXJOBCREATECOMMAND_HPP

#include "CubitCommandInterface.hpp"

/*!
 * \brief The Job create command lets you create your desired Jobs
 */
class ccxJobCreateCommand : public CubitCommand
{
public:
  ccxJobCreateCommand();
  ~ccxJobCreateCommand();

  std::vector<std::string> get_syntax();
  std::vector<std::string> get_syntax_help();
  std::vector<std::string> get_help();
  bool execute(CubitCommandData &data);
};

#endif // CCXJOBCREATECOMMAND_HPP