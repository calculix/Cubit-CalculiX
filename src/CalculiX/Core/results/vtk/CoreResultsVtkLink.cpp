#include "CoreResultsVtkLink.hpp"
#include "CoreResultsInpReader.hpp"
#include "CoreResultsVtkElements.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Core/results/ResultReadNumbers.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "MeshExportInterface.hpp"
#include <algorithm>
#include <array>
#include <charconv>
#include <cctype>
#include <cmath>
#include <limits>
#include <map>
#include <set>
#include <string_view>
#include <system_error>
#include <utility>

namespace
{
  using Order = CoreResultsVtkElements::NodeOrder;

  template<class T>
  const T* entity(const std::vector<T>& values, int id) {
    const auto found = std::lower_bound(values.begin(), values.end(), id, [](const T& value, int key) {
      return value.id < key;
    });
    if (found != values.end() && found->id == id) {
      return &*found;
    }
    return nullptr;
  }

  bool close(double a, double b, double absolute, double relative) {
    return std::isfinite(a) && std::isfinite(b) && std::abs(a - b) <= absolute + relative * std::max(std::abs(a), std::abs(b));
  }

  /** Compare a parsed ASCII FRD coordinate with a model/INP coordinate.
   * CalculiX writes coordinates using %12.5E after conversion to float.
   * Accept the configured tolerance or an exact match after that conversion.
   * This normalization applies only to mesh coordinates.
   */
  bool frd_coordinate_matches(double reference, double candidate, double absolute, double relative) {
    if (close(reference, candidate, absolute, relative)) {
      return true;
    }
    if (!std::isfinite(reference) || !std::isfinite(candidate) ||
      std::abs(candidate) > std::numeric_limits<float>::max()) {
      return false;
    }

    const double solver_coordinate = static_cast<double>(static_cast<float>(candidate));
    char buffer[64];
    const auto formatted = std::to_chars(buffer, buffer + sizeof(buffer), solver_coordinate, std::chars_format::scientific, 5);
    if (formatted.ec != std::errc{}) {
      return false;
    }

    double written_coordinate = 0.0;
    const std::string_view text(buffer, static_cast<std::size_t>(formatted.ptr - buffer));
    return ResultReadNumbers::real(text, written_coordinate) && reference == written_coordinate;
  }

  template<class T>
  bool sort_entities(std::vector<T>& values) {
    std::sort(values.begin(), values.end(), [](const T& a, const T& b) {
      return a.id < b.id;
    });
    for (std::size_t i = 0; i < values.size(); ++i) {
      if (values[i].id <= 0 || (i > 0 && values[i - 1].id == values[i].id)) {
        return false;
      }
    }
    return true;
  }

  bool frd_mesh(const CoreResultsFrd& frd, CoreResultsVtkMesh& mesh, std::string& error) {
    for (const auto& row : frd.nodes) {
      if (row.size() < 2 || row[1] < 0 || static_cast<std::size_t>(row[1]) >= frd.nodes_coords.size() || frd.nodes_coords[row[1]].size() != 3) {
        error = "Invalid FRD node mapping.";
        return false;
      }
      const auto& coordinates = frd.nodes_coords[row[1]];
      if (!std::isfinite(coordinates[0]) || !std::isfinite(coordinates[1]) || !std::isfinite(coordinates[2])) {
        error = "Non-finite FRD mesh coordinate.";
        return false;
      }
      mesh.nodes.push_back({row[0], {{coordinates[0], coordinates[1], coordinates[2]}}});
    }
    if (!sort_entities(mesh.nodes)) {
      error = "Invalid or duplicate FRD node IDs.";
      return false;
    }
    for (const auto& row : frd.elements) {
      if (row.size() < 3 || row[2] < 0 || static_cast<std::size_t>(row[2]) >= frd.elements_connectivity.size()) {
        error = "Invalid FRD element mapping.";
        return false;
      }
      CoreResultsVtkElement element;
      element.id = row[0];
      element.frd_type = row[1];
      if (row.size() >= 4) {
        element.solver_material_id = row[3];
      }
      CoreResultsVtkElements::Description description;
      if (!CoreResultsVtkElements::describe_frd(element.frd_type, description, error) || !CoreResultsVtkElements::reorder(description, Order::Frd, Order::CalculiX, frd.elements_connectivity[row[2]], element.connectivity, error)) {
        return false;
      }
      for (int node : element.connectivity) {
        if (!entity(mesh.nodes, node)) {
          error = "FRD element references missing node " + std::to_string(node) + ".";
          return false;
        }
      }
      mesh.elements.push_back(std::move(element));
    }
    if (mesh.nodes.empty() || !sort_entities(mesh.elements)) {
      error = "Empty FRD mesh or invalid/duplicate element IDs.";
      return false;
    }
    return true;
  }

  bool capture_model(CoreResultsVtkMesh& mesh, const std::string& source_system, std::string& error)
  {
    if (source_system.empty())
    {
      error = "Recorded result units are unavailable.";
      return false;
    }

    double geometryLengthSI = 0.0;
    double coordinateScale = 0.0;

    if (!framework_units().geometry_to_si(1.0, geometryLengthSI, error) ||
        !framework_units().system_from_si(geometryLengthSI, SimUnits::Length,
          source_system, coordinateScale, error))
    {
      return false;
    }

    if (!std::isfinite(coordinateScale) || coordinateScale <= 0.0)
    {
      error = "Invalid current-model coordinate scale.";
      return false;
    }

    CalculiXMeshInterface mesh_interface;
    const auto blocks = mesh_interface.get_blocks();
    if (blocks.empty()) {
      // An absent current analysis mesh is a normal INP/FRD fallback case.
      // Do not initialize the exporter, which would emit Cubit errors.
      error = "Current model has no analysis blocks.";
      return false;
    }
    auto* exporter = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
    if (!exporter || !exporter->initialize_export()) {
      error = "Current model mesh exporter is unavailable.";
      return false;
    }
    const int buffer_size = 100;
    std::vector<double> x(buffer_size), y(buffer_size), z(buffer_size);
    std::vector<int> ids(buffer_size);
    int start = 0;
    int count = 0;
    while ((count = exporter->get_coords(start, buffer_size, x, y, z, ids)) > 0) {
      for (int i = 0; i < count; ++i)
      {
        const std::array<double, 3> coordinates = {
          x[i] * coordinateScale,
          y[i] * coordinateScale,
          z[i] * coordinateScale
        };

        for (double coordinate : coordinates)
        {
          if (!std::isfinite(coordinate))
          {
            error = "Non-finite converted current-model coordinate.";
            return false;
          }
        }

        mesh.nodes.push_back({ids[i], coordinates});
      }
      start += count;
    }
    std::map<int, std::string> formulations;
    for (int block : blocks) {
      CoreResultsVtkGroup group;
      group.kind = CoreResultsVtkGroupKind::Block;
      group.model_id = block;
      group.name = mesh_interface.get_block_name(block);
      group.ids = mesh_interface.get_block_element_ids(block);
      // Resolve native assigned/default colors on the host thread before workers
      // convert the plan. Missing graphics information must not invalidate mesh linking.
      std::string nativeColor, colorError;
      if (mesh_interface.get_block_capture_color(block,nativeColor,colorError) &&
          nativeColor.size() == 7 && nativeColor.front() == '#')
      {
        std::array<double,3> rgb{};
        bool valid = true;
        for (std::size_t channel = 0; channel < rgb.size(); ++channel)
        {
          const char* first = nativeColor.data()+1+2*channel;
          unsigned int value = 0;
          const auto parsed = std::from_chars(first,first+2,value,16);
          if (parsed.ec != std::errc{} || parsed.ptr != first+2 || value > 255)
          {
            valid = false;
            break;
          }
          rgb[channel] = static_cast<double>(value)/255.0;
        }
        if (valid)
        {
          group.solid_color = rgb;
        }
      }
      const std::string type = mesh_interface.get_ccx_element_type(block);
      for (int id : group.ids) {
        const auto found = formulations.find(id);
        if (found != formulations.end() && found->second != type) {
          error = "Conflicting current-model element formulations.";
          return false;
        }
        formulations[id] = type;
      }
      mesh.groups.push_back(std::move(group));
    }
    std::map<int, CoreResultsVtkElement> elements;
    for (const auto& row : mesh_interface.get_element_id_type_connectivity()) {
      if (row.size() < 3 || formulations.count(row[0]) == 0) {
        continue; // Non-exported/free entities are not the analysis mesh.
      }
      CoreResultsVtkElement element;
      element.id = row[0];
      element.calculix_type = formulations.at(row[0]);
      CoreResultsVtkElements::Description description;
      if (!CoreResultsVtkElements::describe(element.calculix_type, description, error) || !CoreResultsVtkElements::reorder(description, Order::Exodus, Order::CalculiX, std::vector<int>(row.begin() + 2, row.end()), element.connectivity, error)) {
        return false;
      }
      element.frd_type = description.frd_type;
      const auto old = elements.find(element.id);
      if (old != elements.end() && old->second.connectivity != element.connectivity) {
        error = "Conflicting current-model connectivity.";
        return false;
      }
      elements[element.id] = std::move(element);
    }
    for (auto& entry : elements) {
      mesh.elements.push_back(std::move(entry.second));
    }
    for (int id : CubitInterface::get_nodeset_id_list()) {
      CoreResultsVtkGroup group;
      group.kind = CoreResultsVtkGroupKind::Nodeset;
      group.model_id = id;
      group.name = mesh_interface.get_nodeset_name(id);
      group.ids = CubitInterface::get_nodeset_nodes_inclusive(id);
      mesh.groups.push_back(std::move(group));
    }
    std::vector<SidesetHandle> sidesets;
    exporter->get_sideset_list(sidesets);
    for (const auto& sideset : sidesets) {
      CoreResultsVtkGroup group;
      group.kind = CoreResultsVtkGroupKind::Surface;
      group.model_id = exporter->id_from_handle(sideset);
      group.name = mesh_interface.get_sideset_name(group.model_id);
      std::vector<ElementHandle> handles;
      std::vector<ElementType> types;
      std::vector<int> sides;
      start = 0;
      while ((count = exporter->get_sideset_sides(sideset, start, buffer_size, handles, types, sides)) > 0) {
        for (int i = 0; i < count; ++i) {
          int local_id = 0;
          exporter->get_element_id(handles[i], local_id);
          const std::string kind = mesh_interface.get_cubit_element_type_entity(exporter->get_element_type_name(types[i]));
          const int id = CubitInterface::get_global_element_id(kind, local_id);
          const auto* element = entity(mesh.elements, id);
          std::string side;
          if (!element || !CoreResultsVtkElements::exodus_side(element->calculix_type, sides[i], side, error)) {
            error = "Current-model surface cannot be resolved to the analysis mesh.";
            return false;
          }
          group.sides.push_back({id, side});
        }
        start += count;
      }
      mesh.groups.push_back(std::move(group));
    }
    if (!sort_entities(mesh.nodes) || !sort_entities(mesh.elements)) {
      error = "Invalid current-model mesh IDs.";
      return false;
    }
    return true;
  }

  bool add_discrete_groups(CoreResultsVtkMesh& mesh, std::string& error)
  {
    const auto& features = mesh.connections.discrete;
    if (features.empty())
    {
      return true;
    }
    const bool has_blocks = std::any_of(mesh.groups.begin(),mesh.groups.end(),[](const auto& group)
    {
      return group.kind == CoreResultsVtkGroupKind::Block;
    });
    if (!has_blocks && !mesh.elements.empty())
    {
      CoreResultsVtkGroup group;
      group.name = "Mesh";
      for (const auto& element : mesh.elements)
      {
        if (!features.count(element.id))
        {
          group.ids.push_back(element.id);
        }
      }
      if (!group.ids.empty())
      {
        mesh.groups.push_back(std::move(group));
      }
    }
    // FRD may contain the SPRINGA as a generic two-node line. Reconstruct the
    // named display group only after checking its exact solver ID and endpoints.
    for (const auto& existing : mesh.elements)
    {
      const auto found = features.find(existing.id);
      if (found != features.end() && (found->second.type != "SPRINGA" || existing.frd_type != 11 ||
          existing.connectivity != std::vector<int>{found->second.a,found->second.b}))
      {
        error = "Conflicting FRD topology for discrete element " + std::to_string(existing.id);
        return false;
      }
    }
    mesh.elements.erase(std::remove_if(mesh.elements.begin(),mesh.elements.end(),[&](const auto& existing)
    {
      return features.count(existing.id) != 0;
    }),mesh.elements.end());
    for (const auto& entry : features)
    {
      const auto& d = entry.second;
      for (const int id : {d.a,d.b})
      {
        if (id > 0 && !entity(mesh.nodes,id))
        {
          CoreResultsVtkNode node{id,id == d.a ? d.position_a : d.position_b};
          auto at = std::lower_bound(mesh.nodes.begin(),mesh.nodes.end(),id,[](const auto& n,int key)
          {
            return n.id < key;
          });
          mesh.nodes.insert(at,node);
        }
      }
      CoreResultsVtkGroup group;
      group.name = (d.type == "MASS" ? "Point mass " : "Spring ") + std::to_string(d.id) + " / " + d.name;
      if (d.type == "SPRINGA")
      {
        mesh.elements.push_back({d.element,11,{}, {d.a,d.b}});
        group.ids.push_back(d.element);
      }
      else
      {
        group.kind = CoreResultsVtkGroupKind::Nodeset;
        group.ids.push_back(d.a);
      }
      mesh.groups.push_back(std::move(group));
    }
    return sort_entities(mesh.elements);
  }

  bool select_groups(CoreResultsVtkPlan& plan, std::string& error) {
    for (const auto kind : {CoreResultsVtkGroupKind::Block, CoreResultsVtkGroupKind::Nodeset, CoreResultsVtkGroupKind::Surface}) {
      const auto* selected = &plan.options.sideset_ids;
      if (kind == CoreResultsVtkGroupKind::Block) {
        selected = &plan.options.block_ids;
      } else if (kind == CoreResultsVtkGroupKind::Nodeset) {
        selected = &plan.options.nodeset_ids;
      }
      const auto& requested = *selected;
      for (int id : requested) {
        const auto found = std::find_if(plan.mesh.groups.begin(), plan.mesh.groups.end(), [&](const CoreResultsVtkGroup& group) {
          return group.kind == kind && group.model_id == id;
        });
        if (found == plan.mesh.groups.end()) {
          error = "Requested model group " + std::to_string(id) + " is not available from the selected link source. INP groups are identified by name, not Cubit ID.";
          return false;
        }
      }
      plan.mesh.groups.erase(std::remove_if(plan.mesh.groups.begin(), plan.mesh.groups.end(), [&](const CoreResultsVtkGroup& group) {
        return group.kind == kind && !requested.empty() && std::find(requested.begin(), requested.end(), group.model_id) == requested.end();
      }), plan.mesh.groups.end());
    }
    return true;
  }

  bool frame(CoreResultsVtkPlan& plan, int step, int increment, int total, double time, bool frd, std::size_t& index, std::string& error, const FrdAnalysis* analysis = nullptr) {
    if (!std::isfinite(time)) {
      error = "Non-finite result time.";
      return false;
    }
    std::vector<std::size_t> matches;
    for (std::size_t i = 0; i < plan.increments.size(); ++i) {
      const auto& candidate = plan.increments[i];
      const bool same_time = close(candidate.time, time, plan.options.time_absolute_tolerance, plan.options.time_relative_tolerance);
      const bool identified = step > 0 && increment > 0 && candidate.step > 0 && candidate.increment > 0;
      const bool same_id = identified && candidate.step == step && candidate.increment == increment;
      if (frd) {
        const bool same_analysis = !analysis || (candidate.analysis_type == analysis->type && candidate.mode == analysis->mode &&
          candidate.nodal_diameter == analysis->nodal_diameter && candidate.subcase == analysis->subcase);
        if (candidate.total_increment == total && candidate.step == step && candidate.increment == increment && same_time && same_analysis) {
          matches.push_back(i);
        }
      }
      else if ((same_id && same_time) || (!identified && same_time)) {
        matches.push_back(i);
      }
    }
    if (matches.size() > 1) {
      error = "Ambiguous FRD/DAT increment match; DAT provides insufficient mode/analysis identity. Results were not merged.";
      return false;
    }
    if (matches.empty()) {
      CoreResultsVtkIncrementPlan increment_plan;
      increment_plan.step = step;
      increment_plan.increment = increment;
      increment_plan.total_increment = -1;
      if (frd) {
        increment_plan.total_increment = total;
      }
      increment_plan.time = time;
      if (analysis) {
        increment_plan.analysis_type = analysis->type;
        increment_plan.mode = analysis->mode;
        increment_plan.nodal_diameter = analysis->nodal_diameter;
        increment_plan.subcase = analysis->subcase;
        increment_plan.source_parameters = analysis->parameters;
      }
      index = plan.increments.size();
      plan.increments.push_back(std::move(increment_plan));
    }
    else {
      index = matches.front();
      if (analysis) {
        auto& parameters = plan.increments[index].source_parameters;
        parameters.insert(parameters.end(), analysis->parameters.begin(), analysis->parameters.end());
      }
    }
    return true;
  }

  bool fields(const CoreResultsFrd& frd, const CoreResultsDat& dat, CoreResultsVtkPlan& plan, std::string& error) {
    std::map<std::string, std::size_t> rule_sizes;
    if (plan.options.include_frd) {
      for (std::size_t i = 0; i < frd.result_blocks.size(); ++i) {
        const auto& block = frd.result_blocks[i];
        if (block.size() < 7 || block[4] < 0 || block[5] < 0 || block[6] < 0 || static_cast<std::size_t>(block[4]) >= frd.total_times.size() || static_cast<std::size_t>(block[5]) >= frd.result_block_type.size() || static_cast<std::size_t>(block[6]) >= frd.result_block_components.size() || static_cast<std::size_t>(block[6]) >= frd.result_block_data.size() || static_cast<std::size_t>(block[6]) >= frd.result_block_node_data.size()) {
          error = "Invalid FRD field metadata.";
          return false;
        }
        std::size_t destination = 0;
        const FrdAnalysis* analysis = nullptr;
        if (i < frd.result_analysis.size()) {
          analysis = &frd.result_analysis[i];
        }
        if (!frame(plan, block[1], block[2], block[3], frd.total_times[block[4]], true, destination, error, analysis)) {
          return false;
        }
        CoreResultsVtkFieldPlan field;
        field.source_block = static_cast<int>(i);
        field.source_data = block[6];
        field.field.source = "FRD";
        field.field.name = frd.result_block_type[block[5]];
        // Ordinary FRD fields use nodal identifiers. CELS reported labels can
        // repeat and need not identify mesh nodes; preserve individual rows.
        field.field.association = SimResults::Association::Node;
        field.field.components = frd.result_block_components[block[6]];
        if (field.field.components.empty()) {
          error = "FRD field has no components.";
          return false;
        }
        const bool contact_table = field.field.name == "CELS";
        if (contact_table && field.field.components.size() != 1) {
          error = "FRD CELS contact contributions require one energy component.";
          return false;
        }
        for (const auto& row : frd.result_block_node_data[block[6]]) {
          if (row.size() < 2 || row[0] <= 0 || row[1] < 0 || static_cast<std::size_t>(row[1]) >= frd.result_block_data[block[6]].size()) {
            error = "FRD field contains an invalid source record.";
            return false;
          }
          if (frd.result_block_data[block[6]][row[1]].size() != field.field.components.size()) {
            error = "FRD field component count does not match its source record.";
            return false;
          }
          if (!contact_table && !entity(plan.mesh.nodes, row[0])) {
            error = "Spatial FRD field references missing geometry.";
            return false;
          }
        }
        if (contact_table) {
          field.field.association = SimResults::Association::Table;
          plan.increments[destination].table_fields.push_back(std::move(field));
        }
        else {
          plan.increments[destination].fields.push_back(std::move(field));
        }
      }
    }
    if (!plan.options.include_dat) {
      return true;
    }
    plan.diagnostics.insert(plan.diagnostics.end(), dat.diagnostics.begin(), dat.diagnostics.end());
    for (std::size_t i = 0; i < dat.result_blocks.size(); ++i) {
      const auto& block = dat.result_blocks[i];
      if (block.size() < 7 || block[1] < 0 || block[2] < 0 || block[3] < 0 || block[4] < 0 || static_cast<std::size_t>(block[1]) >= dat.total_times.size() || static_cast<std::size_t>(block[2]) >= dat.result_block_type.size() || static_cast<std::size_t>(block[3]) >= dat.result_block_set.size() || static_cast<std::size_t>(block[4]) >= dat.result_block_components.size() || static_cast<std::size_t>(block[4]) >= dat.result_block_data.size() || static_cast<std::size_t>(block[4]) >= dat.result_block_c1_data.size()) {
        error = "Invalid DAT field metadata.";
        return false;
      }
      const auto& mapping = dat.result_block_c1_data[block[4]];
      if (mapping.empty()) {
        continue;
      }
      int association = 0;
      if (mapping.front().size() >= 3) {
        association = mapping.front()[2];
      }
      if (association < 1 || association > 3) {
        error = "Unknown DAT field association.";
        return false;
      }
      CoreResultsVtkFieldPlan field;
      field.source_block = static_cast<int>(i);
      field.source_data = block[4];
      field.component_offset = 1;
      if (association == 1) {
        field.component_offset = 0;
      } else if (association == 2) {
        field.component_offset = 2;
      }
      field.field.source = "DAT";
      field.field.name = dat.result_block_type[block[2]];
      field.field.set_name = dat.result_block_set[block[3]];
      field.field.association = SimResults::Association::Element;
      if (association == 1) {
        field.field.association = SimResults::Association::Node;
      } else if (association == 2) {
        field.field.association = SimResults::Association::IntegrationPoint;
      }
      const auto& components = dat.result_block_components[block[4]];
      if (components.size() <= field.component_offset) {
        error = "Invalid DAT component layout.";
        return false;
      }
      field.field.components.assign(components.begin() + field.component_offset, components.end());
      // Nodal DAT output is a set-scoped history, not a whole-mesh field.
      // Preserve mapping order and repeated labels without spatial reduction.
      if (association == 1)
      {
        for (const auto& row : mapping)
        {
          if (row.size() < 3 || row[0] <= 0 || row[2] != association || row[1] < 0 ||
              static_cast<std::size_t>(row[1]) >= dat.result_block_data[block[4]].size() ||
              dat.result_block_data[block[4]][row[1]].size() != components.size())
          {
            error = "Invalid DAT nodal history mapping or component layout.";
            return false;
          }
        }
        std::size_t destination = 0;
        if (!frame(plan, block[5], block[6], -1, dat.total_times[block[1]], false, destination, error))
        {
          return false;
        }
        plan.increments[destination].table_fields.push_back(std::move(field));
        continue;
      }
      CoreResultsVtkGroup group;
      group.name = field.field.set_name;
      group.kind = CoreResultsVtkGroupKind::DatElements;
      if (association == 1) {
        group.kind = CoreResultsVtkGroupKind::DatNodes;
      } else if (association == 2) {
        group.kind = CoreResultsVtkGroupKind::DatIntegrationPoints;
      }
      for (const auto& row : mapping) {
        if (row.size() < 3 || row[2] != association || row[1] < 0 || static_cast<std::size_t>(row[1]) >= dat.result_block_data[block[4]].size()) {
          error = "Invalid or mixed DAT field mapping.";
          return false;
        }
        if (dat.result_block_data[block[4]][row[1]].size() != components.size()) {
          error = "DAT values do not match the component definition.";
          return false;
        }
        if (association == 1) {
          if (!entity(plan.mesh.nodes, row[0])) {
            error = "DAT node is absent from the shared job mesh.";
            return false;
          }
        }
        else {
          if (plan.mesh.connections.joint_elements.count(row[0]))
          {
            continue;
          }
          const auto* element = entity(plan.mesh.elements, row[0]);
          if (!element) {
            error = "DAT element is absent from the shared job mesh (expanded shell/beam mapping is not established).";
            return false;
          }
          if (association == 2) {
            auto size = rule_sizes.find(element->calculix_type);
            if (size == rule_sizes.end()) {
              CoreResultsVtkElements::Description description;
              if (element->calculix_type.empty() || !CoreResultsVtkElements::describe(element->calculix_type, description, error) || description.integration_type.empty()) {
                error = "Exact supported element formulation is required to locate DAT integration points for element " + std::to_string(row[0]) + ".";
                return false;
              }
              std::vector<ElementIntegration::IntegrationPoint> rule;
              if (!ElementIntegration::get_rule(description.integration_type, rule)) {
                error = "Unsupported integration rule for DAT element " + std::to_string(row[0]) + ".";
                return false;
              }
              size = rule_sizes.emplace(element->calculix_type, rule.size()).first;
            }
            const auto& values = dat.result_block_data[block[4]][row[1]];
            if (values.size() < 2 || !std::isfinite(values[1]) || values[1] < 1 || values[1] > static_cast<double>(size->second) || std::floor(values[1]) != values[1] || values[0] != static_cast<double>(row[0])) {
              error = "Invalid DAT element/integration-point identifier.";
              return false;
            }
          }
        }
        group.ids.push_back(row[0]);
      }
      if (group.ids.empty())
      {
        continue;
      }
      std::sort(group.ids.begin(), group.ids.end());
      group.ids.erase(std::unique(group.ids.begin(), group.ids.end()), group.ids.end());
      // DAT set names may be uppercased by the solver. Reuse a model set only
      // when its association, folded name AND exact membership agree uniquely.
      // Similar names alone must never merge distinct output subsets.
      auto equal_name = [](const std::string& left, const std::string& right) {
        return left.size() == right.size() && std::equal(left.begin(),left.end(),right.begin(),[](unsigned char a,unsigned char b) {
          return std::toupper(a) == std::toupper(b);
        });
      };
      std::optional<std::size_t> linked;
      bool ambiguous = false;
      for (std::size_t index = 0; index < plan.mesh.groups.size(); ++index) {
        const auto& candidate = plan.mesh.groups[index];
        const bool association_matches = (group.kind == CoreResultsVtkGroupKind::DatNodes && candidate.kind == CoreResultsVtkGroupKind::Nodeset) ||
          (group.kind == CoreResultsVtkGroupKind::DatElements && candidate.kind == CoreResultsVtkGroupKind::Block);
        if (!association_matches || !equal_name(candidate.name,group.name)) {
          continue;
        }
        auto ids = candidate.ids;
        std::sort(ids.begin(),ids.end());
        ids.erase(std::unique(ids.begin(),ids.end()),ids.end());
        if (ids == group.ids) {
          ambiguous = linked.has_value();
          if (ambiguous) {
            break;
          }
          linked = index;
        }
      }
      auto existing = plan.mesh.groups.end();
      if (linked && !ambiguous) {
        existing = plan.mesh.groups.begin()+*linked;
      }
      else {
        existing = std::find_if(plan.mesh.groups.begin(), plan.mesh.groups.end(), [&](const CoreResultsVtkGroup& candidate) {
          return candidate.kind == group.kind && candidate.name == group.name;
        });
      }
      if (existing == plan.mesh.groups.end()) {
        field.target_group = plan.mesh.groups.size();
        plan.mesh.groups.push_back(std::move(group));
      }
      else {
        field.target_group = static_cast<std::size_t>(existing-plan.mesh.groups.begin());
        existing->ids.insert(existing->ids.end(), group.ids.begin(), group.ids.end());
        std::sort(existing->ids.begin(), existing->ids.end());
        existing->ids.erase(std::unique(existing->ids.begin(), existing->ids.end()), existing->ids.end());
      }
      std::size_t destination = 0;
      if (!frame(plan, block[5], block[6], -1, dat.total_times[block[1]], false, destination, error)) {
        return false;
      }
      plan.increments[destination].fields.push_back(std::move(field));
    }
    for (std::size_t i = 0; i < dat.buckle_data.size(); ++i) {
      plan.tables.push_back({CoreResultsVtkTablePlan::Kind::Buckling, i});
    }
    for (std::size_t i = 0; i < dat.result_section_data.size(); ++i) {
      plan.tables.push_back({CoreResultsVtkTablePlan::Kind::Section, i});
    }
    for (std::size_t i = 0; i < dat.result_tables.size(); ++i) {
      const auto& table = dat.result_tables[i];
      if (table.columns.empty() || !std::isfinite(table.time) ||
        (table.scope != DatResultTableScope::Increment && table.scope != DatResultTableScope::StepSummary)) {
        error = "Invalid DAT table schema/time.";
        return false;
      }
      for (const auto& row : table.rows) {
        if (row.size() != table.columns.size()) {
          error = "Incomplete DAT table record.";
          return false;
        }
      }
      // These rows have no spatial association. Do not attempt to match contact
      // labels or face quadrature IDs to ordinary mesh nodes/volume IPs.
      if (table.scope == DatResultTableScope::StepSummary) {
        if (table.increment != -1) {
          error = "DAT step summary has an increment identity.";
          return false;
        }
        plan.tables.push_back({CoreResultsVtkTablePlan::Kind::DatResult, i, -1});
        continue;
      }
      std::size_t destination = 0;
      if (!frame(plan, table.step, table.increment, -1, table.time, false, destination, error)) {
        return false;
      }
      plan.tables.push_back({CoreResultsVtkTablePlan::Kind::DatResult, i, static_cast<int>(destination)});
    }
    return true;
  }
}

bool CoreResultsVtkLink::compatible(const CoreResultsVtkMesh& result, const CoreResultsVtkMesh& candidate, const CoreResultsVtkOptions& options, std::string& reason) {
  if (result.nodes.empty() || (result.elements.empty() && candidate.connections.discrete.empty()) || !std::isfinite(options.coordinate_absolute_tolerance) || !std::isfinite(options.coordinate_relative_tolerance) || options.coordinate_absolute_tolerance < 0.0 || options.coordinate_relative_tolerance < 0.0) {
    reason = "Empty reference mesh or invalid coordinate tolerances.";
    return false;
  }
  std::size_t structural_count = result.elements.size();
  for (const auto& entry : candidate.connections.discrete)
  {
    const auto& feature = entry.second;
    for (const int id : {feature.a,feature.b})
    {
      if (id == 0)
      {
        continue;
      }
      const auto node = entity(candidate.nodes,id);
      if (!node)
      {
        reason = "A physical discrete endpoint is missing from the candidate mesh.";
        return false;
      }
      const auto& archived = id == feature.a ? feature.position_a : feature.position_b;
      for (int axis = 0; axis < 3; ++axis)
      {
        if (!close(node->coordinates[axis],archived[axis],options.coordinate_absolute_tolerance,
            std::max(1e-12, options.coordinate_relative_tolerance)))
        {
          reason = "The physical endpoint has moved for " + feature.name;
          return false;
        }
      }
    }
    if (entity(candidate.elements,feature.element))
    {
      reason = "A discrete element ID is occupied by the candidate structural mesh.";
      return false;
    }
    const auto observed = entity(result.elements,feature.element);
    if (observed)
    {
      if (feature.type != "SPRINGA" || observed->frd_type != 11 ||
          observed->connectivity != std::vector<int>{feature.a,feature.b})
      {
        reason = "FRD discrete topology differs at solver element " + std::to_string(feature.element);
        return false;
      }
      --structural_count;
    }
  }
  for (const auto& item : candidate.connections.joint_elements)
  {
    if (entity(candidate.elements,item.first))
    {
      reason = "Joint helper element ID is occupied by a structural element.";
      return false;
    }
    const auto observed = entity(result.elements,item.first);
    if (observed)
    {
      if (observed->frd_type != 11 || observed->connectivity != std::vector<int>{item.second[0],item.second[1]})
      {
        reason = "Joint helper element differs from archived topology.";
        return false;
      }
      --structural_count;
    }
  }
  if (structural_count != candidate.elements.size()) {
    reason = "Analysis element counts differ.";
    return false;
  }
  for (const auto& node : result.nodes) {
    const auto* other = entity(candidate.nodes, node.id);
    if (!other) {
      reason = "Result node " + std::to_string(node.id) + " is absent.";
      return false;
    }
    for (std::size_t axis = 0; axis < 3; ++axis) {
      if (!frd_coordinate_matches(node.coordinates[axis], other->coordinates[axis], options.coordinate_absolute_tolerance, options.coordinate_relative_tolerance)) {
        reason = "Coordinates differ at result node " + std::to_string(node.id) + ".";
        return false;
      }
    }
  }
  for (const auto& element : result.elements) {
    if (candidate.connections.discrete.count(element.id) || candidate.connections.joint_elements.count(element.id))
    {
      continue;
    }
    const auto* other = entity(candidate.elements, element.id);
    if (!other || other->frd_type != element.frd_type || other->connectivity != element.connectivity) {
      reason = "Topology/connectivity differs at result element " + std::to_string(element.id) + ".";
      return false;
    }
  }
  reason.clear();
  return true;
}

bool CoreResultsVtkLink::match_input_mesh(const CoreResultsFrd& frd, const std::string& input_path,
  std::vector<std::string>& input_files, std::string& error)
{
  CoreResultsVtkMesh reference;
  CoreResultsVtkMesh input;
  std::vector<std::string> files;
  if (!frd_mesh(frd, reference, error) ||
      !CoreResultsInpReader().read(input_path, input, files, error) ||
      !compatible(reference, input, CoreResultsVtkOptions{}, error))
  {
    return false;
  }
  input_files = std::move(files);
  return true;
}

bool CoreResultsVtkLink::prepare(int job_id, const CoreResultsFrd& frd, const CoreResultsDat& dat, const std::string& input_path, const CoreResultsVtkOptions& options, CoreResultsVtkPlan& output, const std::string& source_system, const ConnectionRunMap* connections) const
{
  const double tolerances[] = {options.coordinate_absolute_tolerance, options.coordinate_relative_tolerance, options.time_absolute_tolerance, options.time_relative_tolerance};
  for (double tolerance : tolerances) {
    if (!std::isfinite(tolerance) || tolerance < 0.0) {
      PRINT_ERROR("Invalid VTK linking tolerance.\n");
      return false;
    }
  }
  if (job_id <= 0 || (!options.include_frd && !options.include_dat)) {
    PRINT_ERROR("Invalid VTK conversion-plan request.\n");
    return false;
  }
  CoreResultsVtkPlan result;
  result.job_id = job_id;
  result.options = options;
  result.input_path = input_path;
  std::string reason;
  CoreResultsVtkMesh reference;
  if (frd.nodes.empty() && connections && (!connections->discrete.empty() || !connections->joint_elements.empty()))
  {
    CoreResultsVtkMesh input;
    const bool matched = !input_path.empty() &&
      CoreResultsInpReader().read(input_path,input,result.input_files,reason) &&
      input.connections.json() == connections->json();
    if (matched)
    {
      result.mesh = std::move(input);
    }
    else
    {
      result.input_files.clear();
      result.mesh.connections = *connections;
      result.diagnostics.push_back("No matching input mesh for DAT history. Displaying archived discrete endpoints.");
    }
    result.link_source = SimResults::LinkSource::JobInput;
    result.layout = SimResults::DatasetLayout::Collection;
    if (!select_groups(result,reason) || !add_discrete_groups(result.mesh,reason) ||
        !fields(frd,dat,result,reason) || (result.increments.empty() && result.tables.empty()))
    {
      PRINT_ERROR("Cannot link discrete history to its run input: %s\n",reason.c_str());
      return false;
    }
    output = std::move(result);
    return true;
  }
  if (!frd_mesh(frd, reference, reason)) {
    PRINT_ERROR("Cannot plan VTK conversion: %s\n", reason.c_str());
    return false;
  }
  CoreResultsVtkMesh current;
  bool currentAvailable = capture_model(current, source_system, reason);
  if (currentAvailable && connections)
  {
    current.connections = *connections;
    std::set<int> occupied;
    for (const auto& node : current.nodes)
    {
      occupied.insert(node.id);
    }
    for (const auto& entry : connections->distributed)
    {
      const auto& a = entry.second;
      if (!occupied.insert(a.ref).second)
      {
        currentAvailable = false;
        reason = "Archived distributing reference node ID is now occupied.";
        break;
      }
      current.nodes.push_back({a.ref,a.position});
    }
    for (const auto& item : connections->joint_nodes)
    {
      if (!occupied.insert(item.first).second)
      {
        currentAvailable = false;
        reason = "Archived joint node ID is occupied.";
        break;
      }
      current.nodes.push_back({item.first,item.second});
    }
    for (const auto& entry : connections->points)
    {
      const auto& point = entry.second;
      for (int id : {point.ref, point.rot})
      {
        if (!(id == point.ref ? point.generated_ref : point.generated_rot))
        {
          continue;
        }
        if (!occupied.insert(id).second)
        {
          currentAvailable = false;
          reason = "An archived connection node ID is now in use by the current mesh.";
          break;
        }
        current.nodes.push_back({id, point.position});
      }
      if (!currentAvailable)
      {
        break;
      }
    }
    if (currentAvailable)
    {
      currentAvailable = sort_entities(current.nodes);
    }
  }
  if (currentAvailable && compatible(reference, current, options, reason))
  {
    result.link_source = SimResults::LinkSource::CurrentModel;
    result.mesh = std::move(current);
    // Even with unchanged topology the user may have changed full/reduced
    // integration since the solve. A matching run INP is authoritative for the
    // formulation; the current model still supplies grouping information.
    if (!input_path.empty()) {
      CoreResultsVtkMesh input;
      CoreResultsInpReader reader;
      if (reader.read(input_path, input, result.input_files, reason) && compatible(reference, input, options, reason)) {
        result.mesh.connections = input.connections;
        for (auto& element : result.mesh.elements) {
          element.calculix_type = entity(input.elements, element.id)->calculix_type;
        }
      }
      else {
        result.input_files.clear();
        result.diagnostics.push_back("Run INP did not establish formulations; using the matched current-model definitions: " + reason);
      }
    }
  }
  else {
    result.diagnostics.push_back("Current model not linked: " + reason);
    CoreResultsVtkMesh input;
    CoreResultsInpReader reader;
    if (!input_path.empty() && reader.read(input_path, input, result.input_files, reason) && compatible(reference, input, options, reason)) {
      result.link_source = SimResults::LinkSource::JobInput;
      result.mesh = std::move(input);
    }
    else {
      std::string inputReason = reason;
      if (input_path.empty()) {
        inputReason = "no input path supplied.";
      }
      result.diagnostics.push_back("Job INP not linked: " + inputReason);
      result.input_files.clear();
      result.mesh = std::move(reference);
    }
  }
  // Keep solver-output coordinates authoritative after a tolerant model/INP
  // match; retain extra source nodes only when required by captured groups.
  for (const auto& node : reference.nodes) {
    auto found = std::lower_bound(result.mesh.nodes.begin(), result.mesh.nodes.end(), node.id, [](const CoreResultsVtkNode& value, int id) {
      return value.id < id;
    });
    if (found != result.mesh.nodes.end() && found->id == node.id) {
      found->coordinates = node.coordinates;
    }
  }
  if (connections)
  {
    result.mesh.connections = *connections;
    auto& elements = result.mesh.elements;
    for (const auto& item : connections->joint_elements)
    {
      const auto observed = entity(elements,item.first);
      if (observed && (observed->frd_type != 11 || observed->connectivity != std::vector<int>{item.second[0],item.second[1]}))
      {
        PRINT_ERROR("Archived joint helper conflicts with result element %d.\n",item.first);
        return false;
      }
    }
    elements.erase(std::remove_if(elements.begin(),elements.end(),[&](const auto& e)
    {
      return connections->joint_elements.count(e.id) != 0;
    }),elements.end());
  }
  if (!select_groups(result, reason) || !add_discrete_groups(result.mesh, reason) || !fields(frd, dat, result, reason)) {
    for (const auto& diagnostic : result.diagnostics) {
      PRINT_INFO("VTK linking: %s\n", diagnostic.c_str());
    }
    PRINT_ERROR("Cannot plan VTK conversion: %s\n", reason.c_str());
    return false;
  }
  if (result.increments.empty() && result.tables.empty()) {
    PRINT_ERROR("Cannot plan VTK conversion: no requested result fields.\n");
    return false;
  }
  result.layout = SimResults::DatasetLayout::SingleGrid;
  if (!result.mesh.groups.empty() || !result.mesh.connections.points.empty() || !result.mesh.connections.discrete.empty() || !result.mesh.connections.distributed.empty()) {
    result.layout = SimResults::DatasetLayout::Collection;
  }
  output = std::move(result);
  return true;
}

bool CoreResultsVtkLink::prepare_external(const CoreResultsFrd& frd, const CoreResultsDat& dat,
  const std::string& input_path, const CoreResultsVtkOptions& options,
  CoreResultsVtkPlan& output, std::string& error) const
{
  CoreResultsVtkPlan plan;
  plan.options = options;
  plan.job_id = 0;
  plan.external_source = true;
  plan.input_path = input_path;
  CoreResultsVtkMesh reference;
  if (!frd.nodes.empty() && !frd_mesh(frd,reference,error)) {
    return false;
  }
  if (!input_path.empty()) {
    CoreResultsInpReader reader;
    if (!reader.read(input_path,plan.mesh,plan.input_files,error) ||
        (!reference.nodes.empty() && !compatible(reference,plan.mesh,options,error))) {
      return false;
    }
    plan.link_source = SimResults::LinkSource::JobInput;
    for (const auto& node : reference.nodes) {
      auto found = std::lower_bound(plan.mesh.nodes.begin(),plan.mesh.nodes.end(),node.id,
        [](const CoreResultsVtkNode& value,int id) {
        return value.id < id;
      });
      if (found != plan.mesh.nodes.end() && found->id == node.id) {
        found->coordinates = node.coordinates;
      }
    }
  }
  else {
    plan.mesh = std::move(reference);
  }
  if (!select_groups(plan,error) || !add_discrete_groups(plan.mesh,error) || !fields(frd,dat,plan,error) ||
      (plan.increments.empty() && plan.tables.empty())) {
    if (error.empty()) {
      error = "No convertible result fields or tables were found.";
    }
    return false;
  }
  plan.layout = SimResults::DatasetLayout::SingleGrid;
  if (!plan.mesh.groups.empty() || !plan.mesh.connections.points.empty() || !plan.mesh.connections.discrete.empty() || !plan.mesh.connections.distributed.empty()) {
    plan.layout = SimResults::DatasetLayout::Collection;
  }
  output = std::move(plan);
  error.clear();
  return true;
}
