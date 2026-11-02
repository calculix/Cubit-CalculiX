#include "ccxDisplayCommand.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "CubitMessage.hpp"

std::vector<std::string> ccxDisplayCommand::get_syntax()
{
  return {
    "ccx display scope model",
    "ccx display scope step <value:label='step_id',help='<step_id>'>",
    "ccx display {overlay|all|bcs|loads|constraints|orientations|topology} {on|off}",
    "ccx display size <value:label='factor',help='<factor>'>",
    "ccx display picking {on|off}",
    "ccx display defaults",
    "ccx display refresh"
  };
}

std::vector<std::string> ccxDisplayCommand::get_syntax_help()
{
  return {"ccx display scope {model|step <id>}", "ccx display {overlay|all|bcs|loads|constraints|orientations|topology} {on|off}", "ccx display size <factor>", "ccx display picking {on|off}", "ccx display defaults", "ccx display refresh"};
}

std::vector<std::string> ccxDisplayCommand::get_help()
{
  return {"Control persistent CalculiX drawing groups. Overlay on/off preserves group selections and drawing settings. Picking on/off selects CalculiX symbols and restores the previous Cubit pick mode when disabled. Defaults resets drawing size and visibility to the configured user defaults and disables picking. Graphics are visible only in the CalculiX workspace. Size is a factor of the automatic model-based symbol size."};
}

bool ccxDisplayCommand::execute(CubitCommandData& data)
{
  CalculiXDrawInterface drawing;
  std::string error;
  bool success = false;
  if (data.find_keyword("SCOPE"))
  {
    DrawScope scope;
    if (data.find_keyword("STEP"))
    {
      scope.kind = DrawScopeKind::Step;
      if (!data.get_value("step_id", scope.stepId))
      {
        PRINT_ERROR("Select one drawing step.\n");
        return false;
      }
    }
    success = drawing.set_scope(scope, error);
  }
  else if (data.find_keyword("SIZE"))
  {
    double factor = 0;
    success = data.get_value("factor", factor) && drawing.set_size_factor(factor, error);
  }
  else if (data.find_keyword("PICKING"))
  {
    success = drawing.set_picking_enabled(data.find_keyword("ON"), error);
  }
  else if (data.find_keyword("DEFAULTS"))
  {
    success = drawing.apply_drawing_defaults(error);
  }
  else if (data.find_keyword("REFRESH"))
  {
    success = drawing.refresh_drawing(error);
  }
  else
  {
    const bool visible = data.find_keyword("ON");
    if (data.find_keyword("OVERLAY"))
    {
      success = drawing.set_overlay_visible(visible, error);
    }
    else if (data.find_keyword("ALL"))
    {
      success = drawing.set_all_visible(visible, error);
    }
    else
    {
      DrawGroup group = DrawGroup::BoundaryConditions;
      if (data.find_keyword("LOADS"))
      {
        group = DrawGroup::Loads;
      }
      else if (data.find_keyword("CONSTRAINTS"))
      {
        group = DrawGroup::Constraints;
      }
      else if (data.find_keyword("ORIENTATIONS"))
      {
        group = DrawGroup::Orientations;
      }
      else if (data.find_keyword("TOPOLOGY"))
      {
        group = DrawGroup::Topology;
      }
      success = drawing.set_group_visible(group, visible, error);
    }
  }
  if (!success)
  {
    PRINT_ERROR("Drawing command failed: %s\n", error.empty() ? "invalid request" : error.c_str());
  }
  return success;
}
