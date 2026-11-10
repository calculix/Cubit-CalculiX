#include "CoreResultsExternalSource.hpp"
#include "frd/CoreResultsFrd.hpp"
#include "dat/CoreResultsDat.hpp"
#include "vtk/CoreResultsVtkLink.hpp"
#include "vtk/CoreResultsVtkConverter.hpp"
#include "vtk/CoreResultsInpReader.hpp"
#include "vtk/CoreResultsVtkElements.hpp"
#include "CubitMessage.hpp"
#include <map>
#include <algorithm>
#include <set>
#include <cmath>
#include <limits>
#include <tuple>
#include "Framework/Results/ResultFileIdentity.hpp"
#include "loadUserOptions.hpp"
#include "CubitInterface.hpp"
#include <QFileInfo>
#include <QDir>
#include <QScopeGuard>
#include <QScopedValueRollback>
#include <atomic>
#include <exception>
namespace
{
  bool busy = false;
  // External-only fallback: preserve an entire DAT block as a table when its
  // spatial association is unavailable. Never invent node coordinates/IP rules.
  // Reader payload arrays stay intact; only detached block descriptors move.
  bool unlinked_tables(const CoreResultsFrd& frd, const CoreResultsVtkMesh& input,
    CoreResultsDat& dat, std::vector<std::string>& diagnostics, std::string& error)
  {
    std::set<int> nodes, elements;
    std::map<int,std::string> formulations;
    for (const auto& node : frd.nodes)
    {
      if (!node.empty())
      {
        nodes.insert(node[0]);
      }
    }
    for (const auto& element : frd.elements)
    {
      if (!element.empty())
      {
        elements.insert(element[0]);
      }
    }
    for (const auto& node : input.nodes)
    {
      nodes.insert(node.id);
    }
    for (const auto& element : input.elements)
    {
      elements.insert(element.id);
      formulations[element.id] = element.calculix_type;
    }
    std::map<std::string,bool> knownRules;
    std::vector<std::vector<int>> spatial;
    std::map<std::tuple<int,int,int,int,int,double>,std::size_t> occurrences;
    for (std::size_t index = 0; index < dat.result_blocks.size(); ++index)
    {
      const auto& block = dat.result_blocks[index];
      if (block.size() < 7 || block[1] < 0 || block[2] < 0 || block[3] < 0 || block[4] < 0 ||
          static_cast<std::size_t>(block[1]) >= dat.total_times.size() ||
          static_cast<std::size_t>(block[2]) >= dat.result_block_type.size() ||
          static_cast<std::size_t>(block[3]) >= dat.result_block_set.size() ||
          static_cast<std::size_t>(block[4]) >= dat.result_block_data.size() ||
          static_cast<std::size_t>(block[4]) >= dat.result_block_components.size() ||
          static_cast<std::size_t>(block[4]) >= dat.result_block_c1_data.size())
      {
        error = "Invalid detached DAT record metadata.";
        return false;
      }
      if (!std::isfinite(dat.total_times[block[1]]))
      {
        error = "Invalid detached DAT record time.";
        return false;
      }
      const auto& mapping = dat.result_block_c1_data[block[4]];
      if (mapping.empty())
      {
        spatial.push_back(block);
        continue;
      }
      const int association = mapping.front().size() >= 3 ? mapping.front()[2] : 0;
      if (association < 1 || association > 3)
      {
        error = "Invalid detached DAT record association.";
        return false;
      }
      const auto& rows = dat.result_block_data[block[4]];
      const auto& columns = dat.result_block_components[block[4]];
      const std::size_t offset = association == 1 ? 0 : association == 2 ? 2 : 1;
      if (columns.size() <= offset)
      {
        error = "Invalid detached DAT component layout.";
        return false;
      }
      std::string reason;
      for (const auto& row : mapping)
      {
        if (row.size() < 3 || row[0] <= 0 || row[2] != association || row[1] < 0 ||
            static_cast<std::size_t>(row[1]) >= rows.size() || rows[row[1]].size() != columns.size())
        {
          error = "Invalid detached DAT record mapping or component count.";
          return false;
        }
        const auto& values = rows[row[1]];
        if (association != 1 && (!std::isfinite(values[0]) || values[0] != row[0]))
        {
          error = "DAT element ID does not agree with its record mapping.";
          return false;
        }
        if (association == 2 && (!std::isfinite(values[1]) || values[1] < 1 ||
            values[1] > std::numeric_limits<int>::max() || std::floor(values[1]) != values[1]))
        {
          error = "Invalid DAT integration-point number.";
          return false;
        }
        if (association == 1 && nodes.count(row[0]) == 0)
        {
          reason = "node "+std::to_string(row[0])+" is absent from the supplied mesh";
        }
        else if (association != 1 && elements.count(row[0]) == 0)
        {
          reason = "element "+std::to_string(row[0])+" is absent from the supplied mesh";
        }
        else if (association == 2)
        {
          const auto found = formulations.find(row[0]);
          const std::string type = found == formulations.end() ? std::string{} : found->second;
          auto rule = knownRules.find(type);
          if (rule == knownRules.end())
          {
            CoreResultsVtkElements::Description description;
            std::vector<ElementIntegration::IntegrationPoint> points;
            std::string issue;
            const bool known = !type.empty() && CoreResultsVtkElements::describe(type,description,issue) &&
              !description.integration_type.empty() && ElementIntegration::get_rule(description.integration_type,points);
            rule = knownRules.emplace(type,known).first;
          }
          if (!rule->second)
          {
            reason = "an exact supported integration-point formulation is unavailable";
          }
        }
      }
      // Count linked contributions too, so fallback keys do not shift when a
      // sibling contribution becomes spatially available in another frame.
      const auto occurrence = occurrences[std::make_tuple(block[2],block[3],association,block[5],block[6],dat.total_times[block[1]])]++;
      if (reason.empty())
      {
        spatial.push_back(block);
        continue;
      }
      DatResultTable table;
      // A field/set has the same logical key in every frame. Distinct repeated
      // contributions in one frame receive a deterministic occurrence suffix.
      table.key = "unlinked:"+std::to_string(block[2])+":"+std::to_string(block[3])+":"+
        std::to_string(association)+":"+std::to_string(occurrence);
      table.name = dat.result_block_type[block[2]];
      table.set_name = dat.result_block_set[block[3]];
      table.step = block[5]; table.increment = block[6]; table.time = dat.total_times[block[1]];
      table.columns = columns;
      if (association == 1)
      {
        table.columns.insert(table.columns.begin(),"NodeId");
      }
      for (const auto& record : mapping)
      {
        auto values = rows[record[1]];
        if (association == 1)
        {
          values.insert(values.begin(),static_cast<double>(record[0]));
        }
        table.rows.push_back(std::move(values));
      }
      const auto diagnostic = "DAT "+table.name+" for set "+table.set_name+" remains a table because "+reason+
        ". Supply a matching INP to establish the missing spatial association.";
      if (std::find(diagnostics.begin(),diagnostics.end(),diagnostic) == diagnostics.end())
      {
        diagnostics.push_back(diagnostic);
      }
      dat.result_tables.push_back(std::move(table));
    }
    dat.result_blocks.swap(spatial);
    return true;
  }
}
bool CoreResultsExternalSource::open(const SimResults::FileSourceReference& request,
  SimResults::PublishedResultSource& output, std::string& error, bool restore) const
{
  error.clear();
  if (busy || request.provider_id != "ccx" || request.paths.empty() || request.paths.size() > 3)
  {
    error = "Invalid external CalculiX request or a conversion is already active.";
    return false;
  }
  QScopedValueRollback<bool> guard(busy,true);
  try
  {
    std::string frdPath, datPath, inputPath;
    std::vector<std::string> paths;
    for (const auto& supplied : request.paths)
    {
      const QFileInfo file(QString::fromUtf8(supplied.c_str()));
      if ((restore && !QDir::isAbsolutePath(file.filePath())) || !file.isFile() || !file.isReadable())
      {
        error = "An external CalculiX file is missing or unreadable: "+supplied;
        return false;
      }
      const auto suffix = file.suffix().toLower();
      std::string* destination = suffix == "frd" ? &frdPath : suffix == "dat" ? &datPath : suffix == "inp" ? &inputPath : nullptr;
      if (!destination || !destination->empty())
      {
        error = "Supply at most one FRD, DAT and INP file.";
        return false;
      }
      *destination = file.canonicalFilePath().toUtf8().toStdString();
      paths.push_back(*destination);
    }
    if (frdPath.empty() && datPath.empty())
    {
      error = "An FRD or DAT result file is required.";
      return false;
    }
    std::vector<std::string> dependencies = paths;
    CoreResultsVtkMesh inputMesh;
    if (!inputPath.empty())
    {
      std::vector<std::string> includes;
      if (!CoreResultsInpReader().read(inputPath,inputMesh,includes,error))
      {
        return false;
      }
      dependencies.insert(dependencies.end(),includes.begin(),includes.end());
    }
    std::string identity;
    if (!SimResults::result_file_identity(dependencies,identity,error) ||
        (restore && (request.content_identity.empty() || request.content_identity != identity)))
    {
      if (error.empty())
      {
        error = "External CalculiX dependencies differ from the saved result.";
      }
      return false;
    }
    CoreResultsFrd frd;
    CoreResultsDat dat;
    if ((!frdPath.empty() && !frd.read_file(frdPath)) || (!datPath.empty() && !dat.read_file(datPath)))
    {
      error = "Cannot read external CalculiX results.";
      return false;
    }
    std::vector<std::string> fallbackDiagnostics;
    if (!unlinked_tables(frd,inputMesh,dat,fallbackDiagnostics,error))
    {
      return false;
    }
    CoreResultsVtkOptions options;
    if (ccx_uo.mConverterThreads < 1)
    {
      error = "Result conversion requires at least one converter thread.";
      return false;
    }
    options.workers = static_cast<std::size_t>(ccx_uo.mConverterThreads);
    options.include_frd = !frdPath.empty(); options.include_dat = !datPath.empty();
    CoreResultsVtkPlan plan;
    if (!CoreResultsVtkLink().prepare_external(frd,dat,inputPath,options,plan,error))
    {
      return false;
    }
    plan.diagnostics.insert(plan.diagnostics.end(),fallbackDiagnostics.begin(),fallbackDiagnostics.end());
    plan.run_identity = identity;
    plan.result_identity = identity;
    auto* progress = CubitInterface::app_util().get()->progress_tool();
    if (!progress)
    {
      error = "Result conversion progress service is unavailable.";
      return false;
    }
    progress->start(0,100,"Importing CalculiX Results");
    const auto endProgress = qScopeGuard([&]()
    {
      progress->end();
    });
    std::atomic<bool> cancelled{false};
    const auto poll = [&](double fraction)
    {
      progress->percent(fraction);
      progress->check_interrupt();
      return !CubitInterface::app_util().get()->interrupt();
    };
    std::shared_ptr<const SimResults::VtkResultsData> data;
    if (!CoreResultsVtkConverter().build(frd,dat,plan,cancelled,poll,data,error))
    {
      if (error.empty())
      {
        error = "External result conversion was cancelled.";
      }
      return false;
    }
    std::string after;
    if (!SimResults::result_file_identity(dependencies,after,error) || after != identity)
    {
      error = "External result dependencies changed during import.";
      return false;
    }
    SimResults::PublishedResultSource candidate;
    candidate.reference = SimResults::FileSourceReference{"ccx",paths,identity};
    candidate.suggested_name = QFileInfo(QString::fromUtf8(paths.front().c_str())).completeBaseName().toUtf8().toStdString();
    candidate.data = std::move(data);
    output = std::move(candidate);
    for (const auto& diagnostic : fallbackDiagnostics)
    {
      PRINT_INFO("CalculiX import: %s\n",diagnostic.c_str());
    }
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("External CalculiX import failed: ")+exception.what();
    return false;
  }
}
