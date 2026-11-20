#pragma once
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "CubitCommandInterface.hpp"

namespace DiscreteCommandInput
{
  bool endpoint(CubitCommandData& data, const char* vertex, const char* point, ConnectionEndpoint& endpoint);
  bool quantity(CubitCommandData& data, const char* key, double& si, std::string& error);
  bool history(CubitCommandData& data, int& step, std::string& error);
}
