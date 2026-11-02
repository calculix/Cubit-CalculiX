#ifndef CORESTEPSPERSISTENCE_HPP
#define CORESTEPSPERSISTENCE_HPP

class CoreSteps;
class HDF5Tool;

class CoreStepsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreSteps& steps);
  static bool write(HDF5Tool& file, const CoreSteps& steps);
};

#endif
