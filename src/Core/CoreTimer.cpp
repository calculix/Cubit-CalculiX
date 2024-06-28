#include "CoreTimer.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"

#include "ComponentInfo.hpp"
#include "Mediator.hpp"
#include "Broker.hpp"
#include <QString>

CoreTimer::CoreTimer()
{
  CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
     
  connect(this, &QTimer::timeout, this, &CoreTimer::timeoutaction);
  this->start(1000);
}

CoreTimer::~CoreTimer()
{}

void CoreTimer::timeoutaction()
{
  ccx_iface->check_jobs();
  //ccx_iface->check_zombie();
  if (!this->bool_init_pythoninterface)
  {
    this->bool_init_pythoninterface = ccx_iface->init_pythoninterface();
  //}
  /*
  Mediator* med = Broker::instance();
  std::vector<ComponentInfo> info;
  med->get_component_list(info);

  QString output;
  std::string log;
  for (size_t i = 0; i < info.size(); i++)
  {
    output = info[i].get_class();
    log = " class: " + output.toStdString() + " \n";
    output = info[i].get_module();
    log.append(" module: " + output.toStdString() + " \n");
    output = info[i].get_path();
    log.append(" path: " + output.toStdString() + " \n");
    output = info[i].get_state();
    log.append(" state: " + output.toStdString() + " \n");
    PRINT_INFO("%s", log.c_str());
  }
  */
  }
}