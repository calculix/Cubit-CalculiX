#include "LoadsFilmPropertiesWidget.hpp"
#include "Core/loads/CalculiXLoadsInterface.hpp"
#include "CubitMessage.hpp"

LoadsFilmPropertiesWidget::LoadsFilmPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool LoadsFilmPropertiesWidget::setFilm(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXLoadsInterface core;
  LoadsFilmSettings settings;
  std::string error;
  if (!core.get_film_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Film properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Film", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* assignment = addGroup("assignment", "Assignment", PropertySection::Assignment);
  addReference(assignment, PropertyReferenceKind::Sideset, "Sideset", QString::fromStdString(settings.sideset));
  auto* components = addGroup("components", "Components", PropertySection::Data);
  addValue(components, "Temperature", QString::fromStdString(settings.temperature));
  addValue(components, "Coefficient", QString::fromStdString(settings.coefficient));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Amplitude", QString::fromStdString(settings.amplitude));
  addValue(settingsGroup, "Time Delay", QString::fromStdString(settings.timeDelay));
  addReference(settingsGroup, PropertyReferenceKind::Amplitude, "Film amplitude", QString::fromStdString(settings.secondaryAmplitude));
  addValue(settingsGroup, "Film Time Delay", QString::fromStdString(settings.secondaryTimeDelay));
  addValue(settingsGroup, "OP", QString::fromStdString(settings.operation));
  return true;
}

void LoadsFilmPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXLoadsInterface core;
  LoadsFilmSettings settings;
  std::string error;
  if (!core.get_film_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify film %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setFilm(id); });
}
