#include "PostprocessorWorkspace.hpp"
#include "Postprocessor/GUI/Explosion/PostprocessorExplosionPlayback.hpp"
#include "Postprocessor/GUI/Navigation/PostprocessorFrameNavigation.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotPane.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorViewport.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "CubitMessage.hpp"
#include <QScopedValueRollback>
#include <algorithm>
#include <cctype>

void PostprocessorWorkspace::submit(const QString& command) {
  const QScopedValueRollback<bool> guard(foregroundCommand,true);
  if (explosionPlayback)
  {
    explosionPlayback->pause_clock();
  }
  viewport->cancel_interaction();
  plotPane->cancel();
  std::string error;

  if (!submitCommand || !submitCommand(command.toUtf8().toStdString(), error)) {
    PRINT_ERROR("Cannot submit Postprocessor command: %s\n", error.c_str());
  }
}

QString PostprocessorWorkspace::frame_command(std::size_t frame) const
{
  const auto active = application.lock();
  if (!active) {
    return {};
  }
  const auto& state = active->session();
  const auto source = state.sources.find(state.active_source_id);
  std::optional<std::size_t> current;
  for (const auto& entry : state.representations) {
    const auto& display = entry.second;
    if (active->display_matches_source(display,state.active_source_id,state.active_view_id)) {
      current = display.increment_index;
      break;
    }
  }
  const auto view = state.views.find(state.active_view_id);
  if (view != state.views.end() && view->second.table && view->second.table->source_id == state.active_source_id) {
    current = view->second.table->frame;
  }
  if (source == state.sources.end() || !navigation->matches_context(state.active_source_id+":"+state.active_view_id,
      active->result_data(state.active_source_id),current)) {
    return {};
  }
  const auto name = QString::fromUtf8(source->second.name.c_str());
  QChar delimiter('"');
  if (name.contains(delimiter)) {
    delimiter = QChar(0x27);
  }
  if (name.isEmpty() || name.contains(delimiter)) {
    return {};
  }
  for (QChar c : name) {
    if (c.unicode() < 32 || c.unicode() == 127) {
      return {};
    }
  }
  return "post frame " + QString::number(static_cast<qulonglong>(frame)) + " source " + delimiter + name + delimiter;
}

void PostprocessorWorkspace::source_command(const QString& verb, const QString& name) {
  // Match Cubit's quoted-string syntax without allowing a name to inject a
  // second command. Do not reach into solver-specific GUI quoting utilities.
  QChar delimiter('"');
  if (name.contains(delimiter)) {
    delimiter = QChar(0x27);
  }

  if (name.isEmpty() || name.contains(delimiter)) {
    PRINT_ERROR("The source name cannot be represented as one quoted command argument.\n");
    return;
  }

  for (const auto character : name) {
    if (character.unicode() < 0x20 || character.unicode() == 0x7f) {
      PRINT_ERROR("A source command requires a name without control characters.\n");
      return;
    }
  }

  submit("post " + verb + " " + delimiter + name + delimiter);
}

void PostprocessorWorkspace::dataset_command(const QString& name, Postprocessor::DatasetVisibilityAction action,
  std::optional<std::size_t> dataset) {
  const auto quote = [](const QString& text) -> QString
  {
    QChar delimiter('"');
    if (text.contains(delimiter)) {
      delimiter = QChar(0x27);
    }
    if (text.isEmpty() || text.contains(delimiter)) {
      return {};
    }
    for (const auto character : text) {
      if (character.unicode() < 32 || character.unicode() == 127) {
        return {};
      }
    }
    return delimiter+text+delimiter;
  };
  const auto source = quote(name);
  const auto active = application.lock();
  if (!active || source.isEmpty()) {
    PRINT_ERROR("Dataset visibility requires a valid source name.\n");
    return;
  }
  std::string sourceId;
  for (const auto& entry : active->session().sources) {
    if (QString::fromUtf8(entry.second.name.c_str()) == name) {
      sourceId = entry.first;
      break;
    }
  }
  using Action = Postprocessor::DatasetVisibilityAction;
  QString selector = "all";
  if (action != Action::ShowAll) {
    if (!dataset) {
      return;
    }
    const auto catalogue = active->mesh_datasets(sourceId);
    const auto found = std::find_if(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value) {
      return value.id == *dataset;
    });
    if (found == catalogue.end()) {
      PRINT_ERROR("The selected mesh dataset is no longer available.\n");
      return;
    }
    selector = QString::number(static_cast<qulonglong>(*dataset));
    const auto label = quote(QString::fromUtf8(found->name.c_str()));
    if (!label.isEmpty() && std::count_if(catalogue.begin(),catalogue.end(),[&](const SimResults::Dataset& value) {
          return value.name == found->name;
        }) == 1) {
      selector = label;
    }
  }
  QString operation = "show dataset ";
  if (action == Action::Hide) {
    operation = "hide dataset ";
  } else if (action == Action::ShowOnly) {
    operation = "show only dataset ";
  }
  submit("post "+operation+selector+" source "+source);
}

void PostprocessorWorkspace::camera_tool(const QString& command)
{
  const auto app = this->application.lock();
  if (!app)
  {
    return;
  }
  if (app->user_options().journalCamera)
  {
    this->submit(command);
  }
  else if (silentCommand)
  {
    std::string error;
    if (!silentCommand(command.toUtf8().toStdString(),error))
    {
      PRINT_ERROR("Cannot change standard view: %s\n",error.c_str());
    }
    viewport->request_redraw();
  }
}
