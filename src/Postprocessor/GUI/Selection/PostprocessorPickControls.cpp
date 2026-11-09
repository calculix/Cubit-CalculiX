#include "PostprocessorPickControls.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Resources/PostprocessorIconLoader.hpp"
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QSignalBlocker>
#include <QToolButton>
#include <QToolBar>
#include <utility>
PostprocessorPickControls::PostprocessorPickControls(std::weak_ptr<PostprocessorApplicationInterface> active,
  std::function<void(const QString&)> dispatcher, QWidget* parent)
  : QWidget(parent), application(std::move(active)), submit(std::move(dispatcher))
{
  setObjectName("Postprocessor.PickControls");
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(0);
  modes = new QButtonGroup(this);
  modes->setObjectName("Postprocessor.PickMode");
  modes->setExclusive(true);
  const auto activeApplication = application.lock();
  const QIcon integrationPointIcon = activeApplication ?
    PostprocessorIconLoader::load(activeApplication->active_icons_path(),"PostprocessorIntegrationPoint") : QIcon();
  const QIcon icons[] = {QIcon(":/cubitgui/node.svg"),QIcon(":/cubitgui/hex.svg"),integrationPointIcon};
  const QString labels[] = {tr("Pick nodes"),tr("Pick elements"),tr("Pick integration points")};
  const QString names[] = {"Postprocessor.PickNode","Postprocessor.PickElement","Postprocessor.PickIntegrationPoint"};
  const Postprocessor::EntityKind kinds[] = {Postprocessor::EntityKind::Node,
    Postprocessor::EntityKind::Element,Postprocessor::EntityKind::IntegrationPoint};
  for (int i = 0; i < 3; ++i)
  {
    auto* button = new QToolButton(this);
    button->setObjectName(names[i]);
    button->setIcon(icons[i]);
    button->setToolTip(labels[i]);
    button->setAccessibleName(labels[i]);
    button->setToolButtonStyle(Qt::ToolButtonIconOnly);
    button->setAutoRaise(true);
    button->setCheckable(true);
    modes->addButton(button,static_cast<int>(kinds[i]));
    layout->addWidget(button);
  }
  if (auto* toolbar = qobject_cast<QToolBar*>(parent))
  {
    for (auto* button : findChildren<QToolButton*>())
    {
      button->setIconSize(toolbar->iconSize());
    }
    connect(toolbar,&QToolBar::iconSizeChanged,this,[this](const QSize& size)
    {
      for (auto* button : findChildren<QToolButton*>())
      {
        button->setIconSize(size);
      }
    });
  }
  connect(modes,&QButtonGroup::idClicked,this,[this](int id)
  {
    if (submit)
    {
      const auto kind = static_cast<Postprocessor::EntityKind>(id);
      const QString mode = kind == Postprocessor::EntityKind::Node ? "node" :
        kind == Postprocessor::EntityKind::Element ? "element" : "integrationpoint";
      submit("post pick mode "+mode);
    }
    refresh();
  });
  refresh();
}
void PostprocessorPickControls::refresh()
{
  const QSignalBlocker blocker(modes);
  const auto active = application.lock();
  if (!active)
  {
    setEnabled(false);
    return;
  }
  const auto& session = active->session();
  const auto view = session.views.find(session.active_view_id);
  setEnabled(view != session.views.end());
  if (view != session.views.end())
  {
    if (auto* button = modes->button(static_cast<int>(view->second.selection.mode)))
    {
      button->setChecked(true);
    }
  }
}
