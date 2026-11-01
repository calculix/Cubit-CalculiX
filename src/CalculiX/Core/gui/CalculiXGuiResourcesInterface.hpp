#ifndef CALCULIXGUIRESOURCESINTERFACE_HPP
#define CALCULIXGUIRESOURCESINTERFACE_HPP

#include <string>
#include <QIcon>

class CalculiXGuiResourcesInterface
{
public:
  QIcon* getIcon(std::string name);
  QIcon getIcon2(std::string name);
};

#endif // CALCULIXGUIRESOURCESINTERFACE_HPP
