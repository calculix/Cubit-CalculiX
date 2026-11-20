#include "CoreStepExport.hpp"
#include "Core/connections/CoreConnections.hpp"
#include "Core/connections/CoreDiscreteElementExport.hpp"
#include "Core/loads/CoreLoadsForces.hpp"
#include "Core/units/CalculiXPhysicalModelUnits.hpp"
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include "CoreSteps.hpp"
#include "CoreStepLoadExport.hpp"
#include "CoreStepBCExport.hpp"
#include "CoreStepOutputExport.hpp"
#include "Core/export/CoreCustomLines.hpp"
#include "Core/mesh/CoreSidesetFaces.hpp"
#include "CubitMessage.hpp"

#include <set>
#include <vector>

std::string CoreStepExport::get_export(const CoreStepExportData& data)
{
  std::vector<std::string> lines = {
    "********************************** S T E P S ****************************"
  };

  int solverStep = 0;
  for (const auto& step : data.steps.steps_data)
  {
    ++solverStep;
    std::string quantity;
    std::string error;
    if (step.size() < 9 || !data.steps.get_step_axis_quantity(step[0], quantity, error))
    {
      PRINT_ERROR("Cannot export step axis: %s\n", error.c_str());
      return {};
    }
    if (!quantity.empty())
    {
      lines.push_back("** CUBIT_SIM_STEP_AXIS=" + std::to_string(solverStep) + ";" + quantity);
    }
    const auto definition = data.steps.get_step_export(step[0]);
    if (definition.empty())
    {
      return {};
    }
    lines.push_back(definition);
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

    if (!data.connections.attachments().empty())
    {
      auto forces = data.loadsforces;
      if (!forces.unit_view && !CalculiXPhysicalModelUnits::convert(forces,CalculiXUnitView::Solver,error))
      {
        PRINT_ERROR("%s\n",error.c_str());
        return {};
      }
      for (int link : load_ids)
      {
        const auto& assignment = data.steps.loads_data.at(link);
        if (assignment[1] != 1)
        {
          continue;
        }
        const int index = forces.get_loads_data_id_from_load_id(assignment[2]);
        if (index < 0 || forces.loads_data[index].size() < 8)
        {
          continue;
        }
        const auto& record = forces.loads_data[index];
        for (const auto& entry : data.connections.attachments())
        {
          if (entry.second.point != record[7])
          {
            continue;
          }
          const auto& values = forces.force_data.at(forces.get_force_data_id_from_force_id(record[5]));
          QJsonArray components;
          for (int i = 1; i <= 6; ++i)
          {
            components.append(values.at(i));
          }
          const auto name = forces.name_data.at(forces.get_name_data_id_from_name_id(record[6])).at(1);
          QJsonObject json{{"attachment",entry.first},{"step",solverStep},{"load",record[0]},{"name",QString::fromStdString(name)},
            {"parameters",QString::fromStdString(forces.get_load_parameter_export(record[0]))},{"components",components}};
          lines.push_back("** CUBIT_SIM_DA_LOAD " + QJsonDocument(json).toJson(QJsonDocument::Compact).toBase64().toStdString());
        }
      }
    }
    const auto bc_ids = data.steps.get_bc_data_ids_from_bcs_id(step[6]);

    if (!CoreStepBCExport::append(data.steps, data.bcsdisplacements, data.customlines, data.mesh, bc_ids, lines) ||
        !CoreStepBCExport::append(data.steps, data.bcstemperatures, data.customlines, bc_ids, lines))
    {
      return {};
    }

    CoreStepOutputExport::append_history(data.steps, data.historyoutputs, step[7], lines);
    if (step[3] == 2 || step[3] == 6 || step[3] == 7 || step[3] == 8)
    {
      for (const auto& entry : data.connections.attachments())
      {
        const auto& a = entry.second;
        const int history = data.connections.points().at(a.point).history_step;
        if (history != -1 && history != step[0])
        {
          continue;
        }
        const auto prefix = "CCX_DA_" + std::to_string(a.id);
        if (a.resultant)
        {
          lines.push_back("*SECTION PRINT,NAME=" + prefix + "_FACE,SURFACE=" + prefix + "_FACE");
          lines.push_back("SOF");
        }
        if (a.mean)
        {
          lines.push_back("*NODE PRINT,NSET=" + prefix + "_NODES");
          lines.push_back("U");
        }
      }
      std::set<int> joint_points;
      for (const auto& entry : data.connections.joints())
      {
        const auto& joint = entry.second;
        if (joint.history_step == -1 || joint.history_step == step[0])
        {
          joint_points.insert(joint.point_a);
          joint_points.insert(joint.point_b);
        }
      }
      for (const auto& entry : data.connections.bodies())
      {
        const auto& point = data.connections.points().at(entry.second.point);
        if (point.history_step == -1 || point.history_step == step[0] || joint_points.count(point.id))
        {
          const auto prefix = "*NODE PRINT,NSET=CCX_RP_" + std::to_string(point.id);
          for (const auto* suffix : {"_REF", "_ROT"})
          {
            lines.push_back(prefix + suffix + ",GLOBAL=YES");
            lines.push_back("U, RF");
          }
        }
      }
    }
    if (CoreDiscreteElementExport::needs_history(data.connections, step[0], step[3]))
    {
      lines.push_back("*NODE PRINT,NSET=" + CoreDiscreteElementExport::history_set(solverStep) + ",GLOBAL=YES");
      lines.push_back("U");
    }
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
