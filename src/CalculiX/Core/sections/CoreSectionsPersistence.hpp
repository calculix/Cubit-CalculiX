#ifndef CORESECTIONSPERSISTENCE_HPP
#define CORESECTIONSPERSISTENCE_HPP

class CoreSections;
class HDF5Tool;

class CoreSectionsPersistence
{
public:
  static void read(HDF5Tool& file, CoreSections& sections);
  static void write(HDF5Tool& file, const CoreSections& sections);
};

#endif
