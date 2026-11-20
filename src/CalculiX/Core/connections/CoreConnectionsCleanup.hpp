#pragma once
#include <string>
class CoreConnections;

class CoreConnectionsCleanup
{
public:
  static bool cleanup(CoreConnections& connections, std::string& log);
};
