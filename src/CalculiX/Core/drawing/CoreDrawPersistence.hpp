#ifndef COREDRAWPERSISTENCE_HPP
#define COREDRAWPERSISTENCE_HPP

class HDF5Tool;
class CoreDraw;

class CoreDrawPersistence
{
public:
  static bool read(HDF5Tool& file, CoreDraw& drawing);
  static bool write(HDF5Tool& file, const CoreDraw& drawing);
};

#endif
