#ifndef COREORIENTATIONSPERSISTENCE_HPP
#define COREORIENTATIONSPERSISTENCE_HPP

class CoreOrientations;
class HDF5Tool;

class CoreOrientationsPersistence
{
public:
  static void read(HDF5Tool& file, CoreOrientations& orientations);
  static void write(HDF5Tool& file, const CoreOrientations& orientations);
};

#endif
