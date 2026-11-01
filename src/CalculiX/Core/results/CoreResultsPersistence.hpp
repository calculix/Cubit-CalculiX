#ifndef CORERESULTSPERSISTENCE_HPP
#define CORERESULTSPERSISTENCE_HPP

class CoreResults;
class HDF5Tool;

class CoreResultsPersistence
{
public:
  static void read_registry(HDF5Tool& file, CoreResults& results);
  static void write_registry(HDF5Tool& file, const CoreResults& results);
  static void initialize_readers(CoreResults& results);

  static bool read_loaded_results(HDF5Tool& file, CoreResults& results);
  static void write(HDF5Tool& file, const CoreResults& results, bool save_loaded_results);
};

#endif
