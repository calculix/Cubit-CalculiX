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
   * unchanged. workers must be positive; one keeps writes serial. Each active
   * write owns private VTK objects/caches while sharing immutable numeric buffers
   * where supported. The result must not be modified during export. poll runs
   * on the calling thread; false cancels scheduling and joins active writes
   * before removing staging output. workers_used reports the effective limit.
   */
  bool export_vtk_result(const VtkResultsData& result, const std::string& destination,
    std::string& index_path, std::string& error, std::size_t workers,
    const std::function<bool(double)>& poll = {}, std::size_t* workers_used = nullptr);
}

#endif
