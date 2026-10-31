#ifndef COREINITIALCONDITIONSPERSISTENCE_HPP
#define COREINITIALCONDITIONSPERSISTENCE_HPP

class CoreInitialConditions;
class HDF5Tool;

class CoreInitialConditionsPersistence
{
public:
  static void read(HDF5Tool& file, CoreInitialConditions& conditions);
  static void write(HDF5Tool& file, const CoreInitialConditions& conditions);
};

#endif
