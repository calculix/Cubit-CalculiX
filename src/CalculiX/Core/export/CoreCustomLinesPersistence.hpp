#ifndef CORECUSTOMLINESPERSISTENCE_HPP
#define CORECUSTOMLINESPERSISTENCE_HPP

class CoreCustomLines;
class HDF5Tool;

class CoreCustomLinesPersistence
{
public:
  static bool read(HDF5Tool& file, CoreCustomLines& lines);
  static bool write(HDF5Tool& file, const CoreCustomLines& lines);
};

#endif
