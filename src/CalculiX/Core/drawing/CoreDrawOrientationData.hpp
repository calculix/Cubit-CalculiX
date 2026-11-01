#ifndef COREDRAWORIENTATIONDATA_HPP
#define COREDRAWORIENTATIONDATA_HPP

#include <vector>

class CoreOrientations;
class CoreSections;

class CoreDrawOrientationData
{
public:
  static std::vector<std::vector<double>> get_draw_data_for_orientation(
    CoreOrientations& orientations, CoreSections& sections, int id);
};

#endif
