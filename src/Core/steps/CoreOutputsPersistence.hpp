#ifndef COREOUTPUTSPERSISTENCE_HPP
#define COREOUTPUTSPERSISTENCE_HPP

class CoreHistoryOutputs;
class CoreFieldOutputs;
class HDF5Tool;

class CoreOutputsPersistence
{
public:
  static void read(HDF5Tool& file, CoreHistoryOutputs& outputs);
  static void write(HDF5Tool& file, const CoreHistoryOutputs& outputs);

  static void read(HDF5Tool& file, CoreFieldOutputs& outputs);
  static void write(HDF5Tool& file, const CoreFieldOutputs& outputs);
};

#endif
