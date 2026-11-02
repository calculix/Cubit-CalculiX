#ifndef COREMODELMETADATAPERSISTENCE_HPP
#define COREMODELMETADATAPERSISTENCE_HPP

#include <string>

class HDF5Tool;

class CoreModelMetadataPersistence
{
public:
  static bool read(HDF5Tool& file);
  static bool write(HDF5Tool& file, const std::string& version);
};

#endif
