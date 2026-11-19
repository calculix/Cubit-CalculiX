#include "ReportNativeProgress.hpp"
#include "AppUtil.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"
#include <QCoreApplication>
#include <QThread>
#include <algorithm>
#include <exception>
#include <utility>

ReportNativeProgress::ReportNativeProgress(bool guiAvailable, ReportProgressStage first)
  : firstStage(first), lastUpdate(std::chrono::steady_clock::now())
{
  auto* application = QCoreApplication::instance();
  if (guiAvailable && application && QThread::currentThread() == application->thread())
  {
    if (auto* host = CubitInterface::app_util().get())
    {
      tool = host->progress_tool();
    }
  }
}

ReportNativeProgress::ReportNativeProgress(bool guiAvailable, const std::string& description,
  std::vector<ReportProgressStage> stages)
  : ReportNativeProgress(guiAvailable)
{
  information = description;
  phaseOrder = std::move(stages);
}

ReportNativeProgress::~ReportNativeProgress()
{
  close();
}

void ReportNativeProgress::close() noexcept
{
  if (started)
  {
    started = false;
    try
    {
      tool->end();
    }
    catch (...)
    {
      PRINT_WARNING("Cannot close the Report progress display.\n");
    }
  }
}

void ReportNativeProgress::finish() noexcept
{
  if (!started)
  {
    return;
  }
  // No cancellation after document/cache or export-file publication.
  try
  {
    tool->percent(1.0);
  }
  catch (...)
  {
    PRINT_WARNING("Cannot finish the Report progress display.\n");
  }
}

bool ReportNativeProgress::update(ReportProgressStage phase, const std::string& stage, std::size_t completed,
  std::size_t total, std::string& error)
{
  if (interrupted)
  {
    error = "Report operation interrupted.";
    return false;
  }
  if (!tool)
  {
    return true;
  }
  const auto now = std::chrono::steady_clock::now();
  const bool changed = stage != currentStage;
  if (!changed && completed != total && now - lastUpdate < std::chrono::milliseconds(200))
  {
    return true;
  }
  lastUpdate = now;
  try
  {
    if (!started)
    {
      if (information.empty())
      {
        information = "Generating report and preparing preview";
        if (firstStage == ReportProgressStage::PreviewContent)
        {
          information = "Preparing report preview";
        }
      }
      started = true;
      tool->start(0, 100, "Report", information.c_str(), true);
    }
    if (changed)
    {
      currentStage = stage;
      PRINT_INFO("Report: %s\n", stage.c_str());
    }
    double fraction = ReportProgress::fraction(phase, completed, total, firstStage);
    if (!phaseOrder.empty())
    {
      if (std::find(phaseOrder.begin(), phaseOrder.end(), phase) == phaseOrder.end())
      {
        error = "Report progress phase is not part of this operation.";
        return false;
      }
      fraction = ReportProgress::fraction(phase, completed, total, phaseOrder);
    }
    overall = (std::max)(overall, fraction);
    tool->percent(overall);
    tool->check_interrupt();
    if (auto* host = CubitInterface::app_util().get())
    {
      interrupted = host->interrupt();
    }
    if (!interrupted)
    {
      return true;
    }
    error = "Report operation interrupted during: " + stage;
  }
  catch (const std::exception& exception)
  {
    error = "Report progress failed: " + std::string(exception.what());
    interrupted = true;
  }
  catch (...)
  {
    error = "Report progress interrupted during: " + stage;
    interrupted = true;
  }
  return false;
}
