#ifndef CALCULIX_PROPERTY_SELECTION_HPP
#define CALCULIX_PROPERTY_SELECTION_HPP

#include <QIcon>
#include <QMetaType>
#include <QString>

// Independent from TopologyTree's existing Qt::UserRole range.
enum PropertyItemRole
{
  PropertyFeatureRole = Qt::UserRole + 1000,
  PropertyIdRole,
  PropertyStepIdRole,
  PropertyTopologyIdRole,
  PropertyIterationIdRole,
  PropertyHomogeneousRole
};

enum class PropertyFeature
{
  None,
  Force,
  Pressure,
  Heatflux,
  SurfaceTraction,
  Gravity,
  Centrifugal,
  Film,
  Radiation,
  BodyHeatflux,
  Temperature,
  Displacement,
  Section,
  Orientation,
  Constraint,
  ContactPair,
  Amplitude,
  CustomLine,
  Step,
  HistoryOutput,
  FieldOutput,
  InitialCondition,
  Job,
  Damping,
  PhysicalConstants,
  SurfaceInteraction,
  Trajectory,
  Block,
  Nodeset,
  Sideset,
  Material,
  HBCs,
  Topology,
  TopologyIteration,
};

struct PropertySelection
{
  PropertyFeature feature = PropertyFeature::None;
  int id = -1;
  int stepId = -1;
  int topologyId = -1;
  int iterationId = -1;
  bool homogeneous = false;
  QString caption;
  QIcon icon;
};

Q_DECLARE_METATYPE(PropertySelection)
#endif // CALCULIX_PROPERTY_SELECTION_HPP
