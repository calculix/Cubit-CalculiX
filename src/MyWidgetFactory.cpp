/*!
 *  \file MyWidgetFactory.cpp
 *  \brief
 *    Provides examples of how to use the widget factory to create
 *    command panels.
 *  \author Michael Plooster
 *  \date 24 Feb 2014
 */

#include "MyWidgetFactory.hpp"
#include "NavigationModel.hpp"
#include "NavigationNode.hpp"
#include "NavigationWidgetCache.hpp"
#include "Claro.hpp"

// A list of headers for my command panels.
#include "MyPanel1.hpp"
#include "MyPanel2.hpp"

#include <QWidget>

MyWidgetFactory::MyWidgetFactory(QObject* parent) :
  QObject(parent),
  widget_cache()
{}

MyWidgetFactory::~MyWidgetFactory()
{
  clear();
}

QWidget* MyWidgetFactory::getWidget(NavigationModel *model,
                                    const QModelIndex &index,
                                    QWidget *widgetParent)
{
  if(!model || !index.isValid() || !widgetParent)
      return 0;

  // Get the marker name for the index.
  NavigationNode *node = model->getNode(index);
  QString name = node->getMarker();

  // If there isn't a marker, use the path.
  if(name.isEmpty())
    name = model->getPath(node);

  // See if the widget already exists.
  QWidget *widget = 0;
  QMap<QWidget *, NavigationWidgetCache *>::Iterator iter =
      widget_cache.find(widgetParent);
  if(iter != widget_cache.end())
  {
    widget = (*iter)->getWidget(name);
    if(widget)
      return widget;
  }

  // Create a new widget based on the name.
  widget = createWidget(name);
  if(widget)
  {
      // Add a new cache for the parent widget if needed.
      NavigationWidgetCache *cache = 0;
      if(iter == widget_cache.end())
      {
        cache = new NavigationWidgetCache(this);
        widget_cache.insert(widgetParent, cache);
        connect(widgetParent, SIGNAL(destroyed(QObject*)),
                this, SLOT(removeCache(QObject*)));
      }
      else
        cache = *iter;

      // Add the widget to the cache.
      cache->insert(name, widget);
  }

  return widget;
}

// We want to be sure to clean up all the memory we allocated.
void MyWidgetFactory::clear()
{
  // Clean up our widget cache if the create_widget_using_cache example was used.
  QMap<QWidget*, NavigationWidgetCache*>::Iterator iter;
  for(iter = widget_cache.begin(); iter != widget_cache.end(); iter++)
    delete *iter;

  widget_cache.clear();
}

void MyWidgetFactory::removeCache(QObject *object)
{
  QWidget *widget = qobject_cast<QWidget *>(object);
  if(widget)
  {
    // Remove the widget cache for the parent widget.
    QMap<QWidget *, NavigationWidgetCache *>::Iterator iter =
        widget_cache.find(widget);
    if(iter != widget_cache.end())
    {
      delete *iter;
      widget_cache.erase(iter);
    }
  }
}

// Create and return a widget based on the marker (used for
// the get_widget_using_cache example).
QWidget* MyWidgetFactory::createWidget(const QString &name)
{
  // We don't worry about setting the widget parent here because it is
  // set in the getWidget() function.

  if(name == "MySecondLevelNode1")
    return new MyPanel1();

  else if(name == "MySecondLevelNode2")
    return new MyPanel2();

  else
    return NULL;
}
