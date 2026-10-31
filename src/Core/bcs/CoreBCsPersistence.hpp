#ifndef COREBCSPERSISTENCE_HPP
#define COREBCSPERSISTENCE_HPP

class CoreBCsDisplacements;
class CoreBCsTemperatures;
class CoreHBCs;
class HDF5Tool;

class CoreBCsPersistence
{
public:
  static void read(HDF5Tool& file, CoreBCsDisplacements& displacements);
  static void write(HDF5Tool& file, const CoreBCsDisplacements& displacements);

  static void read(HDF5Tool& file, CoreBCsTemperatures& temperatures);
  static void write(HDF5Tool& file, const CoreBCsTemperatures& temperatures);

  static void read(HDF5Tool& file, CoreHBCs& hbcs);
  static void write(HDF5Tool& file, const CoreHBCs& hbcs);
};

#endif
