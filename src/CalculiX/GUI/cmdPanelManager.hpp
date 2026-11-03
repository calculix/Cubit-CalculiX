/*!
 *  \file CMDPanelManager.hpp
 *  \brief
 *    Provides an example of how to create command panels.
 *  \author Michael Plooster
 *  \date 11 Feb 2014
 */

#ifndef CMDPANELMANAGER_HPP
#define CMDPANELMANAGER_HPP

#include <memory>
#include <QAction>
#include <QIcon>
#include <QStringList>
#include <QString>

class MyCmdWidgetFactory;
class NavigationModel;
class cmdPanelManager
{
public:
  cmdPanelManager();
  ~cmdPanelManager();

  // Setup navigation nodes and associate command panel widgets with the
  // navigation nodes.
  void initialize();
  NavigationModel* navigation_model() const noexcept;

  // Cleanup (for example, when we unload the component)
  void clear();
  void clearTopologyPanels(int topology_id = -1);

private:
  std::unique_ptr<NavigationModel> navigationModel;
  bool isInitialized;
  MyCmdWidgetFactory* myFactory;


  // Two examples of how to initialize command panel navigation buttons.
  void initialize_from_xml();
  void initialize_from_code();

  // Associate command panels with navigation nodes.
  void associate_panels_with_nodes();
  void add_native_mesh_panels();
};

#endif // CMDPANELMANAGER_HPP
