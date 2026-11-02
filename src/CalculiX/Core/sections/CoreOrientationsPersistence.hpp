#ifndef COREORIENTATIONSPERSISTENCE_HPP
#define COREORIENTATIONSPERSISTENCE_HPP

class CoreOrientations;
class HDF5Tool;

class CoreOrientationsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreOrientations& orientations);
  static bool write(HDF5Tool& file, const CoreOrientations& orientations);
};

#endif
