/*!
 *  \file PanelManager.hpp
 *  \brief
 *    Provides an example of how to create command panels.
 *  \author Michael Plooster
 *  \date 11 Feb 2014
 */

#ifndef PANELMANAGER_HPP
#define PANELMANAGER_HPP

class MyWidgetFactory;

class PanelManager
{
public:
  PanelManager();
  ~PanelManager();

  // Setup navigation nodes and associate command panel widgets with the
  // navigation nodes.
  void initialize();

  // Cleanup (for example, when we unload the component)
  void clear();

private:
  bool isInitialized;
  MyWidgetFactory* myFactory;

  // Two examples of how to initialize command panel navigation buttons.
  void initialize_from_xml();
  void initialize_from_code();

  // Associate command panels with navigation nodes.
  void associate_panels_with_nodes();
};

#endif // PANELMANAGER_HPP
