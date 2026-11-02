#ifndef COREBCSPERSISTENCE_HPP
#define COREBCSPERSISTENCE_HPP

class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreHBCs;
class HDF5Tool;

class CoreBCsPersistence
{
public:
  static bool read(HDF5Tool& file, CoreBCsDisplacements& displacements);
  static bool write(HDF5Tool& file, const CoreBCsDisplacements& displacements);

  static bool read(HDF5Tool& file, CoreBCsTemperatures& temperatures);
  static bool write(HDF5Tool& file, const CoreBCsTemperatures& temperatures);

  static bool read(HDF5Tool& file, CoreHBCs& hbcs);
  static bool write(HDF5Tool& file, const CoreHBCs& hbcs);
};

#endif
