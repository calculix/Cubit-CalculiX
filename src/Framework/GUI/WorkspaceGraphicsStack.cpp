#include "WorkspaceGraphicsStack.hpp"

#include <QCoreApplication>
#include <QLayout>
#include <QSplitter>
#include <QStackedWidget>
#include <QThread>

WorkspaceGraphicsStack::~WorkspaceGraphicsStack()
{
  detach();
}

bool WorkspaceGraphicsStack::attach(QWidget* nativeViewport, std::string& error)
{
  error.clear();

  if (stack != nullptr || nativeViewport == nullptr || nativeViewport->parentWidget() == nullptr ||
      QThread::currentThread() != nativeViewport->thread())
  {
    error = "Cannot attach the workspace graphics stack to the native viewport.";
    return false;
  }

  native = nativeViewport;
  originalParent = native->parentWidget();
  splitter = qobject_cast<QSplitter*>(originalParent.data());
  // A parented splitter child is auto-inserted by Qt. Keep the replacement
  // unparented until replaceWidget() so it is not an existing sibling.
  auto* replacement = new QStackedWidget;
  replacement->setObjectName("CubitSim.WorkspaceGraphicsStack");
  replacement->setSizePolicy(native->sizePolicy());
  bool mounted = false;

  if (splitter != nullptr)
  {
    const int index = splitter->indexOf(native);

    if (index >= 0)
    {
      mounted = splitter->replaceWidget(index, replacement) == native;
    }
  }
  else if (originalParent->layout() != nullptr)
  {
    replacement->setParent(originalParent);
    originalLayout = originalParent->layout();
    auto* item = originalLayout->replaceWidget(native, replacement, Qt::FindDirectChildrenOnly);
    mounted = item != nullptr;
    delete item;
  }

  if (!mounted)
  {
    delete replacement;
    error = "Cubit viewport is not directly managed by a supported splitter or layout.";
    return false;
  }

  stack = replacement;
  stack->addWidget(native);
  stack->setCurrentWidget(native);
  stack->show();
  return true;
}

bool WorkspaceGraphicsStack::activate(QWidget* page, std::string& error)
{
  error.clear();

  if (stack == nullptr || native == nullptr)
  {
    error = "The workspace graphics stack is unavailable.";
    return false;
  }

  QWidget* target = page == nullptr ? native.data() : page;

  if (stack->indexOf(target) < 0)
  {
    pages.append(target);
    stack->addWidget(target);
  }

  stack->setCurrentWidget(target);
  return true;
}

void WorkspaceGraphicsStack::detach() noexcept
{
  if (QCoreApplication::closingDown() || stack == nullptr || originalParent == nullptr || native == nullptr)
  {
    return;
  }

  for (const auto& page : pages)
  {
    if (page != nullptr)
    {
      stack->removeWidget(page);
      page->setParent(originalParent);
      page->hide();
    }
  }
  pages.clear();
  stack->removeWidget(native);

  if (splitter != nullptr)
  {
    const int index = splitter->indexOf(stack);

    if (index >= 0)
    {
      splitter->replaceWidget(index, native);
    }
    else
    {
      native->setParent(originalParent);
      splitter->addWidget(native);
    }
  }
  else if (originalLayout != nullptr)
  {
    auto* item = originalLayout->replaceWidget(stack, native, Qt::FindDirectChildrenOnly);
    if (item == nullptr)
    {
      native->setParent(originalParent);
      originalLayout->addWidget(native);
    }
    delete item;
  }
  else
  {
    native->setParent(originalParent);
  }

  native->show();
  delete stack.data();
}
