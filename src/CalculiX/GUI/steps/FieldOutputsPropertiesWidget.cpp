#include "FieldOutputsPropertiesWidget.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitMessage.hpp"

FieldOutputsPropertiesWidget::FieldOutputsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool FieldOutputsPropertiesWidget::setFieldOutput(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXStepsInterface core;
  FieldOutputsSettings settings;
  std::string error;
  if (!core.get_fieldoutput_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load FieldOutput properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Field Output", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* typeGroup = addGroup("classification", "Classification", PropertySection::Classification);
  switch (settings.type)
  {
    case 1:
    {
      addValue(typeGroup, "Subtype", "Node");
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addReference(settingsGroup, PropertyReferenceKind::Nodeset, "Nodeset", QString::fromStdString(settings.parameters.at(1)));
      addValue(settingsGroup, "Frequency", QString::fromStdString(settings.parameters.at(2)));
      addValue(settingsGroup, "FrequencyF", QString::fromStdString(settings.parameters.at(3)));
      addValue(settingsGroup, "Totals", QString::fromStdString(settings.parameters.at(4)));
      addValue(settingsGroup, "Global", QString::fromStdString(settings.parameters.at(5)));
      addValue(settingsGroup, "Output", QString::fromStdString(settings.parameters.at(6)));
      addValue(settingsGroup, "Output All", QString::fromStdString(settings.parameters.at(7)));
      addValue(settingsGroup, "Last Iterations", QString::fromStdString(settings.parameters.at(9)));
      addValue(settingsGroup, "Contact Elements", QString::fromStdString(settings.parameters.at(10)));
      break;
    }
    case 2:
    {
      addValue(typeGroup, "Subtype", "Element");
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addReference(settingsGroup, PropertyReferenceKind::Nodeset, "Nodeset", QString::fromStdString(settings.parameters.at(1)));
      addValue(settingsGroup, "Frequency", QString::fromStdString(settings.parameters.at(2)));
      addValue(settingsGroup, "FrequencyF", QString::fromStdString(settings.parameters.at(3)));
      addValue(settingsGroup, "Section Forces", QString::fromStdString(settings.parameters.at(7)));
      addValue(settingsGroup, "Global", QString::fromStdString(settings.parameters.at(4)));
      addValue(settingsGroup, "Output", QString::fromStdString(settings.parameters.at(5)));
      addValue(settingsGroup, "Output All", QString::fromStdString(settings.parameters.at(6)));
      addValue(settingsGroup, "Last Iterations", QString::fromStdString(settings.parameters.at(9)));
      addValue(settingsGroup, "Contact Elements", QString::fromStdString(settings.parameters.at(10)));
      break;
    }
    case 3:
    {
      addValue(typeGroup, "Subtype", "Contact");
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addValue(settingsGroup, "Frequency", QString::fromStdString(settings.parameters.at(1)));
      addValue(settingsGroup, "Last Iterations", QString::fromStdString(settings.parameters.at(3)));
      addValue(settingsGroup, "Contact Elements", QString::fromStdString(settings.parameters.at(4)));
      break;
    }
    default:
      PRINT_ERROR("Unknown stored property subtype.\n");
      clear();
      return false;
  }
  auto* output = addGroup("output", "Output", PropertySection::Data);
  switch (settings.type)
  {
    case 1: addValue(output, "Time points", QString::fromStdString(settings.parameters.at(8))); break;
    case 2: addValue(output, "Time points", QString::fromStdString(settings.parameters.at(8))); break;
    case 3: addValue(output, "Time points", QString::fromStdString(settings.parameters.at(2))); break;
    default: break;
  }
  auto* keys = addGroup("enabled_output_keys", "Enabled output keys", PropertySection::Data);
  for (const auto& key : settings.enabledKeys)
  {
    addValue(keys, QString::fromStdString(key), "Yes");
  }
  if (settings.enabledKeys.empty())
  {
    addValue(keys, "Keys", "None");
  }
  return true;
}

void FieldOutputsPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXStepsInterface core;
  FieldOutputsSettings settings;
  std::string error;
  if (!core.get_fieldoutput_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  QString subtype;
  switch (settings.type)
  {
    case 1: subtype = "node"; break;
    case 2: subtype = "element"; break;
    case 3: subtype = "contact"; break;
    default: return;
  }
  const QString command = QString("ccx modify fieldoutput %1 %2 name %3").arg(request.entityId).arg(subtype).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setFieldOutput(id); });
}
