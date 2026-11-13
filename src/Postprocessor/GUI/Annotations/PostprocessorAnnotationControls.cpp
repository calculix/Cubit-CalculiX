#include "PostprocessorAnnotationControls.hpp"
#include <utility>
#include "PostprocessorAnnotationMenu.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include <QToolButton>
#include <QToolBar>
#include <QHBoxLayout>
#include <QSignalBlocker>

PostprocessorAnnotationControls::PostprocessorAnnotationControls(std::weak_ptr<PostprocessorApplicationInterface> active,
  std::function<void(const QString&)> dispatch, QWidget* parent)
  : QWidget(parent), application(std::move(active)), submit(std::move(dispatch))
{
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);
  const auto app = application.lock();
  const char* icons[] = {"PostprocessorMinMax","PostprocessorAddResultLabel","PostprocessorShowLabels"};
  const QString tips[] = {tr("Show current minimum and maximum"),tr("Label the selected result entity"),tr("Show result labels")};
  QToolButton** buttons[] = {&extrema,&add,&visible};
  for (int index = 0; index < 3; ++index)
  {
    auto* button = new QToolButton(this);
    *buttons[index] = button;
    button->setObjectName(QString::fromLatin1(icons[index]));
    button->setToolTip(tips[index]);
    button->setAccessibleName(tips[index]);
    button->setAutoRaise(true);
    button->setCheckable(index != 1);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    if (app)
    {
      button->setIcon(PostprocessorIconLoader::load(app->active_icons_path(),icons[index]));
    }
    if (auto* toolbar = qobject_cast<QToolBar*>(parent))
    {
      button->setIconSize(toolbar->iconSize());
      connect(toolbar,&QToolBar::iconSizeChanged,button,&QToolButton::setIconSize);
    }
    layout->addWidget(button);
  }
  connect(extrema,&QToolButton::clicked,this,[this](bool checked)
  {
    submit(QStringLiteral("post labels extrema ")+(checked ? "on" : "off"));
    refresh();
  });
  connect(visible,&QToolButton::clicked,this,[this](bool checked)
  {
    submit(QStringLiteral("post labels ")+(checked ? "show" : "hide"));
    refresh();
  });
  connect(add,&QToolButton::clicked,this,[this]()
  {
    const auto app = application.lock();
    if (app)
    {
      const auto view = app->session().views.find(app->session().active_view_id);
      if (view != app->session().views.end() && view->second.selection.entity)
      {
        const auto command = annotation_add_command(*app,view->first,*view->second.selection.entity);
        if (!command.isEmpty())
        {
          submit(command);
        }
      }
    }
    refresh();
  });
  refresh();
}
void PostprocessorAnnotationControls::refresh()
{
  const auto app = application.lock();
  if (!app)
  {
    setEnabled(false);
    return;
  }
  const auto view = app->session().views.find(app->session().active_view_id);
  setEnabled(view != app->session().views.end());
  if (view == app->session().views.end())
  {
    return;
  }
  const QSignalBlocker blockExtrema(extrema), blockVisible(visible);
  extrema->setChecked(view->second.annotations.extrema_visible);
  visible->setChecked(view->second.annotations.visible);
  add->setEnabled(view->second.selection.entity &&
    !annotation_add_command(*app,view->first,*view->second.selection.entity).isEmpty());
}
