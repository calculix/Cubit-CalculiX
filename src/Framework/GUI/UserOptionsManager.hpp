#ifndef CUBITSIM_USEROPTIONSMANAGER_HPP
#define CUBITSIM_USEROPTIONSMANAGER_HPP

#include "Framework/Interfaces/WorkspaceProviderInterface.hpp"
#include <QObject>
#include <QPointer>
#include <vector>

class OptionsPanel;

class UserOptionsManager : public QObject
{
public:
  UserOptionsManager();
  ~UserOptionsManager() override;
  bool attach(const std::vector<WorkspaceProviderInterface*>& providers, std::string& error);
  void detach() noexcept;

private:
  QPointer<OptionsPanel> options;
  std::vector<WorkspaceView> pages;
};

#endif
