
#include "ComponentPlugin.hpp"
#include "Broker.hpp"
#include "CalculiXComp.hpp"

COMPONENT_PLUGIN;

PLUGIN_PRINT_VERSION
{
  Broker::command << "CalculiXComp Version 2024.3\n";
}

PLUGIN_PRINT_OPTIONS
{
}

PLUGIN_REGISTER_COMPONENTS
{
  // Add the component to the framework.
  Broker::instance()->register_component(new CalculiXComp());
}