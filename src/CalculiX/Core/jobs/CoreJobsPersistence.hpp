#ifndef COREJOBSPERSISTENCE_HPP
#define COREJOBSPERSISTENCE_HPP

class CoreJobs;
class HDF5Tool;

class CoreJobsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreJobs& jobs);
  static bool write(HDF5Tool& file, const CoreJobs& jobs);
};

#endif
