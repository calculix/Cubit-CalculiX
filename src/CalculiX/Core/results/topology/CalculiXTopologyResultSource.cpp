#include "CalculiXTopologyResultSource.hpp"
#include "TopologyElementPartitioner.hpp"
#include "Core/jobs/CalculiXJobsInterface.hpp"
#include "Core/results/CalculiXResultsInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "Core/topology/CoreTopologyStructs.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include <QByteArray>
#include <QCryptographicHash>
#include <QDataStream>
#include <QIODevice>
#include <QString>
#include <algorithm>
#include <cmath>
#include <exception>
#include <utility>

namespace
{
  bool completed(const std::vector<std::string>& row)
  {
    return row.size() >= 4 && row[2] == "2" && !row[3].empty();
  }

  void append_rows(QDataStream& stream, const std::vector<std::vector<double>>& rows)
  {
    stream << static_cast<quint64>(rows.size());
    for (const auto& row : rows)
    {
      stream << static_cast<quint64>(row.size());
      for (const auto value : row)
      {
        stream << value;
      }
    }
  }
}

void CalculiXTopologyResultSource::append_catalogue(std::vector<SimResults::ResultJobEntry>& entries)
{
  CalculiXJobsInterface jobs;
  CalculiXTopologyInterface topology;
  const auto cases = topology.get_topology_tree_data();
  if (cases.empty())
  {
    return;
  }
  const bool busy = CalculiXResultsInterface().is_vtk_build_active();
  entries.erase(std::remove_if(entries.begin(),entries.end(),[&](const auto& entry)
  {
    int owner = -1, iteration = -1;
    return jobs.get_job_topology_context(entry.reference.job_id,owner,iteration) && owner > 0;
  }),entries.end());
  for (const auto& row : cases)
  {
    const int id = std::stoi(row.at(0));
    const auto iterations = topology.get_topology_iteration_data(id);
    const auto count = std::count_if(iterations.begin(),iterations.end(),completed);
    SimResults::ResultJobEntry entry;
    entry.reference.application_id = "calculix";
    entry.reference.job_id = id;
    entry.reference.topology_threshold = SimResults::default_topology_threshold;
    entry.command_prefix = "ccx";
    entry.application_name = "CalculiX";
    entry.job_name = row.at(1);
    entry.available = !busy && count > 0;
    if (busy)
    {
      entry.unavailable_reason = "Result conversion is running.";
    }
    else if (!count)
    {
      entry.unavailable_reason = "No completed physical topology iteration.";
    }
    entries.push_back(std::move(entry));
  }
}

bool CalculiXTopologyResultSource::load(ResultSourceProviderInterface& provider,
  const SimResults::JobSourceReference& request, bool restoring,
  SimResults::PublishedResultSource& output, std::string& error, std::optional<double> newThreshold)
{
  error.clear();
  if (request.application_id != "calculix" || !request.project_id.empty() || request.job_id <= 0 ||
      !request.topology_threshold || !std::isfinite(*request.topology_threshold) ||
      *request.topology_threshold < 0 || *request.topology_threshold > 1 ||
      (restoring && (request.result_identity.empty() || request.topology_last_iteration <= 0)) ||
      (newThreshold && (!restoring || !std::isfinite(*newThreshold) || *newThreshold < 0 || *newThreshold > 1)))
  {
    error = "Invalid topology result reference or threshold.";
    return false;
  }
  try
  {
    CalculiXTopologyInterface topology;
    CalculiXResultsInterface results;
    const auto cases = topology.get_topology_tree_data();
    const auto found = std::find_if(cases.begin(),cases.end(),[&](const auto& row)
    {
      return row.at(0) == std::to_string(request.job_id);
    });
    if (found == cases.end() || results.is_vtk_build_active())
    {
      error = "The topology case is missing or result conversion is running.";
      return false;
    }
    auto series = std::make_shared<SimResults::VtkResultsData>();
    series->job_id = request.job_id;
    series->source_workspace = "calculix";
    series->layout = SimResults::DatasetLayout::Collection;
    auto reference = request;
    reference.topology_last_iteration = 0;
    const double threshold = newThreshold.value_or(*request.topology_threshold);
    reference.topology_threshold = threshold;
    QCryptographicHash updatedHash(QCryptographicHash::Sha256);
    if (newThreshold)
    {
      updatedHash.addData(QByteArray("cubitsim-topology-series-v1"));
    }
    QCryptographicHash hash(QCryptographicHash::Sha256);
    hash.addData(QByteArray("cubitsim-topology-series-v1"));
    const bool refreshing = !restoring && request.run_identity.empty() && !request.result_identity.empty();
    bool verifiedPrevious = !refreshing;
    TopologyElementPartitioner::MeshIdentity baseline;
    const auto iterations = topology.get_topology_iteration_data(request.job_id);
    for (const auto& row : iterations)
    {
      if (!completed(row))
      {
        continue;
      }
      const int iteration = std::stoi(row[0]);
      if (restoring && iteration > request.topology_last_iteration)
      {
        break;
      }
      const int job = std::stoi(row[1]);
      SimResults::JobSourceReference childReference;
      childReference.application_id = "calculix";
      childReference.job_id = job;
      SimResults::PublishedResultSource child;
      bool loaded = false;
      if (restoring)
      {
        const auto cached = results.get_vtk_result(job);
        if (cached)
        {
          childReference.result_identity = cached->result_identity;
        }
        else
        {
          childReference.result_identity = results.retained_identity(job,error);
        }
        loaded = !childReference.result_identity.empty() && provider.restore(childReference,child,error);
      }
      else
      {
        loaded = provider.open(childReference,child,error);
      }
      if (!loaded || !child.data || child.data->result_identity.empty() || child.data->increments.empty() ||
          !child.data->increments.back().dataset)
      {
        error = "Topology iteration " + std::to_string(iteration) + " has no usable final mesh: " + error;
        return false;
      }
      const auto elements = topology.get_topology_iteration_element_data(request.job_id,iteration);
      TopologyMethod method;
      if (elements.empty() || !topology.get_topology_iteration_method(request.job_id,iteration,method))
      {
        error = "A completed topology iteration has no valid stored design.";
        return false;
      }
      const bool levelSet = method == TopologyMethod::LevelSet;
      std::vector<std::vector<double>> nodes;
      if (levelSet)
      {
        nodes = topology.get_topology_iteration_level_set_field_data(request.job_id,iteration);
        if (nodes.empty())
        {
          error = "The topology iteration has no stored level-set coefficients.";
          return false;
        }
      }
      SimResults::Increment frame;
      std::vector<SimResults::Dataset> datasets;
      TopologyElementPartitioner::MeshIdentity identity;
      if (!TopologyElementPartitioner::split(*child.data,child.data->increments.back(),elements,nodes,levelSet,
          threshold,frame,datasets,identity,error))
      {
        error = "Topology iteration " + std::to_string(iteration) + ": " + error;
        return false;
      }
      if (series->increments.empty())
      {
        baseline = std::move(identity);
        series->source_units = child.data->source_units;
        series->link_source = child.data->link_source;
        series->datasets = std::move(datasets);
      }
      else
      {
        if (identity != baseline || frame.coordinate_unit != series->increments.front().coordinate_unit ||
            datasets.size() != series->datasets.size())
        {
          error = "Topology jobs have incompatible element connectivity, mesh catalogues or coordinate units.";
          return false;
        }
        for (std::size_t i = 0; i < datasets.size(); ++i)
        {
          auto& existing = series->datasets[i];
          if (existing.name != datasets[i].name || existing.kind != datasets[i].kind || existing.model_id != datasets[i].model_id)
          {
            error = "Topology jobs have different dataset identities.";
            return false;
          }
          existing.point_count = std::max(existing.point_count,datasets[i].point_count);
          existing.cell_count = std::max(existing.cell_count,datasets[i].cell_count);
        }
      }
      frame.topology_iteration = iteration;
      frame.topology_job = job;
      // Frame binding must distinguish final solver frames with identical step/time.
      frame.source_parameters.push_back("Topology solver step " + std::to_string(frame.step) +
        " increment " + std::to_string(frame.increment) + " total " + std::to_string(frame.total_increment) +
        " parameter " + QString::number(frame.time,'g',17).toStdString());
      frame.source_parameters.push_back("Topology cutoff " + QString::number(threshold,'g',17).toStdString());
      frame.step = 1;
      frame.increment = iteration;
      frame.total_increment = iteration;
      frame.time = iteration;
      frame.independent_variable = SimUnits::QuantityUnit("", "1", SimUnits::Dimensionless);
      series->increments.push_back(std::move(frame));
      reference.topology_last_iteration = iteration;
      const auto addRecord = [&](QCryptographicHash& target, double cutoff)
      {
        QByteArray record;
        QDataStream stream(&record,QIODevice::WriteOnly);
        stream.setVersion(QDataStream::Qt_6_0);
        stream << request.job_id << iteration << job << cutoff << levelSet
          << QByteArray::fromStdString(child.data->result_identity);
        append_rows(stream,elements);
        append_rows(stream,nodes);
        target.addData(record);
      };
      addRecord(hash,*request.topology_threshold);
      if (newThreshold)
      {
        addRecord(updatedHash,threshold);
      }
      if (refreshing && iteration == request.topology_last_iteration)
      {
        verifiedPrevious = hash.result().toHex().toStdString() == request.result_identity;
        if (!verifiedPrevious)
        {
          error = "The topology case changed since this source was opened. Open a new source for the current run.";
          return false;
        }
      }
    }
    if (series->increments.empty())
    {
      error = "No completed physical topology iteration is available.";
      return false;
    }
    if (!verifiedPrevious)
    {
      error = "The previous topology iteration boundary is no longer available for refresh.";
      return false;
    }
    series->result_identity = hash.result().toHex().toStdString();
    series->run_identity = series->result_identity;
    if ((restoring && series->result_identity != request.result_identity) ||
        (!request.run_identity.empty() && !restoring && series->run_identity != request.run_identity))
    {
      error = "The topology iterations differ from the requested result snapshot.";
      return false;
    }
    if (newThreshold)
    {
      series->result_identity = updatedHash.result().toHex().toStdString();
      series->run_identity = series->result_identity;
    }
    reference.result_identity = series->result_identity;
    reference.run_identity = series->run_identity;
    output = {reference,"Topology " + std::to_string(request.job_id) + " / " + found->at(1),std::move(series)};
    return true;
  }
  catch (const std::exception& exception)
  {
    error = std::string("Cannot assemble topology results: ") + exception.what();
    return false;
  }
}
