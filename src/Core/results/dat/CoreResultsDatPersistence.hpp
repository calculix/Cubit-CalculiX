#ifndef CORERESULTSDATPERSISTENCE_HPP
#define CORERESULTSDATPERSISTENCE_HPP

class CoreResultsDat;
class HDF5Tool;

class CoreResultsDatPersistence
{
public:
  static void read_block_metadata(HDF5Tool& file, CoreResultsDat& results);
  static void write_block_metadata(HDF5Tool& file, const CoreResultsDat& results);

  static void read_result_values(HDF5Tool& file, CoreResultsDat& results);
  static void write_result_values(HDF5Tool& file, const CoreResultsDat& results);

  static bool read_section_results(HDF5Tool& file, CoreResultsDat& results);
  static void write_section_results(HDF5Tool& file, const CoreResultsDat& results);

  static void read_sorted_indices(HDF5Tool& file, CoreResultsDat& results);
  static void write_sorted_indices(HDF5Tool& file, const CoreResultsDat& results);

  static bool read(HDF5Tool& file, CoreResultsDat& results);
  static void write(HDF5Tool& file, const CoreResultsDat& results);
};

#endif
