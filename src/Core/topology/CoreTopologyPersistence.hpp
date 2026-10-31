#ifndef CORETOPOLOGYPERSISTENCE_HPP
#define CORETOPOLOGYPERSISTENCE_HPP

class CoreTopology;
class CoreMaterials;
class CoreSections;
class CoreJobs;
class HDF5Tool;

class CoreTopologyPersistence
{
public:
  static bool read_derived_data(HDF5Tool& file, const CoreTopology& topology, CoreMaterials& materials, CoreSections& sections, CoreJobs& jobs);
  static bool write_derived_data(HDF5Tool& file, CoreMaterials& materials, CoreSections& sections);

  static bool read(HDF5Tool& file, CoreTopology& topology, CoreMaterials& materials, CoreSections& sections, CoreJobs& jobs);
  static bool write(HDF5Tool& file, const CoreTopology& topology, CoreMaterials& materials, CoreSections& sections);
};

#endif
