/*!
 *  \file ToolbarManager.hpp
 *  \brief
 *    Provides an example of how to create a toolbar and toolbar items
 *    using the Claro framework.
 *  \author Michael Plooster
 *  \date 11 Feb 2014
 */

#ifndef TOOLBARMANAGER_HPP
#define TOOLBARMANAGER_HPP

#include <QObject>

class ToolbarManager : public QObject
{
  Q_OBJECT

public:
  ToolbarManager(QObject* parent = 0);
  ~ToolbarManager();

  // Provides an example of adding an tiem to an existing toolbar
  void add_to_existing_toolbar();

  // Provides an example of creating your own toolbar
  void setup_custom_toolbar();

  // Provides an example of removing your toolbar items.
  void remove_toolbar_items();

public slots:
  void topview_triggered();
  void button1_triggered();
  void button2_triggered();

private:
  bool existingInitialized;
  bool customInitialized;

};

#endif // TOOLBARMANAGER_HPP
