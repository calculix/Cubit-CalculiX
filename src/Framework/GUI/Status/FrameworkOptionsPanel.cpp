#include "Framework/Commands/UserOptionCommandSupport.hpp"
#include "FrameworkOptionsPanel.hpp"
#include "Framework/Core/FrameworkContext.hpp"
#include "Framework/GUI/CommandDispatcher.hpp"
#include "Claro.hpp"
#include <QCheckBox>
#include <QColorDialog>
#include <QPixmap>
#include <QIcon>
#include <QVariant>
#include <QPushButton>
#include <QFormLayout>
#include <QSpinBox>
#include <QVBoxLayout>

namespace
{
void setColor(QPushButton* button, const QColor& color)
{
  button->setText(color.name(QColor::HexRgb).toUpper());
  button->setProperty("rgb", static_cast<uint>(color.rgb() & 0x00ffffff));
  QPixmap swatch(20, 16);
  swatch.fill(color);
  button->setIcon(QIcon(swatch));
}
}

FrameworkOptionsPanel::FrameworkOptionsPanel(QWidget* parent) : QWidget(parent)
{
  auto* layout = new QVBoxLayout(this);
  auto* form = new QFormLayout;
  enabled = new QCheckBox(tr("Show machine resources at the bottom"), this);
  interval = new QSpinBox(this);
  interval->setRange(1, 60);
  interval->setSuffix(tr(" s"));
  interval->setToolTip(tr("Changes take effect immediately after Apply."));
  cpu = new QCheckBox(tr("Machine CPU"), this);
  memory = new QCheckBox(tr("Machine RAM"), this);
  cubit = new QCheckBox(tr("Show Cubit attribution in CPU and RAM"), this);
  jobs = new QCheckBox(tr("Show Jobs button"), this);
  jobAttribution = new QCheckBox(tr("Show solver-job attribution in CPU and RAM"), this);
  systemColor = new QPushButton(this);
  cubitColor = new QPushButton(this);
  jobsColor = new QPushButton(this);
  for (auto* button : {systemColor, cubitColor, jobsColor})
  {
    connect(button, &QPushButton::clicked, this, [this, button]()
    {
      const auto color = QColorDialog::getColor(QColor::fromRgb(button->property("rgb").toUInt()), this, tr("Choose resource color"));
      if (color.isValid())
      {
        setColor(button, color);
      }
    });
  }
  form->addRow(enabled);
  form->addRow(tr("Refresh interval"), interval);
  form->addRow(cpu);
  form->addRow(memory);
  form->addRow(cubit);
  form->addRow(jobs);
  form->addRow(jobAttribution);
  form->addRow(tr("System / other usage color"), systemColor);
  form->addRow(tr("Cubit color"), cubitColor);
  form->addRow(tr("Solver jobs color"), jobsColor);
  layout->addLayout(form);
  layout->addStretch();
  refresh_settings();
}

void FrameworkOptionsPanel::refresh_settings()
{
  const auto options = framework_context().user_options();
  enabled->setChecked(options.monitorEnabled);
  interval->setValue(options.intervalSeconds);
  cpu->setChecked(options.showCpu);
  memory->setChecked(options.showMemory);
  cubit->setChecked(options.showCubit);
  jobs->setChecked(options.showJobs);
  jobAttribution->setChecked(options.showJobAttribution);
  setColor(systemColor, QColor::fromRgb(options.systemColorRgb));
  setColor(cubitColor, QColor::fromRgb(options.cubitColorRgb));
  setColor(jobsColor, QColor::fromRgb(options.jobsColorRgb));
  baseline = option_values();
}

QJsonObject FrameworkOptionsPanel::option_values() const
{
  QJsonObject values;
  values["monitor/enabled"] = enabled->isChecked();
  values["monitor/interval_seconds"] = interval->value();
  values["monitor/show_cpu"] = cpu->isChecked();
  values["monitor/show_memory"] = memory->isChecked();
  values["monitor/show_cubit"] = cubit->isChecked();
  values["monitor/show_jobs"] = jobs->isChecked();
  values["monitor/show_job_attribution"] = jobAttribution->isChecked();
  values["monitor/system_color"] = systemColor->text();
  values["monitor/cubit_color"] = cubitColor->text();
  values["monitor/jobs_color"] = jobsColor->text();
  return values;
}

bool FrameworkOptionsPanel::save_settings(std::string& error)
{
  error.clear();
  const auto edited = option_values();
  std::vector<std::pair<std::string, std::string>> commands;
  if (!UserOptionCommandSupport::edited_commands("sim useroptions", framework_context().user_option_definitions(), edited, baseline, commands, error))
  {
    return false;
  }
  for (const auto& command : commands)
  {
    if (!CommandDispatcher::submit(Claro::instance(), QString::fromUtf8(command.second.c_str()), error))
    {
      return false;
    }
    const auto key = QString::fromStdString(command.first);
    baseline[key] = edited.value(key);
  }
  refresh_settings();
  return true;
}
