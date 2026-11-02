#ifndef CORESTEPSETTINGSPERSISTENCE_HPP
#define CORESTEPSETTINGSPERSISTENCE_HPP

class CoreDamping;
class CorePhysicalConstants;
class HDF5Tool;

class CoreStepSettingsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreDamping& damping);
  static bool write(HDF5Tool& file, const CoreDamping& damping);

  static bool read(HDF5Tool& file, CorePhysicalConstants& constants);
  static bool write(HDF5Tool& file, const CorePhysicalConstants& constants);
};

#endif
