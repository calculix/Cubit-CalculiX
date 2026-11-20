#include "DiscreteCommandInput.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"

bool DiscreteCommandInput::endpoint(CubitCommandData& data, const char* vertex, const char* point, ConnectionEndpoint& endpoint)
{
  int id = 0;
  if (data.get_value(vertex,id))
  {
    endpoint = {ConnectionEndpointKind::Vertex,id};
    return true;
  }
  if (data.get_value(point,id))
  {
    endpoint = {ConnectionEndpointKind::ReferencePoint,id};
    return true;
  }
  return false;
}

bool DiscreteCommandInput::quantity(CubitCommandData& data, const char* key, double& si, std::string& error)
{
  double value;
  if (!data.get_value("amount",value))
  {
    return true;
  }
  SimUnits::QuantityUnit quantity;
  std::string unit;
  if (!framework_units().quantity(key,quantity,error))
  {
    return false;
  }
  if (!data.get_string("unit_name",unit) && !framework_units().display_unit("calculix",key,unit,error))
  {
    return false;
  }
  return framework_units().to_si(value,unit,quantity.dimension,quantity.role,si,error);
}

bool DiscreteCommandInput::history(CubitCommandData& data, int& step, std::string& error)
{
  if (data.find_keyword("OFF"))
  {
    step = 0;
  }
  else if (data.find_keyword("ALL"))
  {
    step = -1;
  }
  else if (data.find_keyword("STEP") && (!data.get_value("step",step) || step <= 0))
  {
    error = "Select a positive history step ID.";
    return false;
  }
  return true;
}
