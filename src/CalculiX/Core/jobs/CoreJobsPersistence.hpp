#ifndef COREJOBSPERSISTENCE_HPP
#define COREJOBSPERSISTENCE_HPP

class CoreJobs;
class HDF5Tool;

class CoreJobsPersistence
{
public:
  static void read(HDF5Tool& file, CoreJobs& jobs);
  static void write(HDF5Tool& file, const CoreJobs& jobs);
};

#endif
