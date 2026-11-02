#ifndef PROPERTYREFERENCERESOLVER_HPP
#define PROPERTYREFERENCERESOLVER_HPP

#include <QString>

enum class PropertyReferenceKind
{
  Block, Nodeset, Sideset, Material, Orientation, Amplitude,
  Step, Job, SurfaceInteraction, ContactPair, Vertex, Curve, Surface,
  Displacement, Temperature, Topology
};

class PropertyReferenceResolver
{
public:
  static QString name(PropertyReferenceKind kind, const QString& id);
  static QString names(PropertyReferenceKind kind, const QString& ids);
};

#endif
