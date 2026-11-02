#ifndef CORERESULTSFRDPERSISTENCE_HPP
#define CORERESULTSFRDPERSISTENCE_HPP

class CoreResultsFrd;
class HDF5Tool;

class CoreResultsFrdPersistence
{
public:
  static bool read(HDF5Tool& file, CoreResultsFrd& results);
  static bool write(HDF5Tool& file, const CoreResultsFrd& results);
};

#endif
