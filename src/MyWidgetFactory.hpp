/*!
 *  \file MyWidgetFactory.hpp
 *  \brief
 *    Provides examples of how to use the widget factory to create
 *    command panels.
 *  \author Michael Plooster
 *  \date 24 Feb 2014
 */

#ifndef MYWIDGETFACTORY_HPP
#define MYWIDGETFACTORY_HPP

#include "NavigationWidgetFactory.hpp"

#include <QMap>
#include <QObject>

class NavigationWidgetCache;

class MyWidgetFactory : public QObject, public NavigationWidgetFactory
{
  Q_OBJECT

public:
  MyWidgetFactory(QObject *parent=0);
  ~MyWidgetFactory();

  // Re-implemented from NavigationWidgetFactory (required).
  // This returns the command panel widget that is requested by the framework.
  QWidget* getWidget(NavigationModel *model,
                     const QModelIndex &index,
                     QWidget *parent);

  // Clean up our widgets
  void clear();

private slots:
  // Removes our cache when a parent widget (object) is destroyed.
  void removeCache(QObject *object);

private:
  // Map to store our created widgets (cache).
  QMap<QWidget*, NavigationWidgetCache*> widget_cache;

  // Create a widget based on the name (used in the get_widget_using_cache
  // example).
  QWidget* createWidget(const QString &name);
};

#endif // MYWIDGETFACTORY_HPP
