#include "ResourceStatusWidget.hpp"
#include "FrameworkOptionsPanel.hpp"
#include "ResourceUsageButton.hpp"
#include "Framework/Core/Monitoring/ResourceAttribution.hpp"
#include "Framework/Core/Monitoring/FrameworkUserOptions.hpp"
#include "Framework/Core/ModuleManager.hpp"
#include <QDialog>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QStatusBar>
#include <QTimer>
#include <QToolButton>
#include <QTreeWidget>
#include <QVBoxLayout>
#include <algorithm>
#include <exception>
#include <iterator>

namespace
{
QString memory(const std::optional<std::uint64_t>& bytes)
{
  return bytes ? QString::number(static_cast<double>(*bytes) / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GiB" : QString(QChar(0x2014));
}

QString percent(const std::optional<double>& value)
{
  return value ? QString::number(*value, 'f', 0) + "%" : QString(QChar(0x2014));
}

QString stateName(JobRunState state)
{
  switch (state)
  {
    case JobRunState::NotStarted: return QObject::tr("Not started");
    case JobRunState::Running: return QObject::tr("Running");
    case JobRunState::Finished: return QObject::tr("Finished");
    case JobRunState::Killed: return QObject::tr("Killed");
    case JobRunState::Failed: return QObject::tr("Failed");
    case JobRunState::Waiting: return QObject::tr("Waiting");
    default: return QObject::tr("Unknown");
  }
}
}

ResourceStatusWidget::ResourceStatusWidget(QStatusBar* statusBar, ModuleManager& modules, FrameworkUserOptions& preferences)
  : QWidget(statusBar), modules(modules), preferences(preferences)
{
  setObjectName("CubitSim.ResourceStatus");
  setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(4, 0, 4, 0);
  cpuIndicator = new ResourceUsageButton(this);
  memoryIndicator = new ResourceUsageButton(this);
  jobsButton = new QToolButton(this);
  jobsButton->setAutoRaise(true);
  layout->addWidget(cpuIndicator);
  layout->addWidget(memoryIndicator);
  layout->addWidget(jobsButton);
  for (auto* button : {static_cast<QAbstractButton*>(cpuIndicator), static_cast<QAbstractButton*>(memoryIndicator), static_cast<QAbstractButton*>(jobsButton)})
  {
    connect(button, &QAbstractButton::clicked, this, [this]()
    {
      show_details();
    });
  }
  optionsPanel = new FrameworkOptionsPanel(this);
  optionsPanel->hide();
  timer = new QTimer(this);
  timer->setInterval(200);
  connect(timer, &QTimer::timeout, this, [this]()
  {
    tick();
  });
  // Append after Cubit's native progress control, keeping resources on the right
  // of the same status row without replacing or reparenting the native control.
  statusBar->addPermanentWidget(this, 0);
  apply_options();
}

ResourceStatusWidget::~ResourceStatusWidget()
{
  timer->stop();
  monitor.reset();
  delete details.data();
  delete optionsPanel.data();
}

WorkspaceView ResourceStatusWidget::options_page() const
{
  WorkspaceView page;
  page.label = "Cubit-Sim";
  page.optionsPage = optionsPanel;
  page.refreshOptions = [panel = optionsPanel]()
  {
    if (panel != nullptr)
    {
      panel->refresh_settings();
    }
  };
  page.applyOptions = [panel = optionsPanel](std::string& error)
  {
    if (panel == nullptr)
    {
      error = "Framework options page is unavailable.";
      return false;
    }
    return panel->save_settings(error);
  };
  return page;
}

void ResourceStatusWidget::apply_options()
{
  timer->stop();
  monitor.reset();
  latest.reset();
  lastRequest.invalidate();
  if (detailsTree != nullptr)
  {
    detailsTree->clear();
  }
  const auto& options = preferences.current();
  const bool enabled = options.monitorEnabled && (options.showCpu || options.showMemory || options.showJobs);
  cpuIndicator->setVisible(options.showCpu);
  memoryIndicator->setVisible(options.showMemory);
  jobsButton->setVisible(options.showJobs);
  setVisible(enabled);
  if (details != nullptr)
  {
    details->hide();
  }
  if (enabled)
  {
    monitor = std::make_unique<ResourceMonitor>();
    clear_indicators(tr("Sampling resources..."));
    timer->start();
    tick();
  }
}

void ResourceStatusWidget::tick()
{
  if (!monitor)
  {
    return;
  }
  try
  {
    const auto maxAge = std::chrono::seconds((std::max)(10, preferences.current().intervalSeconds * 3));
    auto completed = monitor->take_snapshot();
    if (completed && std::chrono::steady_clock::now() - completed->sampledAt <= maxAge)
    {
      latest = std::move(completed);
      display_snapshot();
    }
    if (latest && std::chrono::steady_clock::now() - latest->sampledAt > maxAge)
    {
      latest.reset();
      clear_indicators(tr("Resources awaiting update..."));
      if (detailsTree != nullptr)
      {
        detailsTree->clear();
      }
    }
    if (lastRequest.isValid() && lastRequest.elapsed() < preferences.current().intervalSeconds * 1000)
    {
      return;
    }
    std::vector<JobMonitorEntry> jobs;
    std::vector<std::string> errors;
    if (preferences.current().showJobs || preferences.current().showJobAttribution)
    {
      for (auto* provider : modules.job_monitor_providers())
      {
        std::vector<JobMonitorEntry> entries;
        std::string error;
        if (provider->snapshot(entries, error))
        {
          jobs.insert(jobs.end(), std::make_move_iterator(entries.begin()), std::make_move_iterator(entries.end()));
        }
        else
        {
          errors.push_back(error.empty() ? "Job provider unavailable" : error);
        }
      }
    }
    if (monitor->request(std::move(jobs), std::move(errors)))
    {
      lastRequest.start();
    }
  }
  catch (const std::exception&)
  {
    latest.reset();
    clear_indicators(tr("Resource monitoring unavailable"));
    if (detailsTree != nullptr)
    {
      detailsTree->clear();
    }
    lastRequest.start();
  }
}

void ResourceStatusWidget::display_snapshot()
{
  if (!latest)
  {
    return;
  }
  const auto& options = preferences.current();
  const auto cpu = cpu_attribution(*latest, options.showCubit, options.showJobAttribution);
  const auto ram = memory_attribution(*latest, options.showCubit, options.showJobAttribution);
  const auto segments = [&options](const ResourceAttribution& value)
  {
    if (!value.available)
    {
      return std::vector<ResourceBarSegment>{};
    }
    return std::vector<ResourceBarSegment>{
      {value.cubit, QColor::fromRgb(options.cubitColorRgb)},
      {value.jobs, QColor::fromRgb(options.jobsColorRgb)},
      {value.otherShared, QColor::fromRgb(options.systemColorRgb)}
    };
  };
  cpuIndicator->set_usage(tr("CPU %1").arg(percent(latest->machine.cpuPercent)), segments(cpu));
  memoryIndicator->set_usage(tr("RAM %1 / %2").arg(memory(latest->machine.physicalUsedBytes), memory(latest->machine.physicalTotalBytes)), segments(ram));
  const auto tooltip = [this](const ResourceAttribution& value, bool cpu)
  {
    const auto format = [cpu](const std::optional<double>& measured)
    {
      if (!measured)
      {
        return QString(QChar(0x2014));
      }
      return cpu ? QString::number(*measured, 'f', 1) + "%"
        : QString::number(*measured / (1024.0 * 1024.0 * 1024.0), 'f', 1) + " GiB";
    };
    const QString jobs = format(std::optional<double>(value.jobsMeasured)) +
      (value.jobsIncomplete ? tr(" known; some measurements unavailable") : QString());
    QString text = tr("Cubit: %1\nSolver jobs: %2\nColors: Cubit, solver jobs, then remaining / shared usage.\nAttribution is approximate; these are original, unscaled measurements.\nClick for individual jobs.")
      .arg(format(value.cubitMeasured), jobs);
    if (cpu)
    {
      text += tr("\nCPU uses total machine capacity. Sampling intervals may differ.");
    }
    else
    {
      text += tr("\nProcess memory can count shared pages repeatedly.");
    }
    if (value.adjusted)
    {
      text += tr("\nColored widths were reduced proportionally to fit measured machine usage.");
    }
    if (value.incomplete)
    {
      text += tr("\nSome attribution is unavailable; remaining usage includes unknown usage.");
    }
    if (!value.available)
    {
      text += tr("\nMachine usage is unavailable; the background is empty.");
    }
    return text;
  };
  cpuIndicator->setToolTip(tooltip(cpu, true));
  memoryIndicator->setToolTip(tooltip(ram, false));
  const auto count = std::count_if(latest->jobs.begin(), latest->jobs.end(), [](const JobResourceSample& job)
  {
    return job.job.state == JobRunState::Running || job.job.state == JobRunState::Waiting;
  });
  jobsButton->setText(latest->providerErrors.empty() ? tr("Jobs %1").arg(static_cast<qlonglong>(count)) : tr("Jobs unavailable"));
  jobsButton->setToolTip(tr("Show machine and individual solver-process measurements."));
  if (detailsTree != nullptr)
  {
    detailsTree->clear();
    new QTreeWidgetItem(detailsTree, {tr("Machine"), QString(), QString(), percent(latest->machine.cpuPercent),
      memory(latest->machine.physicalUsedBytes) + " / " + memory(latest->machine.physicalTotalBytes)});
    new QTreeWidgetItem(detailsTree, {tr("Cubit"), QString(), QString(), percent(latest->cubit.cpuPercent), memory(latest->cubit.residentBytes)});
    for (const auto& job : latest->jobs)
    {
      new QTreeWidgetItem(detailsTree, {QString::fromStdString(job.job.commandPrefix),
        QString::number(job.job.jobId) + " - " + QString::fromUtf8(job.job.name.c_str()), stateName(job.job.state),
        percent(job.usage.cpuPercent), memory(job.usage.residentBytes)});
    }
    for (const auto& error : latest->providerErrors)
    {
      new QTreeWidgetItem(detailsTree, {tr("Unavailable"), QString::fromUtf8(error.c_str())});
    }
  }
}

void ResourceStatusWidget::clear_indicators(const QString& reason)
{
  const auto dash = QString(QChar(0x2014));
  cpuIndicator->set_usage(tr("CPU %1").arg(dash), {});
  memoryIndicator->set_usage(tr("RAM %1").arg(dash), {});
  jobsButton->setText(tr("Jobs %1").arg(dash));
  cpuIndicator->setToolTip(reason);
  memoryIndicator->setToolTip(reason);
  jobsButton->setToolTip(reason);
}

void ResourceStatusWidget::show_details()
{
  if (details == nullptr)
  {
    details = new QDialog(this);
    details->setWindowTitle(tr("Machine and solver resources"));
    details->resize(760, 340);
    auto* layout = new QVBoxLayout(details);
    detailsTree = new QTreeWidget(details);
    detailsTree->setHeaderLabels({tr("Application"), tr("Job"), tr("State"), tr("CPU"), tr("Physical memory")});
    detailsTree->setRootIsDecorated(false);
    detailsTree->header()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layout->addWidget(detailsTree);
    detailsTree->setToolTip(tr("CPU uses total machine capacity. Solver usage excludes child processes and conversion. Process memory includes shared pages; do not sum it as machine usage. A dash means unavailable or awaiting a second sample."));
  }
  display_snapshot();
  details->show();
  details->raise();
  details->activateWindow();
}
