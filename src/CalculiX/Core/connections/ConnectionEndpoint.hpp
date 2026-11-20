#pragma once
#include <string>

enum class ConnectionEndpointKind
{
  Vertex,
  ReferencePoint
};

struct ConnectionEndpoint
{
  ConnectionEndpointKind kind = ConnectionEndpointKind::Vertex;
  int id = 0;
  bool uses_point(int point) const;
  const char* keyword() const;
};
