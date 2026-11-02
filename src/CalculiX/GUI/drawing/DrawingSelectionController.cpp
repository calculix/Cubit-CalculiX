#include "DrawingSelectionController.hpp"
#include "Core/drawing/CalculiXDrawInterface.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "CubitMessage.hpp"
#include <string>

DrawingSelectionController::DrawingSelectionController(QObject* parent)
  : QObject(parent)
{
}

void DrawingSelectionController::showSelection(const PropertySelection& selection)
{
  if (selection.id < 0)
  {
    clearSelection();
    return;
  }
  switch (selection.feature)
  {
    case PropertyFeature::Force:
    {
      selectFeature(DrawFeature::Force, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Pressure:
    {
      selectFeature(DrawFeature::Pressure, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Heatflux:
    {
      selectFeature(DrawFeature::Heatflux, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::SurfaceTraction:
    {
      selectFeature(DrawFeature::SurfaceTraction, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Gravity:
    {
      selectFeature(DrawFeature::Gravity, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Centrifugal:
    {
      selectFeature(DrawFeature::Centrifugal, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Film:
    {
      selectFeature(DrawFeature::Film, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Radiation:
    {
      selectFeature(DrawFeature::Radiation, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::BodyHeatflux:
    {
      selectFeature(DrawFeature::BodyHeatflux, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Trajectory:
    {
      selectFeature(DrawFeature::Trajectory, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Displacement:
    {
      selectFeature(DrawFeature::Displacement, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Temperature:
    {
      selectFeature(DrawFeature::Temperature, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Orientation:
    {
      selectFeature(DrawFeature::Orientation, selection.id, selection.stepId);
      break;
    }
    case PropertyFeature::Constraint:
    {
      selectConstraint(selection.id);
      break;
    }
    default:
    {
      clearSelection();
      break;
    }
  }
}

void DrawingSelectionController::selectFeature(DrawFeature feature, int id, int stepId)
{
  CalculiXDrawSelection selection;
  selection.active = true;
  selection.entity = {feature, id};
  selection.stepId = stepId;
  CalculiXDrawInterface drawing;
  std::string error;
  if (!drawing.set_highlight(selection, error))
  {
    PRINT_ERROR("CalculiX selection highlighting failed: %s\n", error.c_str());
  }
}

void DrawingSelectionController::selectConstraint(int id)
{
  CalculiXInteractionsInterface interactions;
  ConstraintsSettings settings;
  std::string error;
  if (!interactions.get_constraint_settings(id, settings, error))
  {
    // A tree item may disappear while the model is being refreshed.
    clearSelection();
    return;
  }
  switch (settings.type)
  {
    case 3:
    {
      selectFeature(DrawFeature::Equation, id, -1);
      break;
    }
    case 4:
    {
      selectFeature(DrawFeature::EquationGroup, id, -1);
      break;
    }
    default:
    {
      clearSelection();
      break;
    }
  }
}

void DrawingSelectionController::clearSelection()
{
  CalculiXDrawInterface drawing;
  std::string error;
  if (!drawing.clear_highlight(error))
  {
    PRINT_ERROR("CalculiX selection highlighting could not be cleared: %s\n", error.c_str());
  }
}
