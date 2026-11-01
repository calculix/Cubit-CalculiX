#ifndef COREBLOCKSPERSISTENCE_HPP
#define COREBLOCKSPERSISTENCE_HPP

class CoreBlocks;
class HDF5Tool;

class CoreBlocksPersistence
{
public:
  static void read(HDF5Tool& file, CoreBlocks& blocks);
  static void write(HDF5Tool& file, const CoreBlocks& blocks);
};

#endif
