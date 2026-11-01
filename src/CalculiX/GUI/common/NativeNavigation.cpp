#include "NativeNavigation.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"
#include "NavigationWidgetFactory.hpp"

#include <QAction>
#include <QIcon>
#include <QPersistentModelIndex>
#include <QPointer>
#include <stdexcept>

namespace
{
NavigationWidgetFactory* inheritedFactory(NavigationNode* node)
{
  for (auto* ancestor = node; ancestor != nullptr; ancestor = ancestor->getParent())
  {
    if (ancestor->getFactory() != nullptr)
    {
      return ancestor->getFactory();
    }
  }

  return nullptr;
}

class BorrowedNavigationWidgetFactory : public NavigationWidgetFactory
{
public:
  BorrowedNavigationWidgetFactory(NavigationModel* model, NavigationNode* source) : model(model), index(model->getIndex(source))
  {
  }

  QWidget* getWidget(NavigationModel*, const QModelIndex&, QWidget* parent) override
  {
    if (model == nullptr || !index.isValid())
    {
      return nullptr;
    }

    auto* factory = inheritedFactory(model->getNode(index));

    // Native panels receive their original model/index, including unmarked nodes.
    return factory == nullptr ? nullptr : factory->getWidget(model.data(), index, parent);
  }

private:
  QPointer<NavigationModel> model;
  QPersistentModelIndex index;
};
}

void NativeNavigation::copy_branch(NavigationModel* sourceModel, NavigationNode* source, NavigationModel* targetModel, NavigationNode* target)
{
  if (sourceModel == nullptr || source == nullptr || targetModel == nullptr || target == nullptr)
  {
    throw std::runtime_error("Cannot copy an unavailable Cubit navigation branch.");
  }

  target->setLabel(source->getLabel());
  target->setTitle(source->getTitle());
  target->setToolTip(source->getToolTip());
  target->setEnabled(source->isEnabled());
  target->setDisplayType(source->getDisplayType());
  target->setUseComboForChildren(source->getUseComboForChildren());

  for (const auto& tag : source->getSearchTags())
  {
    target->addSearchTag(tag);
  }

  if (source->getIcon() != nullptr)
  {
    target->setIcon(new QIcon(*source->getIcon()));
  }

  if (inheritedFactory(source) != nullptr)
  {
    target->setFactory(new BorrowedNavigationWidgetFactory(sourceModel, source), true);
  }

  if (auto* action = source->getAction())
  {
    auto* proxy = new QAction(action->icon(), action->text(), targetModel);
    proxy->setEnabled(action->isEnabled());
    QObject::connect(proxy, &QAction::triggered, action, &QAction::trigger);
    QObject::connect(action, &QAction::changed, proxy, [guard = QPointer<QAction>(action), proxy]()
    {
      if (guard != nullptr)
      {
        proxy->setEnabled(guard->isEnabled());
      }
    });
    QObject::connect(action, &QObject::destroyed, proxy, [proxy]()
    {
      proxy->setEnabled(false);
    });
    target->setAction(proxy);
  }

  for (int index = 0; index < source->childCount(); ++index)
  {
    auto* child = source->getChild(index);
    auto* copy = targetModel->addNode(child->getName(), target);

    if (!child->getMarker().isEmpty())
    {
      if (targetModel->getMarkedNode(child->getMarker()) != nullptr)
      {
        throw std::runtime_error("Duplicate native navigation marker: " + child->getMarker().toStdString());
      }

      targetModel->setNodeMarker(copy, child->getMarker());
    }

    copy_branch(sourceModel, child, targetModel, copy);

    if (source->getActiveChild() == child)
    {
      target->setActiveChild(copy);
    }
  }

  targetModel->updateNode(target);
}
