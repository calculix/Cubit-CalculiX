#ifndef COREMODELMETADATAPERSISTENCE_HPP
#define COREMODELMETADATAPERSISTENCE_HPP

#include <string>

class HDF5Tool;

class CoreModelMetadataPersistence
{
public:
  static void read(HDF5Tool& file);
  static void write(HDF5Tool& file, const std::string& version);
};

#endif
