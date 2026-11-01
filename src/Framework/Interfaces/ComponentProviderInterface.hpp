#ifndef COMPONENTPROVIDERINTERFACE_HPP
#define COMPONENTPROVIDERINTERFACE_HPP

#include <string>

class QWidget;

class ComponentProviderInterface
{
public:
  virtual ~ComponentProviderInterface() = default;

  // Called in GUI and headless sessions. Each provider selects its features.
  // The optional host is borrowed; implementations must not delete it.
  virtual bool start_component(int withGUI, QWidget* host, std::string& error) = 0;

  // Also valid after partial startup; implementations must not throw.
  virtual void stop_component() noexcept = 0;
};

#endif // COMPONENTPROVIDERINTERFACE_HPP
