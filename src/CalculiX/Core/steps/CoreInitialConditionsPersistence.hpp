#ifndef COREINITIALCONDITIONSPERSISTENCE_HPP
#define COREINITIALCONDITIONSPERSISTENCE_HPP

class CoreInitialConditions;
class HDF5Tool;

class CoreInitialConditionsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreInitialConditions& conditions);
  static bool write(HDF5Tool& file, const CoreInitialConditions& conditions);
};

#endif
