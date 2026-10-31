#ifndef COREAMPLITUDESPERSISTENCE_HPP
#define COREAMPLITUDESPERSISTENCE_HPP

class CoreAmplitudes;
class HDF5Tool;

class CoreAmplitudesPersistence
{
public:
  static void read(HDF5Tool& file, CoreAmplitudes& amplitudes);
  static void write(HDF5Tool& file, const CoreAmplitudes& amplitudes);
};

#endif
