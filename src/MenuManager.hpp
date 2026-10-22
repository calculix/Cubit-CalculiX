/*! @file MenuManager.hpp
 *  @brief Provides the Cubit-CalculiX documentation entry in the Help menu.
 */
#ifndef MENUMANAGER_HPP
#define MENUMANAGER_HPP

#include <QObject>
#include <QPointer>

class QAction;

class MenuManager : public QObject
{
  Q_OBJECT

public:
  MenuManager(QObject* parent = nullptr);
  ~MenuManager();

  void setup_help_menu();
  void remove_menu_items();

public slots:
  void open_documentation();

private:
  bool help_initialized = false;
  QPointer<QAction> help_action;
};

#endif // MENUMANAGER_HPP
