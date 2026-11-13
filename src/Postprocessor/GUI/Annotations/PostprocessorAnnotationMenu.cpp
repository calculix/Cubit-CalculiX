#include "PostprocessorAnnotationMenu.hpp"
#include "Postprocessor/GUI/Plots/PostprocessorPlotControls.hpp"
#include <utility>
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/selection/PickCommandText.hpp"
#include <QMenu>
#include <QInputDialog>
#include <QLineEdit>
#include <algorithm>

QString annotation_quote(const std::string& value)
{
  const char delimiter = value.find('"') == std::string::npos ? '"' : '\'';
  if (value.empty() || value.find(delimiter) != std::string::npos ||
      std::any_of(value.begin(),value.end(),[](unsigned char c)
      {
        return c < 32 || c == 127;
      }))
  {
    return {};
  }
  return QString::fromStdString(std::string(1,delimiter)+value+delimiter);
}
QString annotation_add_command(const PostprocessorApplicationInterface& application,
  const std::string& viewId, const Postprocessor::EntityReference& entity)
{
  const auto view = application.session().views.find(viewId);
  if (view == application.session().views.end())
  {
    return {};
  }
  std::string name;
  for (std::size_t number = 1; number <= view->second.annotations.labels.size()+1; ++number)
  {
    const auto candidate = "label_"+std::to_string(number);
    if (view->second.annotations.labels.count(candidate) == 0)
    {
      name = candidate;
      break;
    }
  }
  std::string pick, error;
  if (!Postprocessor::pick_command_text(application,entity,pick,error))
  {
    return {};
  }
  for (const auto& item : application.session().representations)
  {
    const auto& display = item.second;
    if (display.view_id == viewId && display.input.owner == Postprocessor::OutputOwner::Source &&
        display.input.object_id == entity.source_id && display.coloring.field)
    {
      const auto& field = *display.coloring.field;
      const auto fieldName = annotation_quote(field.group_key.empty() ? field.array_name : field.group_key);
      const auto component = field.magnitude ? QStringLiteral("magnitude") : annotation_quote(field.component);
      if (fieldName.isEmpty() || component.isEmpty() || annotation_quote(viewId).isEmpty())
      {
        return {};
      }
      return QStringLiteral("post label add ")+annotation_quote(name)+" "+QString::fromStdString(pick.substr(10))+
        " field "+fieldName+" component "+component+" view "+annotation_quote(viewId);
    }
  }
  return {};
}
void show_annotation_menu(QWidget* parent, const QPoint& globalPosition,
  const PostprocessorApplicationInterface& application, const std::string& viewId,
  const std::string& hit, const std::optional<Postprocessor::EntityReference>& entity,
  const std::function<void(const QString&)>& submit)
{
  const auto found = application.session().views.find(viewId);
  if (found == application.session().views.end())
  {
    return;
  }
  // Snapshot before entering a nested menu/dialog event loop.
  const auto state = found->second.annotations;
  const auto suffix = QStringLiteral(" view ")+annotation_quote(viewId);
  QMenu menu(parent);
  if (entity)
  {
    add_plot_context_actions(menu,application,*entity,submit);
    const auto command = annotation_add_command(application,viewId,*entity);
    auto* add = menu.addAction(QObject::tr("Create result label"));
    add->setEnabled(!command.isEmpty());
    QObject::connect(add,&QAction::triggered,&menu,[submit,command]()
    {
      submit(command);
    });
  }
  auto* extrema = menu.addAction(QObject::tr("Show minimum / maximum"));
  extrema->setCheckable(true);
  extrema->setChecked(state.extrema_visible);
  QObject::connect(extrema,&QAction::triggered,&menu,[submit,suffix](bool checked)
  {
    submit(QStringLiteral("post labels extrema ")+(checked ? "on" : "off")+suffix);
  });
  auto* visible = menu.addAction(QObject::tr("Show labels"));
  visible->setCheckable(true);
  visible->setChecked(state.visible);
  QObject::connect(visible,&QAction::triggered,&menu,[submit,suffix](bool checked)
  {
    submit(QStringLiteral("post labels ")+(checked ? "show" : "hide")+suffix);
  });
  auto* labels = menu.addMenu(QObject::tr("Result labels"));
  labels->setEnabled(!state.labels.empty());
  for (const auto& entry : state.labels)
  {
    const auto name = entry.first;
    const auto quoted = annotation_quote(name);
    auto* label = labels->addMenu(QString::fromUtf8(name.c_str()));
    if (name == hit)
    {
      auto font = label->menuAction()->font();
      font.setBold(true);
      label->menuAction()->setFont(font);
    }
    auto* show = label->addAction(entry.second.visible ? QObject::tr("Hide") : QObject::tr("Show"));
    const auto visibility = entry.second.visible ? "hide " : "show ";
    QObject::connect(show,&QAction::triggered,&menu,[submit,quoted,suffix,visibility]()
    {
      submit(QStringLiteral("post label ")+visibility+quoted+suffix);
    });
    auto* rename = label->addAction(QObject::tr("Rename…"));
    QObject::connect(rename,&QAction::triggered,&menu,[parent,submit,name,quoted,suffix]()
    {
      bool accepted = false;
      const auto replacement = QInputDialog::getText(parent,QObject::tr("Rename result label"),QObject::tr("Name"),
        QLineEdit::Normal,QString::fromUtf8(name.c_str()),&accepted);
      const auto next = annotation_quote(replacement.toUtf8().toStdString());
      if (accepted && !next.isEmpty())
      {
        submit(QStringLiteral("post label rename ")+quoted+" "+next+suffix);
      }
    });
    auto* remove = label->addAction(QObject::tr("Remove"));
    QObject::connect(remove,&QAction::triggered,&menu,[submit,quoted,suffix]()
    {
      submit(QStringLiteral("post label remove ")+quoted+suffix);
    });
  }
  menu.exec(globalPosition);
}
