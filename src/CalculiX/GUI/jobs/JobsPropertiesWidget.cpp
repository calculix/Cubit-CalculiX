#include "JobsPropertiesWidget.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "CubitMessage.hpp"

JobsPropertiesWidget::JobsPropertiesWidget(QWidget* parent)
  : PropertyTreeWidget(parent)
{
}

bool JobsPropertiesWidget::setJob(int id)
{
  clear();
  currentEntityId = -1;
  CalculiXJobsInterface core;
  JobsSettings settings;
  std::string error;
  if (!core.get_job_settings(id, settings, error))
  {
    PRINT_ERROR("Cannot load Job properties %d: %s\n", id, error.c_str());
    return false;
  }
  currentEntityId = id;
  auto* general = addIdentity("Job", settings.id);
  addEditableName(general, id, QString::fromStdString(settings.name));
  auto* settingsGroup = addGroup("settings", "Settings", PropertySection::Settings);
  addValue(settingsGroup, "Filepath", QString::fromStdString(settings.filepath));
  QString status;
  switch (core.get_job_status(id))
  {
    case -1: status = "Not started"; break;
    case 1: status = "Running"; break;
    case 2: status = "Finished"; break;
    case 3: status = "Killed"; break;
    case 4: status = "Failed"; break;
    case 5: status = "Waiting"; break;
    default: status = "Unknown"; break;
  }
  addValue(addGroup("execution", "Execution", PropertySection::Settings), "Status", status);
  return true;
}

void JobsPropertiesWidget::applyEdit(const PropertyEditRequest& request)
{
  QString quotedName;
  if (!prepareNameEdit(request, currentEntityId, quotedName))
  {
    return;
  }
  CalculiXJobsInterface core;
  JobsSettings settings;
  std::string error;
  if (!core.get_job_settings(request.entityId, settings, error))
  {
    PRINT_ERROR("Cannot reload the selected entity for renaming: %s\n", error.c_str());
    return;
  }
  const QString command = QString("ccx modify job %1 name %2").arg(request.entityId).arg(quotedName);
  submitNameCommand(request, QString::fromStdString(settings.name), command, [this, id = request.entityId]() { return setJob(id); });
}
