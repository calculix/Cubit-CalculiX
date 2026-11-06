#ifndef CUBITSIM_VTKRESULTSEXPORTER_HPP
#define CUBITSIM_VTKRESULTSEXPORTER_HPP

#include "VtkResultsData.hpp"
#include <functional>
#include <string>

namespace SimResults
{
  /** Export already converted data without consulting a solver or current model.
   * destination must not exist. A private staging directory is renamed only
   * after all data and the PVD index have been written. Published arrays remain
   * unchanged. poll runs on the calling thread; false cancels the export.
   */
  bool export_vtk_result(const VtkResultsData& result, const std::string& destination,
    std::string& index_path, std::string& error, const std::function<bool(double)>& poll = {});
}

#endif
