#ifndef CORERESULTSPERSISTENCE_HPP
#define CORERESULTSPERSISTENCE_HPP

class CoreResults;
class HDF5Tool;

class CoreResultsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreResults& results);
  static bool read_registry(HDF5Tool& file, CoreResults& results);
  static bool write_registry(HDF5Tool& file, const CoreResults& results);
  static bool initialize_readers(CoreResults& results);

  static bool read_loaded_results(HDF5Tool& file, CoreResults& results);
  static bool write(HDF5Tool& file, const CoreResults& results, bool save_loaded_results);
};

#endif
