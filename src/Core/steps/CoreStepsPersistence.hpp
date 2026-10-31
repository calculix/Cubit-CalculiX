#ifndef CORESTEPSPERSISTENCE_HPP
#define CORESTEPSPERSISTENCE_HPP

class CoreSteps;
class HDF5Tool;

class CoreStepsPersistence
{
public:
  static void read(HDF5Tool& file, CoreSteps& steps);
  static void write(HDF5Tool& file, const CoreSteps& steps);
};

#endif
