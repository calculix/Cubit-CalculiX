#include "CoreStepLoadExport.hpp"
#include "CoreSteps.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/mesh/CoreSidesetFaces.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/loads/CoreLoadsPressures.hpp"
#include "Core/loads/CoreLoadsHeatfluxes.hpp"
#include "Core/loads/CoreLoadsGravity.hpp"
#include "Core/loads/CoreLoadsCentrifugal.hpp"
#include "Core/loads/CoreLoadsFilm.hpp"
#include "Core/loads/CoreLoadsRadiation.hpp"
#include "Core/loads/CoreLoadsSurfaceTraction.hpp"
#include "Core/loads/CoreLoadsBodyHeatflux.hpp"
#include "CubitMessage.hpp"

namespace
{
  template<typename LoadType, typename AppendData>
  bool append_selected_loads(
    CoreSteps& steps, LoadType& loads, CoreCustomLines& customlines,
    const std::vector<int>& assignment_ids, std::vector<std::string>& lines,
    int load_type, const std::string& keyword, AppendData append_data)
  {
    for (const auto& load : loads.loads_data)
    {
      for (int assignment_id : assignment_ids)
      {
        const auto& assignment = steps.loads_data[assignment_id];

        if (assignment[1] != load_type || assignment[2] != load[0])
        {
          continue;
        }

        const int load_id = load[0];
        const auto before = customlines.get_customline_data("BEFORE", keyword, load_id);
        lines.insert(lines.end(), before.begin(), before.end());

        if (!append_data(load, lines))
        {
          return false;
        }

        const auto after = customlines.get_customline_data("AFTER", keyword, load_id);
        lines.insert(lines.end(), after.begin(), after.end());
      }
    }

    return true;
  }

  template<typename LoadType>
  bool append_direct_loads(
    CoreSteps& steps, LoadType& loads, CoreCustomLines& customlines,
    const std::vector<int>& assignment_ids, std::vector<std::string>& lines,
    int load_type, const std::string& keyword,
    const std::string& error_label, bool require_data)
  {
    const auto append_data = [&](const std::vector<int>& load, std::vector<std::string>& output)
    {
      const int load_id = load[0];
      const std::string data = loads.get_load_export(load_id);

      if (require_data && data.empty())
      {
        PRINT_ERROR("Failed exporting %s %d.\n", error_label.c_str(), load_id);
        return false;
      }

      output.push_back(data);
      return true;
    };

    return append_selected_loads(steps, loads, customlines, assignment_ids, lines, load_type, keyword, append_data);
  }
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsForces& loads, CoreCustomLines& customlines,
  const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
{
  return append_direct_loads(steps, loads, customlines, assignment_ids, lines, 1, "FORCE", "force", true);
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsPressures& loads, CoreCustomLines& customlines,
  const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
{
  return append_direct_loads(steps, loads, customlines, assignment_ids, lines, 2, "PRESSURE", "pressure", true);
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsHeatfluxes& loads, CoreCustomLines& customlines,
  const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
{
  return append_direct_loads(steps, loads, customlines, assignment_ids, lines, 3, "HEATFLUX", "heatflux", true);
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsGravity& loads, CoreCustomLines& customlines,
  const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
{
  return append_direct_loads(steps, loads, customlines, assignment_ids, lines, 4, "GRAVITY", "gravity", false);
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsCentrifugal& loads, CoreCustomLines& customlines,
  const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
{
  return append_direct_loads(steps, loads, customlines, assignment_ids, lines, 5, "CENTRIFUGAL", "centrifugal", false);
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsFilm& loads, CoreCustomLines& customlines,
  CoreSidesetFaces& sideset_faces, const std::vector<int>& assignment_ids,
  std::vector<std::string>& lines)
{
  const auto append_data = [&](const std::vector<int>& load, std::vector<std::string>& output)
  {
    const auto faces = sideset_faces.get_sideset_face(load[4]);

    for (const auto& face : faces)
    {
      const int index = loads.get_loads_data_id_from_load_id(load[0]);
      output.push_back(loads.get_load_export(load[0]));

      const int temperature_index = loads.get_temperature_data_id_from_temperature_id(loads.loads_data[index][5]);
      const int coefficient_index = loads.get_coefficient_data_id_from_coefficient_id(loads.loads_data[index][6]);
      const std::string& temperature = loads.temperature_data[temperature_index][1];
      const std::string& coefficient = loads.coefficient_data[coefficient_index][1];

      output.push_back(face[1] + ",F" + face[2] + "," + temperature + "," + coefficient);
    }

    return true;
  };

  return append_selected_loads(steps, loads, customlines, assignment_ids, lines, 7, "FILM", append_data);
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsRadiation& loads, CoreCustomLines& customlines,
  CoreSidesetFaces& sideset_faces, const std::vector<int>& assignment_ids,
  std::vector<std::string>& lines)
{
  const auto append_data = [&](const std::vector<int>& load, std::vector<std::string>& output)
  {
    const auto faces = sideset_faces.get_sideset_face(load[4]);

    for (const auto& face : faces)
    {
      const int index = loads.get_loads_data_id_from_load_id(load[0]);
      const std::string header = loads.get_load_export(load[0]);

      if (header.empty())
      {
        return false;
      }

      output.push_back(header);

      const int temperature_index = loads.get_temperature_data_id_from_temperature_id(loads.loads_data[index][5]);
      const int emissivity_index = loads.get_emissivity_data_id_from_emissivity_id(loads.loads_data[index][6]);
      const std::string& temperature = loads.temperature_data[temperature_index][1];
      const std::string& emissivity = loads.emissivity_data[emissivity_index][1];
      const std::string suffix = loads.loads_data[index][11] == 1 ? "CR," : ",";

      output.push_back(face[1] + ",R" + face[2] + suffix + temperature + "," + emissivity);
    }

    return true;
  };

  return append_selected_loads(steps, loads, customlines, assignment_ids, lines, 8, "RADIATION", append_data);
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsSurfaceTraction& loads, CoreCustomLines& customlines,
  const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
{
  return append_direct_loads(steps, loads, customlines, assignment_ids, lines, 9, "SURFACETRACTION", "surface traction", false);
}

bool CoreStepLoadExport::append(
  CoreSteps& steps, CoreLoadsBodyHeatflux& loads, CoreCustomLines& customlines,
  const std::vector<int>& assignment_ids, std::vector<std::string>& lines)
{
  return append_direct_loads(steps, loads, customlines, assignment_ids, lines, 10, "BODYHEATFLUX", "body heat flux", false);
}
