#ifndef CORERESULTSDATPERSISTENCE_HPP
#define CORERESULTSDATPERSISTENCE_HPP

class CoreResultsDat;
class HDF5Tool;

class CoreResultsDatPersistence
{
public:
  static bool read(HDF5Tool& file, CoreResultsDat& results);
  static bool write(HDF5Tool& file, const CoreResultsDat& results);
};

#endif
