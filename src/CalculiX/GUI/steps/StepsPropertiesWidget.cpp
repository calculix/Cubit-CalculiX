#include "StepsPropertiesWidget.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitMessage.hpp"

StepsPropertiesWidget::StepsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool StepsPropertiesWidget::setStep(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXStepsInterface core;
  StepsSettings settings;
  std::string error;
  if (!core.get_step_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Step properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Step", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* parametersGroup = addGroup("step_parameters", "Step parameters", PropertySection::Settings);
  addValue(parametersGroup, "Perturbation", QString::fromStdString(settings.parameters.at(1)));
  addValue(parametersGroup, "Nonlinear Geometry", QString::fromStdString(settings.parameters.at(2)));
  addValue(parametersGroup, "Increment", QString::fromStdString(settings.parameters.at(3)));
  addValue(parametersGroup, "Increment Fluid", QString::fromStdString(settings.parameters.at(4)));
  addValue(parametersGroup, "Thermal Network", QString::fromStdString(settings.parameters.at(5)));
  addValue(parametersGroup, "Amplitude Type", QString::fromStdString(settings.parameters.at(6)));
  addValue(parametersGroup, "Shocksmoothing", QString::fromStdString(settings.parameters.at(7)));
  auto* typeGroup = addGroup("classification", "Classification", PropertySection::Classification);
  switch (settings.type)
  {
    case 1:
    {
      addValue(typeGroup, "Subtype", "Parameters only");
      break;
    }
    case 2:
    {
      addValue(typeGroup, "Subtype", "Static");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Direct", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "Explicit", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "Time Reset", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "Total Time at Start", QString::fromStdString(settings.procedure.at(5)));
      addValue(settingsGroup, "Initial  Time Increment", QString::fromStdString(settings.procedure.at(6)));
      addValue(settingsGroup, "Time period of Step", QString::fromStdString(settings.procedure.at(7)));
      addValue(settingsGroup, "Minimum Time Increment", QString::fromStdString(settings.procedure.at(8)));
      addValue(settingsGroup, "Maximum Time Increment", QString::fromStdString(settings.procedure.at(9)));
      addValue(settingsGroup, "Initial  Time Increment CFD", QString::fromStdString(settings.procedure.at(10)));
      break;
    }
    case 3:
    {
      addValue(typeGroup, "Subtype", "Frequency");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Storage", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "Global", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "CYCMPC", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "Eigenfrequencies", QString::fromStdString(settings.procedure.at(5)));
      addValue(settingsGroup, "lower", QString::fromStdString(settings.procedure.at(6)));
      addValue(settingsGroup, "upper", QString::fromStdString(settings.procedure.at(7)));
      break;
    }
    case 4:
    {
      addValue(typeGroup, "Subtype", "Buckle");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Buckling Factors", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "Accuracy", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "Lanczos Vectors", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "Maximum Iterations", QString::fromStdString(settings.procedure.at(5)));
      break;
    }
    case 5:
    {
      addValue(typeGroup, "Subtype", "Heat transfer");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Direct", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "Steady State", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "Frequency", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "Modal Dynamic", QString::fromStdString(settings.procedure.at(5)));
      addValue(settingsGroup, "Storage", QString::fromStdString(settings.procedure.at(6)));
      addValue(settingsGroup, "DELTMX", QString::fromStdString(settings.procedure.at(7)));
      addValue(settingsGroup, "Time Reset", QString::fromStdString(settings.procedure.at(8)));
      addValue(settingsGroup, "Total Time at Start", QString::fromStdString(settings.procedure.at(9)));
      addValue(settingsGroup, "Initial  Time Increment", QString::fromStdString(settings.procedure.at(10)));
      addValue(settingsGroup, "Time Period of Step", QString::fromStdString(settings.procedure.at(11)));
      addValue(settingsGroup, "Minimum  Time Increment", QString::fromStdString(settings.procedure.at(12)));
      addValue(settingsGroup, "Maximum  Time Increment", QString::fromStdString(settings.procedure.at(13)));
      addValue(settingsGroup, "Eigenfrequencies", QString::fromStdString(settings.procedure.at(14)));
      addValue(settingsGroup, "lower", QString::fromStdString(settings.procedure.at(15)));
      addValue(settingsGroup, "upper", QString::fromStdString(settings.procedure.at(16)));
      break;
    }
    case 6:
    {
      addValue(typeGroup, "Subtype", "Coupled temperature-displacement");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Direct", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "Alpha", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "Steady State", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "DELTMX", QString::fromStdString(settings.procedure.at(5)));
      addValue(settingsGroup, "Time Reset", QString::fromStdString(settings.procedure.at(6)));
      addValue(settingsGroup, "Total Time at Start", QString::fromStdString(settings.procedure.at(7)));
      addValue(settingsGroup, "Compressible", QString::fromStdString(settings.procedure.at(8)));
      addValue(settingsGroup, "Initial  Time Increment", QString::fromStdString(settings.procedure.at(9)));
      addValue(settingsGroup, "Time Period of Step", QString::fromStdString(settings.procedure.at(10)));
      addValue(settingsGroup, "Minimum  Time Increment", QString::fromStdString(settings.procedure.at(11)));
      addValue(settingsGroup, "Maximum  Time Increment", QString::fromStdString(settings.procedure.at(12)));
      addValue(settingsGroup, "Initial  Time Increment CFD", QString::fromStdString(settings.procedure.at(13)));
      break;
    }
    case 7:
    {
      addValue(typeGroup, "Subtype", "Uncoupled temperature-displacement");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Direct", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "Alpha", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "Steady State", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "DELTMX", QString::fromStdString(settings.procedure.at(5)));
      addValue(settingsGroup, "Explicit", QString::fromStdString(settings.procedure.at(6)));
      addValue(settingsGroup, "Time Reset", QString::fromStdString(settings.procedure.at(7)));
      addValue(settingsGroup, "Total Time at Start", QString::fromStdString(settings.procedure.at(8)));
      addValue(settingsGroup, "Initial  Time Increment", QString::fromStdString(settings.procedure.at(9)));
      addValue(settingsGroup, "Time Period of Step", QString::fromStdString(settings.procedure.at(10)));
      addValue(settingsGroup, "Minimum  Time Increment", QString::fromStdString(settings.procedure.at(11)));
      addValue(settingsGroup, "Maximum  Time Increment", QString::fromStdString(settings.procedure.at(12)));
      break;
    }
    case 8:
    {
      addValue(typeGroup, "Subtype", "Dynamic");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Direct", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "Alpha", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "Explicit", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "Realtive to Absolute", QString::fromStdString(settings.procedure.at(5)));
      addValue(settingsGroup, "Initial  Time Increment", QString::fromStdString(settings.procedure.at(6)));
      addValue(settingsGroup, "Time Period of Step", QString::fromStdString(settings.procedure.at(7)));
      addValue(settingsGroup, "Minimum  Time Increment", QString::fromStdString(settings.procedure.at(8)));
      addValue(settingsGroup, "Maximum  Time Increment", QString::fromStdString(settings.procedure.at(9)));
      addValue(settingsGroup, "Initial  Time Increment CFD", QString::fromStdString(settings.procedure.at(10)));
      break;
    }
    case 9:
    {
      addValue(typeGroup, "Subtype", "Modal dynamic");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Direct", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "DELTMX", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "Steady State", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "Initial  Time Increment", QString::fromStdString(settings.procedure.at(5)));
      addValue(settingsGroup, "Time Period of Step", QString::fromStdString(settings.procedure.at(6)));
      addValue(settingsGroup, "Minimum  Time Increment", QString::fromStdString(settings.procedure.at(7)));
      addValue(settingsGroup, "Maximum  Time Increment", QString::fromStdString(settings.procedure.at(8)));
      break;
    }
    case 10:
    {
      addValue(typeGroup, "Subtype", "Steady-state dynamics");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Solver", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Harmonic", QString::fromStdString(settings.procedure.at(2)));
      addValue(settingsGroup, "lower Bound Frequency", QString::fromStdString(settings.procedure.at(3)));
      addValue(settingsGroup, "upper Bound Frequency", QString::fromStdString(settings.procedure.at(4)));
      addValue(settingsGroup, "Data Points", QString::fromStdString(settings.procedure.at(5)));
      addValue(settingsGroup, "Bias", QString::fromStdString(settings.procedure.at(6)));
      addValue(settingsGroup, "Fourier", QString::fromStdString(settings.procedure.at(7)));
      addValue(settingsGroup, "lower Bound Time", QString::fromStdString(settings.procedure.at(8)));
      addValue(settingsGroup, "upper Bound Time", QString::fromStdString(settings.procedure.at(9)));
      break;
    }
    case 11:
    {
      addValue(typeGroup, "Subtype", "Complex frequency");
      auto* settingsGroup = addGroup("procedure", "Procedure", PropertySection::Data);
      addValue(settingsGroup, "Coriolis", QString::fromStdString(settings.procedure.at(1)));
      addValue(settingsGroup, "Eigenfrequencies", QString::fromStdString(settings.procedure.at(2)));
      break;
    }
    default:
      PRINT_ERROR("Unknown stored property subtype.\n");
      clear();
      return false;
  }
  return true;
}

void StepsPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXStepsInterface core;
  StepsSettings settings;
  std::string error;
  if (!core.get_step_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify step %1 parameter name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setStep(id); });
}
