#include "DiscreteConnectionProperties.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "GUI/common/CalculiXUnitPresentation.hpp"
#include "CubitMessage.hpp"

namespace
{
QString endpoint_name(const ConnectionEndpoint& endpoint)
{
  const QString type = endpoint.kind == ConnectionEndpointKind::Vertex ? "Vertex" : "RP";
  return type + " " + QString::number(endpoint.id);
}
}

DiscreteConnectionProperties::DiscreteConnectionProperties(QWidget* parent) : PropertyTreeWidget(parent)
{
}

bool DiscreteConnectionProperties::load(PropertyFeature feature, int id)
{
  clear();
  CalculiXConnectionsInterface iface;
  const bool spring = feature == PropertyFeature::Spring;
  double si = 0;
  int history = 0;
  if (spring)
  {
    ConnectionSpring record;
    if (!iface.spring(id,record))
    {
      return false;
    }
    addIdentity("Spring",id,record.name);
    auto* endpoints = addGroup("endpoints","Endpoints",PropertySection::Assignment);
    addValue(endpoints,"Endpoint A",endpoint_name(record.first));
    if (record.second)
    {
      addValue(endpoints,"Endpoint B",endpoint_name(*record.second));
    }
    else
    {
      addValue(endpoints,"Endpoint B","Ground");
      addValue(endpoints,"Global direction",QString(QChar("XYZ"[record.direction-1])));
    }
    si = record.stiffness_si;
    history = record.history_step;
  }
  else
  {
    ConnectionPointMass record;
    if (!iface.point_mass(id,record))
    {
      return false;
    }
    addIdentity("Point mass",id,record.name);
    auto* point = addGroup("point","Assignment",PropertySection::Assignment);
    addValue(point,"Point",endpoint_name(record.endpoint));
    si = record.mass_si;
    history = record.history_step;
  }
  const std::string key = spring ? "force_per_length" : "mass";
  SimUnits::QuantityUnit quantity;
  std::string unit,error;
  double displayed = 0;
  if (!framework_units().quantity(key,quantity,error) || !framework_units().display_unit("calculix",key,unit,error) ||
      !framework_units().from_si(si,unit,quantity.dimension,quantity.role,displayed,error))
  {
    PRINT_ERROR("%s\n",error.c_str());
    return false;
  }
  auto* settings = addGroup("settings","Settings",PropertySection::Settings);
  addNumber(settings,CalculiXUnitPresentation::title(spring ? "Stiffness" : "Mass",key),displayed);
  QString assignment = "All supported steps";
  if (history > 0)
  {
    assignment = "Step " + QString::number(history);
  }
  else if (history == 0)
  {
    assignment = "Off";
  }
  addValue(settings,"History output",assignment);
  return true;
}
