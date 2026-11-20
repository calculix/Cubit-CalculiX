#include "ConnectionEndpoint.hpp"

bool ConnectionEndpoint::uses_point(int point) const
{
  return kind == ConnectionEndpointKind::ReferencePoint && id == point;
}

const char* ConnectionEndpoint::keyword() const
{
  return kind == ConnectionEndpointKind::Vertex ? "vertex" : "rp";
}
