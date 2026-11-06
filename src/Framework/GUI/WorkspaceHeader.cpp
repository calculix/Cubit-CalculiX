#include "WorkspaceHeader.hpp"

#include <QCoreApplication>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMainWindow>
#include <QScopedValueRollback>
#include <QSignalBlocker>
#include <QTabBar>
#include <QTimer>

WorkspaceHeader::WorkspaceHeader(QMainWindow* window, std::function<void(const QString&)> request)
  : QWidget(window), host(window), previousMargins(window->contentsMargins()), previousLayoutOnEntireRect(window->testAttribute(Qt::WA_LayoutOnEntireRect))
{
  setObjectName("CubitSim.WorkspaceHeader");
  setAutoFillBackground(true);
  auto* row = new QHBoxLayout(this);
  row->setContentsMargins(4, 2, 4, 2);
  tabs = new QTabBar(this);
  tabs->setObjectName("CubitSim.WorkspaceTabs");
  tabs->setAccessibleName(tr("Application workspace"));
  tabs->setExpanding(false);
  tabs->setDrawBase(false);
  tabs->setUsesScrollButtons(false);
  tabs->setElideMode(Qt::ElideNone);
  row->addWidget(tabs);
  row->addStretch();
  auto* caption = new QLabel(tr("  Workspace  "), this);
  caption->setForegroundRole(QPalette::Mid);
  row->addWidget(caption);

  connect(tabs, &QTabBar::currentChanged, this, [this, request](int index)
  {
    const QString target = tabs->tabData(index).toString();
    set_current(current);

    if (!target.isEmpty() && target != current)
    {
      request(target);
    }
  });

  host->installEventFilter(this);
  update_header();
  QTimer::singleShot(0, this, [this]()
  {
    update_header();
  });
  show();
}

WorkspaceHeader::~WorkspaceHeader()
{
  if (host != nullptr)
  {
    host->removeEventFilter(this);

    if (!QCoreApplication::closingDown() && host->layout() != nullptr)
    {
      host->setContentsMargins(previousMargins);
      host->setAttribute(Qt::WA_LayoutOnEntireRect, previousLayoutOnEntireRect);
    }
  }
}

void WorkspaceHeader::add_workspace(const QString& id, const QString& label, const QIcon& icon)
{
  const QSignalBlocker blocker(tabs);
  const int index = tabs->addTab(icon, label);
  tabs->setTabData(index, id);
  tabs->setMinimumSize(tabs->sizeHint());
  update_header();
}

bool WorkspaceHeader::eventFilter(QObject* object, QEvent* event)
{
  if (object == host && !updatingHeader)
  {
    if (event->type() == QEvent::Resize || event->type() == QEvent::Show || event->type() == QEvent::LayoutRequest || event->type() == QEvent::ContentsRectChange || event->type() == QEvent::FontChange || event->type() == QEvent::StyleChange)
    {
      update_header();
    }
  }

  return QWidget::eventFilter(object, event);
}

void WorkspaceHeader::update_header()
{
  if (host == nullptr || updatingHeader || QCoreApplication::closingDown())
  {
    return;
  }

  const QScopedValueRollback<bool> guard(updatingHeader, true);
  const QSize tabSize = tabs->sizeHint();
  const int rowHeight = tabSize.height() + 4;
  tabs->setMinimumSize(tabSize);
  QMargins margins = previousMargins;
  margins.setTop(margins.top() + rowHeight);

  // Qt must lay out the native menu and toolbars inside the reserved margins.
  host->setAttribute(Qt::WA_LayoutOnEntireRect, false);

  if (host->contentsMargins() != margins)
  {
    host->setContentsMargins(margins);
  }

  setGeometry(previousMargins.left(), previousMargins.top(), qMax(0, host->width() - previousMargins.left() - previousMargins.right()), rowHeight);
  raise();
}

void WorkspaceHeader::set_current(const QString& id)
{
  current = id;
  const QSignalBlocker blocker(tabs);

  for (int index = 0; index < tabs->count(); ++index)
  {
    if (tabs->tabData(index).toString() == id)
    {
      tabs->setCurrentIndex(index);
      break;
    }
  }

  update_header();
}
