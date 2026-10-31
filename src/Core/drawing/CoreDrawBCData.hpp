#ifndef COREDRAWBCDATA_HPP
#define COREDRAWBCDATA_HPP

#include <vector>

class CoreBCsDisplacements;
class CoreBCsTemperatures;
class MeshExportInterface;

class CoreDrawBCData
{
public:
  static std::vector<std::vector<double>> get_draw_data_for_bc_displacement(
    CoreBCsDisplacements& bcs, MeshExportInterface& mesh, int id);

  static std::vector<std::vector<double>> get_draw_data_for_bc_temperature(
    CoreBCsTemperatures& bcs, MeshExportInterface& mesh, int id);
};

#endif
