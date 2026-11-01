#ifndef CORERESULTSFRDPERSISTENCE_HPP
#define CORERESULTSFRDPERSISTENCE_HPP

class CoreResultsFrd;
class HDF5Tool;

class CoreResultsFrdPersistence
{
public:
  static void read_base_data(HDF5Tool& file, CoreResultsFrd& results);
  static void write_base_data(HDF5Tool& file, const CoreResultsFrd& results);

  static void read_result_blocks(HDF5Tool& file, CoreResultsFrd& results);
  static void write_result_blocks(HDF5Tool& file, const CoreResultsFrd& results);

  static void read_sorted_indices(HDF5Tool& file, CoreResultsFrd& results);
  static void write_sorted_indices(HDF5Tool& file, const CoreResultsFrd& results);

  static void read(HDF5Tool& file, CoreResultsFrd& results);
  static void write(HDF5Tool& file, const CoreResultsFrd& results);
};

#endif
