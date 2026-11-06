#include "CubitSimVersionCommand.hpp"
#include "Framework/Core/CubitSimVersion.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> CubitSimVersionCommand::get_syntax()
{
  return {"sim version"};
}

std::vector<std::string> CubitSimVersionCommand::get_syntax_help()
{
  return get_syntax();
}

std::vector<std::string> CubitSimVersionCommand::get_help()
{
  return {"Display cubit-sim, Cubit, geometry, graphics, and Exodus versions."};
}

bool CubitSimVersionCommand::execute(CubitCommandData&)
{
  PRINT_INFO("\tcubit-sim Version %s\n", CubitSim::Version);
  PRINT_INFO("\tCubit Version %s Build %s\n",
    CubitInterface::get_version().c_str(), CubitInterface::get_build_number().c_str());
  PRINT_INFO("\t%s\n", CubitInterface::get_graphics_version().c_str());
  PRINT_INFO("\t%s\n", CubitInterface::get_acis_version().c_str());
  PRINT_INFO("\tExodus Version %s\n", CubitInterface::get_exodus_version().c_str());
  PRINT_INFO("\tRevised %s\n\n", CubitInterface::get_revision_date().c_str());
  return true;
}
