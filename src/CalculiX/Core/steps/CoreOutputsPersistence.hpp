#ifndef COREOUTPUTSPERSISTENCE_HPP
#define COREOUTPUTSPERSISTENCE_HPP

class CoreHistoryOutputs;
class CoreFieldOutputs;
class HDF5Tool;

class CoreOutputsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreHistoryOutputs& outputs);
  static bool write(HDF5Tool& file, const CoreHistoryOutputs& outputs);

  static bool read(HDF5Tool& file, CoreFieldOutputs& outputs);
  static bool write(HDF5Tool& file, const CoreFieldOutputs& outputs);
};

#endif
