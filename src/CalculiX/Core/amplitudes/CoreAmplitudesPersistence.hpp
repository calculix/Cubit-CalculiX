#ifndef COREAMPLITUDESPERSISTENCE_HPP
#define COREAMPLITUDESPERSISTENCE_HPP

class CoreAmplitudes;
class HDF5Tool;

class CoreAmplitudesPersistence
{
public:
  static bool read(HDF5Tool& file, CoreAmplitudes& amplitudes);
  static bool write(HDF5Tool& file, const CoreAmplitudes& amplitudes);
};

#endif
