#ifndef COREBLOCKSPERSISTENCE_HPP
#define COREBLOCKSPERSISTENCE_HPP

class CoreBlocks;
class HDF5Tool;

class CoreBlocksPersistence
{
public:
  static bool read(HDF5Tool& file, CoreBlocks& blocks);
  static bool write(HDF5Tool& file, const CoreBlocks& blocks);
};

#endif
