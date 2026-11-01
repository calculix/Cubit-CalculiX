#include "FEAComp.hpp"
#include "Framework/Core/FrameworkContext.hpp"
#include "Claro.hpp"
#include "CubitMessage.hpp"
#include "Framework/Utility/StartupTrace.hpp"

#include <QPlainTextEdit>
#include <QTextCursor>

namespace
{
bool removeStartupText(QWidget* host, const QString& text)
{
  if (host == nullptr || text.isEmpty())
  {
    return false;
  }

  auto* window = host->findChild<QPlainTextEdit*>("mCommandWindow");

  if (window == nullptr)
  {
    return false;
  }

  const auto position = window->toPlainText().lastIndexOf(text);

  if (position < 0)
  {
    return false;
  }

  QTextCursor cursor(window->document());
  cursor.setPosition(static_cast<int>(position));
  cursor.setPosition(static_cast<int>(position + text.size()), QTextCursor::KeepAnchor);
  cursor.removeSelectedText();

  return true;
}
}

FEAComp::FEAComp()
  : Component("fea_comp")
{
}

FEAComp::~FEAComp()
{
  release_framework();
}

void FEAComp::start_up(int withGUI)
{
  startup_trace("FEAComp startup withGUI=" + QString::number(withGUI));
  if (acquired)
  {
    return;
  }

  auto* host = withGUI > 0 ? Claro::instance() : nullptr;

  removeStartupText(host, QStringLiteral("\"fea_comp\" has been loaded."));

  std::string error;
  auto& context = framework_context();

  if (!context.has_command_plugin())
  {
    PRINT_ERROR("Load fea_plugin from CUBIT_PLUGIN_DIR before starting FEAComp.\n");
    return;
  }

  if (!context.acquire(error))
  {
    PRINT_ERROR("FEA framework initialization failed: %s\n", error.c_str());
    return;
  }

  acquired = true;
  startup_trace("Framework acquired");

  if (withGUI > 0)
  {
    if (!context.attach_workspace_host(host, error))
    {
      PRINT_ERROR("Workspace host initialization failed: %s\n", error.c_str());
      release_framework();
      return;
    }

    owns_workspace_host = true;
    startup_trace("Workspace host attached");
  }

  if (!context.module_manager().start_components(withGUI, host, error))
  {
    PRINT_ERROR("FEA framework component startup failed: %s\n", error.c_str());
    release_framework();
    return;
  }

  owns_components = true;
  startup_trace("Components started");

  if (withGUI > 0 && !context.register_workspaces(error))
  {
    PRINT_ERROR("Workspace registration failed: %s\n", error.c_str());
    release_framework();
    return;
  }
  startup_trace("FEAComp startup complete");
}

void FEAComp::clean_up()
{
  release_framework();
  clean_up_complete();
}

void FEAComp::clean_up_complete()
{
  // Matches the current CalculiX component's loader-facing method.
}

void FEAComp::interrupt_progress()
{
  // Operation cancellation remains owned by the individual modules.
}

void FEAComp::release_framework() noexcept
{
  startup_trace("FEAComp release");
  if (!acquired)
  {
    return;
  }

  auto& context = framework_context();

  if (owns_workspace_host)
  {
    context.detach_workspace_host();
    owns_workspace_host = false;
  }

  if (owns_components)
  {
    context.module_manager().stop_components();
    owns_components = false;
  }

  context.release();
  acquired = false;
}
