#ifndef CORESTEPSETTINGSPERSISTENCE_HPP
#define CORESTEPSETTINGSPERSISTENCE_HPP

class CoreDamping;
class CorePhysicalConstants;
class HDF5Tool;

class CoreStepSettingsPersistence
{
public:
  static void read(HDF5Tool& file, CoreDamping& damping);
  static void write(HDF5Tool& file, const CoreDamping& damping);

  static void read(HDF5Tool& file, CorePhysicalConstants& constants);
  static void write(HDF5Tool& file, const CorePhysicalConstants& constants);
};

#endif
