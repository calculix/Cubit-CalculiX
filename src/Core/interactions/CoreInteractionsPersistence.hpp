#ifndef COREINTERACTIONSPERSISTENCE_HPP
#define COREINTERACTIONSPERSISTENCE_HPP

class CoreConstraints;
class CoreSurfaceInteractions;
class CoreContactPairs;
class HDF5Tool;

class CoreInteractionsPersistence
{
public:
  static void read(HDF5Tool& file, CoreConstraints& constraints);
  static void write(HDF5Tool& file, const CoreConstraints& constraints);

  static void read(HDF5Tool& file, CoreSurfaceInteractions& interactions);
  static void write(HDF5Tool& file, const CoreSurfaceInteractions& interactions);

  static void read(HDF5Tool& file, CoreContactPairs& contacts);
  static void write(HDF5Tool& file, const CoreContactPairs& contacts);
};

#endif
