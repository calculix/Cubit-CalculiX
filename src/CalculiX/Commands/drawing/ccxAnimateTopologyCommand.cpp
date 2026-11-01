#include "ccxAnimateTopologyCommand.hpp"
#include <string>
#include <vector>
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxAnimateTopologyCommand::get_syntax()
{
  return {
      "ccx animate topology <value:label='topology_id'> "
      "[threshold <value:label='threshold'>] "
      "[fps <value:label='fps'>] "
      "[save <string:type='unquoted', number='1', "
      "label='save_filepath', help='<save_filepath>'>] "
      "[overwrite]"
  };
}

std::vector<std::string> ccxAnimateTopologyCommand::get_syntax_help()
{
  return {
      "ccx animate topology <topology_id> "
      "[threshold <threshold>] [fps <fps>] [save <save_filepath>] "
      "[overwrite]"
  };
}

std::vector<std::string> ccxAnimateTopologyCommand::get_help()
{
  return {
      "Start asynchronous export of a looping GIF from completed topology iterations. "
      "Kept elements are green; removal candidates have red edges. "
      "Success means startup; completion or failure is reported later. "
      "Cancel from the topology Results panel. Avoid changing the mesh or graphics view during capture."
  };
}

bool ccxAnimateTopologyCommand::execute(CubitCommandData& data)
{
  CalculiXDrawInterface ccx_draw_iface;


  int topology_id = -1;
  double threshold = 0.5;
  int fps = 5;
  std::string save_filepath;

  if (!data.get_value("topology_id", topology_id))
  {
    PRINT_ERROR("Missing topology ID.\n");
    return false;
  }

  if (data.find_keyword("threshold"))
  {
    if (!data.get_value("threshold", threshold))
    {
      PRINT_ERROR("Invalid topology threshold.\n");
      return false;
    }
  }

  if (data.find_keyword("fps"))
  {
    if (!data.get_value("fps", fps))
    {
      PRINT_ERROR("Invalid animation frame rate.\n");
      return false;
    }
  }

  if (data.find_keyword("save"))
  {
    if (!data.get_string("save_filepath", save_filepath) ||
        save_filepath.empty())
    {
      PRINT_ERROR("Missing animation save path.\n");
      return false;
    }
  }

  return ccx_draw_iface.animate_topology(topology_id, threshold, fps, save_filepath,data.find_keyword("overwrite"));
}