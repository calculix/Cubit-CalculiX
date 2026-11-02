#ifndef COREINTERACTIONSPERSISTENCE_HPP
#define COREINTERACTIONSPERSISTENCE_HPP

class CoreConstraints;
class CoreSurfaceInteractions;
class CoreContactPairs;
class HDF5Tool;

class CoreInteractionsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreConstraints& constraints);
  static bool write(HDF5Tool& file, const CoreConstraints& constraints);

  static bool read(HDF5Tool& file, CoreSurfaceInteractions& interactions);
  static bool write(HDF5Tool& file, const CoreSurfaceInteractions& interactions);

  static bool read(HDF5Tool& file, CoreContactPairs& contacts);
  static bool write(HDF5Tool& file, const CoreContactPairs& contacts);
};

#endif
