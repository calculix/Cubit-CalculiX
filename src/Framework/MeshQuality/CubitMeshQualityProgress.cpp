#include "CubitMeshQualityProgress.hpp"
#include "cubit/CubitInterface.hpp"
#include "Util/AppUtil.hpp"
#include "Util/ProgressTool.hpp"
#include "Util/CubitMessage.hpp"
#include <QCoreApplication>
#include <QThread>
#include <exception>
CubitMeshQualityProgress::CubitMeshQualityProgress() : lastLog(std::chrono::steady_clock::now())
{
  auto* application = QCoreApplication::instance();
  if (application && QThread::currentThread() == application->thread())
  {
    if (auto* host = CubitInterface::app_util().get())
    {
      tool = host->progress_tool();
    }
  }
}
CubitMeshQualityProgress::~CubitMeshQualityProgress()
{
  if (tool && started)
  {
    try
    {
      tool->end();
    }
    catch (...)
    {
      // Native progress cleanup must not interrupt job archive destruction.
    }
  }
}
bool CubitMeshQualityProgress::update(ReportProgressStage, const std::string& stage,
  std::size_t completed, std::size_t total, std::string& error)
{
  if (total != 0 && completed > total)
  {
    error = "Invalid quality archive progress count.";
    return false;
  }
  if (!tool)
  {
    return true;
  }
  try
  {
    if (!started)
    {
      started = true;
      tool->start(0, 0, "Mesh quality", "Archiving native mesh quality", true);
    }
    const auto now = std::chrono::steady_clock::now();
    if (stage != lastStage && (lastStage.empty() || now - lastLog >= std::chrono::milliseconds(200)))
    {
      PRINT_INFO("Mesh quality: %s\n", stage.c_str());
      lastStage = stage;
      lastLog = now;
    }
    tool->check_interrupt();
    if (auto* host = CubitInterface::app_util().get())
    {
      if (host->interrupt())
      {
        error = "Mesh quality archive interrupted.";
        return false;
      }
    }
  }
  catch (const std::exception& exception)
  {
    error = std::string("Quality archive progress failed: ") + exception.what();
    return false;
  }
  catch (...)
  {
    error = "Quality archive progress interrupted.";
    return false;
  }
  return true;
}
