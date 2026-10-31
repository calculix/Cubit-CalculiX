#include "CoreStepExport.hpp"
#include "CoreSteps.hpp"
#include "CoreStepLoadExport.hpp"
#include "CoreStepBCExport.hpp"
#include "CoreStepOutputExport.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/mesh/CoreSidesetFaces.hpp"

#include <vector>

std::string CoreStepExport::get_export(const CoreStepExportData& data)
{
  std::vector<std::string> lines = {
    "********************************** S T E P S ****************************"
  };

  for (const auto& step : data.steps.steps_data)
  {
    lines.push_back(data.steps.get_step_export(step[0]));
    const auto load_ids = data.steps.get_load_data_ids_from_loads_id(step[5]);

    if (!CoreStepLoadExport::append(data.steps, data.loadsforces, data.customlines, load_ids, lines) ||
        !CoreStepLoadExport::append(data.steps, data.loadspressures, data.customlines, load_ids, lines) ||
        !CoreStepLoadExport::append(data.steps, data.loadsheatfluxes, data.customlines, load_ids, lines) ||
        !CoreStepLoadExport::append(data.steps, data.loadsgravity, data.customlines, load_ids, lines) ||
        !CoreStepLoadExport::append(data.steps, data.loadscentrifugal, data.customlines, load_ids, lines) ||
        !CoreStepLoadExport::append(data.steps, data.loadsfilm, data.customlines, data.sideset_faces, load_ids, lines) ||
        !CoreStepLoadExport::append(data.steps, data.loadsradiation, data.customlines, data.sideset_faces, load_ids, lines) ||
        !CoreStepLoadExport::append(data.steps, data.loadssurfacetraction, data.customlines, load_ids, lines) ||
        !CoreStepLoadExport::append(data.steps, data.loadsbodyheatflux, data.customlines, load_ids, lines))
    {
      return {};
    }

    const auto bc_ids = data.steps.get_bc_data_ids_from_bcs_id(step[6]);

    if (!CoreStepBCExport::append(data.steps, data.bcsdisplacements, data.customlines, data.mesh, bc_ids, lines) ||
        !CoreStepBCExport::append(data.steps, data.bcstemperatures, data.customlines, bc_ids, lines))
    {
      return {};
    }

    CoreStepOutputExport::append_history(data.steps, data.historyoutputs, step[7], lines);
    CoreStepOutputExport::append_field(data.steps, data.fieldoutputs, step[8], lines);

    const auto before = data.customlines.get_customline_data("BEFORE", "STEP_END", step[0]);
    lines.insert(lines.end(), before.begin(), before.end());
    lines.push_back("*END STEP");

    const auto after = data.customlines.get_customline_data("AFTER", "STEP_END", step[0]);
    lines.insert(lines.end(), after.begin(), after.end());
  }

  std::string output;

  for (const auto& line : lines)
  {
    output += line + "\n";
  }

  data.sideset_faces.clear();
  return output;
}
