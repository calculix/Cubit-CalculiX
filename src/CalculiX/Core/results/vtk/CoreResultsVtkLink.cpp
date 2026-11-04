#include "CoreResultsVtkLink.hpp"
#include "CoreResultsInpReader.hpp"
#include "CoreResultsVtkElements.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "MeshExportInterface.hpp"
#include <algorithm>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <utility>

namespace
{
  using Order = CoreResultsVtkElements::NodeOrder;

  template<class T>
  const T* entity(const std::vector<T>& values, int id)
  {
    const auto found = std::lower_bound(values.begin(), values.end(), id, [](const T& value, int key)
    {
      return value.id < key;
    });
    return found != values.end() && found->id == id ? &*found : nullptr;
  }

  bool close(double a, double b, double absolute, double relative)
  {
    return std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= absolute + relative * std::max(std::abs(a), std::abs(b));
  }

  template<class T>
  bool sort_entities(std::vector<T>& values)
  {
    std::sort(values.begin(), values.end(), [](const T& a, const T& b)
    {
      return a.id < b.id;
    });
    for (std::size_t i = 0; i < values.size(); ++i)
    {
      if (values[i].id <= 0 || (i > 0 && values[i - 1].id == values[i].id))
      {
        return false;
      }
    }
    return true;
  }

  bool frd_mesh(const CoreResultsFrd& frd, CoreResultsVtkMesh& mesh, std::string& error)
  {
    for (const auto& row : frd.nodes)
    {
      if (row.size() < 2 || row[1] < 0 || static_cast<std::size_t>(row[1]) >= frd.nodes_coords.size() || frd.nodes_coords[row[1]].size() != 3)
      {
        error = "Invalid FRD node mapping.";
        return false;
      }
      const auto& coordinates = frd.nodes_coords[row[1]];
      if (!std::isfinite(coordinates[0]) || !std::isfinite(coordinates[1]) || !std::isfinite(coordinates[2]))
      {
        error = "Non-finite FRD mesh coordinate.";
        return false;
      }
      mesh.nodes.push_back({row[0], {{coordinates[0], coordinates[1], coordinates[2]}}});
    }
    if (!sort_entities(mesh.nodes))
    {
      error = "Invalid or duplicate FRD node IDs.";
      return false;
    }
    for (const auto& row : frd.elements)
    {
      if (row.size() < 3 || row[2] < 0 || static_cast<std::size_t>(row[2]) >= frd.elements_connectivity.size())
      {
        error = "Invalid FRD element mapping.";
        return false;
      }
      CoreResultsVtkElement element;
      element.id = row[0];
      element.frd_type = row[1];
      CoreResultsVtkElements::Description description;
      if (!CoreResultsVtkElements::describe_frd(element.frd_type, description, error) || !CoreResultsVtkElements::reorder(description, Order::Frd, Order::CalculiX, frd.elements_connectivity[row[2]], element.connectivity, error))
      {
        return false;
      }
      for (int node : element.connectivity)
      {
        if (!entity(mesh.nodes, node))
        {
          error = "FRD element references missing node " + std::to_string(node) + ".";
          return false;
        }
      }
      mesh.elements.push_back(std::move(element));
    }
    if (mesh.nodes.empty() || mesh.elements.empty() || !sort_entities(mesh.elements))
    {
      error = "Empty FRD mesh or invalid/duplicate element IDs.";
      return false;
    }
    return true;
  }

  bool capture_model(CoreResultsVtkMesh& mesh, std::string& error)
  {
    auto* exporter = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
    if (!exporter || !exporter->initialize_export())
    {
      error = "Current model mesh exporter is unavailable.";
      return false;
    }
    CalculiXMeshInterface mesh_interface;
    const int buffer_size = 100;
    std::vector<double> x(buffer_size), y(buffer_size), z(buffer_size);
    std::vector<int> ids(buffer_size);
    int start = 0;
    int count = 0;
    while ((count = exporter->get_coords(start, buffer_size, x, y, z, ids)) > 0)
    {
      for (int i = 0; i < count; ++i)
      {
        mesh.nodes.push_back({ids[i], {{x[i], y[i], z[i]}}});
      }
      start += count;
    }
    std::map<int, std::string> formulations;
    for (int block : mesh_interface.get_blocks())
    {
      CoreResultsVtkGroup group;
      group.kind = CoreResultsVtkGroupKind::Block;
      group.model_id = block;
      group.name = mesh_interface.get_block_name(block);
      group.ids = mesh_interface.get_block_element_ids(block);
      const std::string type = mesh_interface.get_ccx_element_type(block);
      for (int id : group.ids)
      {
        const auto found = formulations.find(id);
        if (found != formulations.end() && found->second != type)
        {
          error = "Conflicting current-model element formulations.";
          return false;
        }
        formulations[id] = type;
      }
      mesh.groups.push_back(std::move(group));
    }
    std::map<int, CoreResultsVtkElement> elements;
    for (const auto& row : mesh_interface.get_element_id_type_connectivity())
    {
      if (row.size() < 3 || formulations.count(row[0]) == 0)
      {
        continue; // Non-exported/free entities are not the analysis mesh.
      }
      CoreResultsVtkElement element;
      element.id = row[0];
      element.calculix_type = formulations.at(row[0]);
      CoreResultsVtkElements::Description description;
      if (!CoreResultsVtkElements::describe(element.calculix_type, description, error) || !CoreResultsVtkElements::reorder(description, Order::Exodus, Order::CalculiX, std::vector<int>(row.begin() + 2, row.end()), element.connectivity, error))
      {
        return false;
      }
      element.frd_type = description.frd_type;
      const auto old = elements.find(element.id);
      if (old != elements.end() && old->second.connectivity != element.connectivity)
      {
        error = "Conflicting current-model connectivity.";
        return false;
      }
      elements[element.id] = std::move(element);
    }
    for (auto& entry : elements)
    {
      mesh.elements.push_back(std::move(entry.second));
    }
    for (int id : CubitInterface::get_nodeset_id_list())
    {
      CoreResultsVtkGroup group;
      group.kind = CoreResultsVtkGroupKind::Nodeset;
      group.model_id = id;
      group.name = mesh_interface.get_nodeset_name(id);
      group.ids = CubitInterface::get_nodeset_nodes_inclusive(id);
      mesh.groups.push_back(std::move(group));
    }
    std::vector<SidesetHandle> sidesets;
    exporter->get_sideset_list(sidesets);
    for (const auto& sideset : sidesets)
    {
      CoreResultsVtkGroup group;
      group.kind = CoreResultsVtkGroupKind::Surface;
      group.model_id = exporter->id_from_handle(sideset);
      group.name = mesh_interface.get_sideset_name(group.model_id);
      std::vector<ElementHandle> handles;
      std::vector<ElementType> types;
      std::vector<int> sides;
      start = 0;
      while ((count = exporter->get_sideset_sides(sideset, start, buffer_size, handles, types, sides)) > 0)
      {
        for (int i = 0; i < count; ++i)
        {
          int local_id = 0;
          exporter->get_element_id(handles[i], local_id);
          const std::string kind = mesh_interface.get_cubit_element_type_entity(exporter->get_element_type_name(types[i]));
          const int id = CubitInterface::get_global_element_id(kind, local_id);
          const auto* element = entity(mesh.elements, id);
          std::string side;
          if (!element || !CoreResultsVtkElements::exodus_side(element->calculix_type, sides[i], side, error))
          {
            error = "Current-model surface cannot be resolved to the analysis mesh.";
            return false;
          }
          group.sides.push_back({id, side});
        }
        start += count;
      }
      mesh.groups.push_back(std::move(group));
    }
    if (!sort_entities(mesh.nodes) || !sort_entities(mesh.elements))
    {
      error = "Invalid current-model mesh IDs.";
      return false;
    }
    return true;
  }

  bool select_groups(CoreResultsVtkPlan& plan, std::string& error)
  {
    for (const auto kind : {CoreResultsVtkGroupKind::Block, CoreResultsVtkGroupKind::Nodeset, CoreResultsVtkGroupKind::Surface})
    {
      const auto& requested = kind == CoreResultsVtkGroupKind::Block ? plan.options.block_ids : kind == CoreResultsVtkGroupKind::Nodeset ? plan.options.nodeset_ids : plan.options.sideset_ids;
      for (int id : requested)
      {
        const auto found = std::find_if(plan.mesh.groups.begin(), plan.mesh.groups.end(), [&](const CoreResultsVtkGroup& group)
        {
          return group.kind == kind && group.model_id == id;
        });
        if (found == plan.mesh.groups.end())
        {
          error = "Requested model group " + std::to_string(id) + " is not available from the selected link source. INP groups are identified by name, not Cubit ID.";
          return false;
        }
      }
      plan.mesh.groups.erase(std::remove_if(plan.mesh.groups.begin(), plan.mesh.groups.end(), [&](const CoreResultsVtkGroup& group)
      {
        return group.kind == kind && !requested.empty() && std::find(requested.begin(), requested.end(), group.model_id) == requested.end();
      }), plan.mesh.groups.end());
    }
    return true;
  }

  bool frame(CoreResultsVtkPlan& plan, int step, int increment, int total, double time, bool frd, std::size_t& index, std::string& error)
  {
    if (!std::isfinite(time))
    {
      error = "Non-finite result time.";
      return false;
    }
    std::vector<std::size_t> matches;
    for (std::size_t i = 0; i < plan.increments.size(); ++i)
    {
      const auto& candidate = plan.increments[i];
      const bool same_time = close(candidate.time, time, plan.options.time_absolute_tolerance, plan.options.time_relative_tolerance);
      const bool identified = step > 0 && increment > 0 && candidate.step > 0 && candidate.increment > 0;
      const bool same_id = identified && candidate.step == step && candidate.increment == increment;
      if (frd)
      {
        if (candidate.total_increment == total && candidate.step == step && candidate.increment == increment && same_time)
        {
          matches.push_back(i);
        }
      }
      else if ((same_id && same_time) || (!identified && same_time))
      {
        matches.push_back(i);
      }
    }
    if (matches.size() > 1)
    {
      error = "Ambiguous FRD/DAT increment match. Mode/analysis metadata is required; reader coverage revision is deferred.";
      return false;
    }
    if (matches.empty())
    {
      CoreResultsVtkIncrementPlan increment_plan;
      increment_plan.step = step;
      increment_plan.increment = increment;
      increment_plan.total_increment = frd ? total : -1;
      increment_plan.time = time;
      index = plan.increments.size();
      plan.increments.push_back(std::move(increment_plan));
    }
    else
    {
      index = matches.front();
    }
    return true;
  }

  bool fields(const CoreResultsFrd& frd, const CoreResultsDat& dat, CoreResultsVtkPlan& plan, std::string& error)
  {
    std::map<std::string, std::size_t> rule_sizes;
    if (plan.options.include_frd)
    {
      for (std::size_t i = 0; i < frd.result_blocks.size(); ++i)
      {
        const auto& block = frd.result_blocks[i];
        if (block.size() < 7 || block[4] < 0 || block[5] < 0 || block[6] < 0 || static_cast<std::size_t>(block[4]) >= frd.total_times.size() || static_cast<std::size_t>(block[5]) >= frd.result_block_type.size() || static_cast<std::size_t>(block[6]) >= frd.result_block_components.size() || static_cast<std::size_t>(block[6]) >= frd.result_block_data.size() || static_cast<std::size_t>(block[6]) >= frd.result_block_node_data.size())
        {
          error = "Invalid FRD field metadata.";
          return false;
        }
        std::size_t destination = 0;
        if (!frame(plan, block[1], block[2], block[3], frd.total_times[block[4]], true, destination, error))
        {
          return false;
        }
        CoreResultsVtkFieldPlan field;
        field.source_block = static_cast<int>(i);
        field.source_data = block[6];
        field.field.source = "FRD";
        field.field.name = frd.result_block_type[block[5]];
        // CCX frd.c writes nodal identifiers, including the slave-node ID for
        // CELS. Do not infer cell association from a field name or tuple count.
        field.field.association = FeaResults::Association::Node;
        field.field.components = frd.result_block_components[block[6]];
        if (field.field.components.empty())
        {
          error = "FRD field has no components.";
          return false;
        }
        for (const auto& row : frd.result_block_node_data[block[6]])
        {
          if (row.size() < 2 || row[1] < 0 || static_cast<std::size_t>(row[1]) >= frd.result_block_data[block[6]].size() || !entity(plan.mesh.nodes, row[0]) || frd.result_block_data[block[6]][row[1]].size() != field.field.components.size())
          {
            error = "FRD field references missing geometry or an invalid data row.";
            return false;
          }
        }
        plan.increments[destination].fields.push_back(std::move(field));
      }
    }
    if (!plan.options.include_dat)
    {
      return true;
    }
    for (std::size_t i = 0; i < dat.result_blocks.size(); ++i)
    {
      const auto& block = dat.result_blocks[i];
      if (block.size() < 7 || block[1] < 0 || block[2] < 0 || block[3] < 0 || block[4] < 0 || static_cast<std::size_t>(block[1]) >= dat.total_times.size() || static_cast<std::size_t>(block[2]) >= dat.result_block_type.size() || static_cast<std::size_t>(block[3]) >= dat.result_block_set.size() || static_cast<std::size_t>(block[4]) >= dat.result_block_components.size() || static_cast<std::size_t>(block[4]) >= dat.result_block_data.size() || static_cast<std::size_t>(block[4]) >= dat.result_block_c1_data.size())
      {
        error = "Invalid DAT field metadata.";
        return false;
      }
      const auto& mapping = dat.result_block_c1_data[block[4]];
      if (mapping.empty())
      {
        continue;
      }
      const int association = mapping.front().size() >= 3 ? mapping.front()[2] : 0;
      if (association < 1 || association > 3)
      {
        error = "Unknown DAT field association.";
        return false;
      }
      CoreResultsVtkFieldPlan field;
      field.source_block = static_cast<int>(i);
      field.source_data = block[4];
      field.component_offset = association == 1 ? 0 : association == 2 ? 2 : 1;
      field.field.source = "DAT";
      field.field.name = dat.result_block_type[block[2]];
      field.field.set_name = dat.result_block_set[block[3]];
      field.field.association = association == 1 ? FeaResults::Association::Node : association == 2 ? FeaResults::Association::IntegrationPoint : FeaResults::Association::Element;
      const auto& components = dat.result_block_components[block[4]];
      if (components.size() <= field.component_offset)
      {
        error = "Invalid DAT component layout.";
        return false;
      }
      field.field.components.assign(components.begin() + field.component_offset, components.end());
      CoreResultsVtkGroup group;
      group.name = field.field.set_name;
      group.kind = association == 1 ? CoreResultsVtkGroupKind::DatNodes : association == 2 ? CoreResultsVtkGroupKind::DatIntegrationPoints : CoreResultsVtkGroupKind::DatElements;
      for (const auto& row : mapping)
      {
        if (row.size() < 3 || row[2] != association || row[1] < 0 || static_cast<std::size_t>(row[1]) >= dat.result_block_data[block[4]].size())
        {
          error = "Invalid or mixed DAT field mapping.";
          return false;
        }
        if (dat.result_block_data[block[4]][row[1]].size() != components.size())
        {
          error = "DAT values do not match the component definition.";
          return false;
        }
        if (association == 1)
        {
          if (!entity(plan.mesh.nodes, row[0]))
          {
            error = "DAT node is absent from the shared job mesh.";
            return false;
          }
        }
        else
        {
          const auto* element = entity(plan.mesh.elements, row[0]);
          if (!element)
          {
            error = "DAT element is absent from the shared job mesh (expanded shell/beam mapping is not established).";
            return false;
          }
          if (association == 2)
          {
            auto size = rule_sizes.find(element->calculix_type);
            if (size == rule_sizes.end())
            {
              CoreResultsVtkElements::Description description;
              if (element->calculix_type.empty() || !CoreResultsVtkElements::describe(element->calculix_type, description, error) || description.integration_type.empty())
              {
                error = "Exact supported element formulation is required to locate DAT integration points for element " + std::to_string(row[0]) + ".";
                return false;
              }
              std::vector<ElementIntegration::IntegrationPoint> rule;
              if (!ElementIntegration::get_rule(description.integration_type, rule))
              {
                error = "Unsupported integration rule for DAT element " + std::to_string(row[0]) + ".";
                return false;
              }
              size = rule_sizes.emplace(element->calculix_type, rule.size()).first;
            }
            const auto& values = dat.result_block_data[block[4]][row[1]];
            if (values.size() < 2 || !std::isfinite(values[1]) || values[1] < 1 || values[1] > static_cast<double>(size->second) || std::floor(values[1]) != values[1] || values[0] != static_cast<double>(row[0]))
            {
              error = "Invalid DAT element/integration-point identifier.";
              return false;
            }
          }
        }
        group.ids.push_back(row[0]);
      }
      std::sort(group.ids.begin(), group.ids.end());
      group.ids.erase(std::unique(group.ids.begin(), group.ids.end()), group.ids.end());
      auto existing = std::find_if(plan.mesh.groups.begin(), plan.mesh.groups.end(), [&](const CoreResultsVtkGroup& candidate)
      {
        return candidate.kind == group.kind && candidate.name == group.name;
      });
      if (existing == plan.mesh.groups.end())
      {
        plan.mesh.groups.push_back(std::move(group));
      }
      else
      {
        existing->ids.insert(existing->ids.end(), group.ids.begin(), group.ids.end());
        std::sort(existing->ids.begin(), existing->ids.end());
        existing->ids.erase(std::unique(existing->ids.begin(), existing->ids.end()), existing->ids.end());
      }
      std::size_t destination = 0;
      if (!frame(plan, block[5], block[6], -1, dat.total_times[block[1]], false, destination, error))
      {
        return false;
      }
      plan.increments[destination].fields.push_back(std::move(field));
    }
    for (std::size_t i = 0; i < dat.buckle_data.size(); ++i)
    {
      plan.tables.push_back({CoreResultsVtkTablePlan::Kind::Buckling, i});
    }
    for (std::size_t i = 0; i < dat.result_section_data.size(); ++i)
    {
      plan.tables.push_back({CoreResultsVtkTablePlan::Kind::Section, i});
    }
    return true;
  }
}

bool CoreResultsVtkLink::compatible(const CoreResultsVtkMesh& result, const CoreResultsVtkMesh& candidate, const CoreResultsVtkOptions& options, std::string& reason)
{
  if (result.nodes.empty() || result.elements.empty() || !std::isfinite(options.coordinate_absolute_tolerance) || !std::isfinite(options.coordinate_relative_tolerance) || options.coordinate_absolute_tolerance < 0.0 || options.coordinate_relative_tolerance < 0.0)
  {
    reason = "Empty reference mesh or invalid coordinate tolerances.";
    return false;
  }
  if (result.elements.size() != candidate.elements.size())
  {
    reason = "Analysis element counts differ.";
    return false;
  }
  for (const auto& node : result.nodes)
  {
    const auto* other = entity(candidate.nodes, node.id);
    if (!other)
    {
      reason = "Result node " + std::to_string(node.id) + " is absent.";
      return false;
    }
    for (std::size_t axis = 0; axis < 3; ++axis)
    {
      if (!close(node.coordinates[axis], other->coordinates[axis], options.coordinate_absolute_tolerance, options.coordinate_relative_tolerance))
      {
        reason = "Coordinates differ at result node " + std::to_string(node.id) + ".";
        return false;
      }
    }
  }
  for (const auto& element : result.elements)
  {
    const auto* other = entity(candidate.elements, element.id);
    if (!other || other->frd_type != element.frd_type || other->connectivity != element.connectivity)
    {
      reason = "Topology/connectivity differs at result element " + std::to_string(element.id) + ".";
      return false;
    }
  }
  reason.clear();
  return true;
}

bool CoreResultsVtkLink::prepare(int job_id, const CoreResultsFrd& frd, const CoreResultsDat& dat, const std::string& input_path, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& output) const
{
  const double tolerances[] = {options.coordinate_absolute_tolerance, options.coordinate_relative_tolerance, options.time_absolute_tolerance, options.time_relative_tolerance};
  for (double tolerance : tolerances)
  {
    if (!std::isfinite(tolerance) || tolerance < 0.0)
    {
      PRINT_ERROR("Invalid VTK linking tolerance.\n");
      return false;
    }
  }
  if (job_id <= 0 || (!options.include_frd && !options.include_dat))
  {
    PRINT_ERROR("Invalid VTK conversion-plan request.\n");
    return false;
  }
  CoreResultsVtkPlan result;
  result.job_id = job_id;
  result.options = options;
  result.input_path = input_path;
  std::string reason;
  CoreResultsVtkMesh reference;
  if (!frd_mesh(frd, reference, reason))
  {
    PRINT_ERROR("Cannot plan VTK conversion: %s\n", reason.c_str());
    return false;
  }
  CoreResultsVtkMesh current;
  if (capture_model(current, reason) && compatible(reference, current, options, reason))
  {
    result.link_source = FeaResults::LinkSource::CurrentModel;
    result.mesh = std::move(current);
    // Even with unchanged topology the user may have changed full/reduced
    // integration since the solve. A matching run INP is authoritative for the
    // formulation; the current model still supplies grouping information.
    if (!input_path.empty())
    {
      CoreResultsVtkMesh input;
      CoreResultsInpReader reader;
      if (reader.read(input_path, input, result.input_files, reason) && compatible(reference, input, options, reason))
      {
        for (auto& element : result.mesh.elements)
        {
          element.calculix_type = entity(input.elements, element.id)->calculix_type;
        }
      }
      else
      {
        result.input_files.clear();
        result.diagnostics.push_back("Run INP did not establish formulations; using the matched current-model definitions: " + reason);
      }
    }
  }
  else
  {
    result.diagnostics.push_back("Current model not linked: " + reason);
    CoreResultsVtkMesh input;
    CoreResultsInpReader reader;
    if (!input_path.empty() && reader.read(input_path, input, result.input_files, reason) && compatible(reference, input, options, reason))
    {
      result.link_source = FeaResults::LinkSource::JobInput;
      result.mesh = std::move(input);
    }
    else
    {
      result.diagnostics.push_back("Job INP not linked: " + (input_path.empty() ? std::string("no input path supplied.") : reason));
      result.input_files.clear();
      result.mesh = std::move(reference);
    }
  }
  // Keep solver-output coordinates authoritative after a tolerant model/INP
  // match; retain extra source nodes only when required by captured groups.
  for (const auto& node : reference.nodes)
  {
    auto found = std::lower_bound(result.mesh.nodes.begin(), result.mesh.nodes.end(), node.id, [](const CoreResultsVtkNode& value, int id)
    {
      return value.id < id;
    });
    if (found != result.mesh.nodes.end() && found->id == node.id)
    {
      found->coordinates = node.coordinates;
    }
  }
  if (!select_groups(result, reason) || !fields(frd, dat, result, reason))
  {
    PRINT_ERROR("Cannot plan VTK conversion: %s\n", reason.c_str());
    return false;
  }
  if (result.increments.empty() && result.tables.empty())
  {
    PRINT_ERROR("Cannot plan VTK conversion: no requested result fields.\n");
    return false;
  }
  result.layout = result.mesh.groups.empty() ? FeaResults::DatasetLayout::SingleGrid : FeaResults::DatasetLayout::Collection;
  output = std::move(result);
  return true;
}
