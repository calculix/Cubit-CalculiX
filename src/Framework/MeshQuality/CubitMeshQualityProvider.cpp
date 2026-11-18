#include <utility>
#include "CubitMeshQualityProvider.hpp"
#include "cubit/CubitInterface.hpp"
#include <algorithm>
#include <cmath>
#include <exception>
#include <limits>
#include <tuple>
#include <unordered_map>
CubitMeshQualityProvider::CubitMeshQualityProvider() = default;
bool CubitMeshQualityProvider::collect(const std::map<int, std::string>& blocks,
  const std::string& coordinateUnit, const MeshQualitySettings& settings, ReportProgress* progress,
  std::shared_ptr<const MeshQualitySnapshot>& output, std::string& error, const std::string& inputFingerprint) const
{
  try
  {
    auto candidate = std::make_shared<MeshQualitySnapshot>();
    candidate->coordinate_unit = coordinateUnit;
    candidate->input_fingerprint = inputFingerprint;
    candidate->evaluator_version = CubitInterface::get_version() + " / native quality / report catalogue 1";
    std::unordered_map<std::int64_t, std::size_t> nodeIndex;
    std::vector<std::vector<int>> nativeGroups;
    std::size_t completed = 0, elementCount = 0;
    std::uint64_t payloadBytes = 0;
    constexpr std::uint64_t payloadBudget = 240ULL * 1024 * 1024;
    for (const auto& block : blocks)
    {
      if (!ReportProgress::advance(progress, ReportProgressStage::Model,
            "Reading quality mesh / block " + std::to_string(block.first), completed, blocks.size(), error))
      {
        return false;
      }
      const auto familyIds = std::vector<std::pair<std::string, std::vector<int>>>{
        {"hex", CubitInterface::get_block_hexes(block.first)},
        {"tet", CubitInterface::get_block_tets(block.first)},
        {"quad", CubitInterface::get_block_faces(block.first)},
        {"tri", CubitInterface::get_block_tris(block.first)}};
      for (const auto& family : familyIds)
      {
        std::string entity = family.first;
        if (entity == "quad")
        {
          entity = "face";
        }
        std::map<std::size_t, std::pair<MeshQualityBlock, std::vector<int>>> groups;
        for (std::size_t index = 0; index < family.second.size(); ++index)
        {
          if (index % 2048 == 0 && !ReportProgress::advance(progress, ReportProgressStage::Model,
                "Reading block " + std::to_string(block.first) + " / " + family.first + " connectivity",
                index, family.second.size(), error))
          {
            return false;
          }
          if (++elementCount > 2000000)
          {
            error = "Quality capture exceeds the two-million-element limit.";
            return false;
          }
          const int id = family.second[index];
          const auto connectivity = CubitInterface::get_connectivity(entity, id);
          if (connectivity.empty() || connectivity.size() > 64)
          {
            error = "Native quality connectivity is unavailable or unsupported.";
            return false;
          }
          auto& group = groups[connectivity.size()];
          auto& part = group.first;
          part.block_id = block.first;
          part.block_name = block.second;
          part.family = family.first;
          part.nodes_per_element = connectivity.size();
          const int global = CubitInterface::get_global_element_id(entity, id);
          if (global <= 0)
          {
            error = "Quality element has no verified global ID.";
            return false;
          }
          payloadBytes += 8 + 8 * connectivity.size();
          part.element_ids.push_back(global);
          group.second.push_back(id);
          for (int node : connectivity)
          {
            part.connectivity.push_back(node);
            if (nodeIndex.count(node) == 0)
            {
              const auto xyz = CubitInterface::get_nodal_coordinates(node);
              if (xyz.size() != 3 || candidate->node_ids.size() >= 8000000)
              {
                error = "Quality mesh coordinates are unavailable or oversized.";
                return false;
              }
              payloadBytes += 32;
              nodeIndex.emplace(node, candidate->node_ids.size());
              candidate->node_ids.push_back(node);
              candidate->coordinates.push_back({xyz[0], xyz[1], xyz[2]});
            }
          }
          if (payloadBytes > payloadBudget)
          {
            error = "Quality mesh exceeds the archive geometry budget.";
            return false;
          }
        }
        for (auto& group : groups)
        {
          candidate->blocks.push_back(std::move(group.second.first));
          nativeGroups.push_back(std::move(group.second.second));
        }
      }
      if (!CubitInterface::get_block_wedges(block.first).empty() ||
          !CubitInterface::get_block_pyramids(block.first).empty() ||
          !CubitInterface::get_block_edges(block.first).empty())
      {
        candidate->diagnostics.push_back("Block " + std::to_string(block.first) +
          ": edge, wedge or pyramid elements have no configured quality family; they are not included.");
      }
      ++completed;
    }
    // Geometry is captured before allocating metric arrays. Prioritize default
    // metrics across every block before optional metrics consume the budget.
    for (const auto& metric : settings.metrics)
    {
      for (std::size_t group = 0; group < candidate->blocks.size(); ++group)
      {
        auto& part = candidate->blocks[group];
        const auto& ids = nativeGroups[group];
        if (metric.family != part.family)
        {
          continue;
        }
        if (ids.size() * 8ULL > payloadBudget - payloadBytes)
        {
          part.unavailable[metric.id] = "Metric exceeds the quality snapshot sample budget.";
          continue;
        }
        std::vector<double> values;
        values.reserve(ids.size());
        bool available = true;
        // Group-dependent metrics are queried once for this complete group;
        // never alter Cubit's reference settings between sub-queries.
        std::size_t chunk = 2048;
        if (metric.requires_group_reference)
        {
          chunk = std::max(std::size_t(1), ids.size());
        }
        for (std::size_t begin = 0; begin < ids.size(); begin += chunk)
        {
          if (!ReportProgress::advance(progress, ReportProgressStage::Model,
                "Evaluating block " + std::to_string(part.block_id) + " / " + part.family + " / " + metric.label,
                begin, ids.size(), error))
          {
            return false;
          }
          const auto end = std::min(ids.size(), begin + chunk);
          const std::vector<int> batch(ids.begin() + begin, ids.begin() + end);
          const int errorsBefore = CubitInterface::get_error_count();
          const auto measured = CubitInterface::get_quality_values(metric.family, batch, metric.native_name);
          if (CubitInterface::get_error_count() != errorsBefore || measured.size() != batch.size())
          {
            part.unavailable[metric.id] = "Cubit could not evaluate this metric for the element family/order.";
            available = false;
            break;
          }
          values.insert(values.end(), measured.begin(), measured.end());
        }
        if (available)
        {
          payloadBytes += ids.size() * 8ULL;
          part.values.emplace(metric.id, std::move(values));
        }
      }
    }
    candidate->mesh_fingerprint = candidate->fingerprint();
    if (!candidate->validate(error))
    {
      return false;
    }
    output = std::move(candidate);
    error.clear();
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot collect Cubit mesh quality: ") + exception.what();
    return false;
  }
}
