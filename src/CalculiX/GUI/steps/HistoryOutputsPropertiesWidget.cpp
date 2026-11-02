#include "HistoryOutputsPropertiesWidget.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "CubitMessage.hpp"

HistoryOutputsPropertiesWidget::HistoryOutputsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool HistoryOutputsPropertiesWidget::setHistoryOutput(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXStepsInterface core;
  HistoryOutputsSettings settings;
  std::string error;
  if (!core.get_historyoutput_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load HistoryOutput properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("History Output", settings.id);
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
      break;
    }
    case 2:
    {
      addValue(typeGroup, "Subtype", "Element");
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addReference(settingsGroup, PropertyReferenceKind::Block, "Block", QString::fromStdString(settings.parameters.at(1)));
      addValue(settingsGroup, "Frequency", QString::fromStdString(settings.parameters.at(2)));
      addValue(settingsGroup, "FrequencyF", QString::fromStdString(settings.parameters.at(3)));
      addValue(settingsGroup, "Totals", QString::fromStdString(settings.parameters.at(4)));
      addValue(settingsGroup, "Global", QString::fromStdString(settings.parameters.at(5)));
      break;
    }
    case 3:
    {
      addValue(typeGroup, "Subtype", "Contact");
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addReference(settingsGroup, PropertyReferenceKind::ContactPair, "Contact pair", QString::fromStdString(settings.parameters.at(1)));
      addValue(settingsGroup, "Frequency", QString::fromStdString(settings.parameters.at(2)));
      addValue(settingsGroup, "Totals", QString::fromStdString(settings.parameters.at(3)));
      addValue(settingsGroup, "Global", QString::fromStdString(settings.parameters.at(4)));
      break;
    }
    case 4:
    {
      addValue(typeGroup, "Subtype", "Section");
      auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
      addReference(settingsGroup, PropertyReferenceKind::Sideset, "Sideset", QString::fromStdString(settings.parameters.at(1)));
      addValue(settingsGroup, "FrequencyF", QString::fromStdString(settings.parameters.at(2)));
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
    case 1: addValue(output, "Time points", QString::fromStdString(settings.parameters.at(6))); break;
    case 2: addValue(output, "Time points", QString::fromStdString(settings.parameters.at(6))); break;
    case 3: addValue(output, "Time points", QString::fromStdString(settings.parameters.at(5))); break;
    case 4: addValue(output, "Time points", QString::fromStdString(settings.parameters.at(3))); break;
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

void HistoryOutputsPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXStepsInterface core;
  HistoryOutputsSettings settings;
  std::string error;
  if (!core.get_historyoutput_settings(request.entityId, settings, error))
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
    case 4: subtype = "section"; break;
    default: return;
  }
  const QString command = QString("ccx modify historyoutput %1 %2 name %3").arg(request.entityId).arg(subtype).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setHistoryOutput(id); });
}
