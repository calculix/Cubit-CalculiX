#include "VtkResultsExporter.hpp"
#include <vtkAbstractArray.h>
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataAssembly.h>
#include <vtkDataObject.h>
#include <vtkDataSetAttributes.h>
#include <vtkFieldData.h>
#include <vtkInformation.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedCharArray.h>
#include <vtkXMLPartitionedDataSetCollectionWriter.h>
#include <vtkXMLTableWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLWriterBase.h>
#include <chrono>
#include <algorithm>
#include <atomic>
#include <condition_variable>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <exception>
#include <mutex>
#include <thread>
#include <vector>

namespace
{
  namespace fs = std::filesystem;

  std::string xml(const std::string& value)
  {
    std::string escaped;
    for (const char character : value)
    {
      switch (character)
      {
        case '&': escaped += "&amp;"; break;
        case '<': escaped += "&lt;"; break;
        case '>': escaped += "&gt;"; break;
        case '"': escaped += "&quot;"; break;
        case '\'': escaped += "&apos;"; break;
        default: escaped += character; break;
      }
    }
    return escaped;
  }

  struct Staging
  {
    fs::path path;
    ~Staging()
    {
      if (!path.empty())
      {
        std::error_code ignored;
        fs::remove_all(path, ignored);
      }
    }
  };

  // vtkXMLWriter calls GetRange(), including the overload that writes an
  // array's internal Range buffer. Warming the source caches is insufficient:
  // every active write needs distinct array objects, even for shared geometry.
  vtkSmartPointer<vtkAbstractArray> export_array(vtkAbstractArray* source)
  {
    vtkSmartPointer<vtkAbstractArray> copy;
    copy.TakeReference(source->NewInstance());
    if (auto* numeric = vtkDataArray::SafeDownCast(source))
    {
      // Standard AOS arrays share their reference-counted value buffer here.
      // VTK's base implementation deep-copies unsupported numeric layouts.
      vtkDataArray::SafeDownCast(copy)->ShallowCopy(numeric);
    }
    else
    {
      copy->DeepCopy(source);
    }
    copy->SetName(source->GetName());
    copy->CopyComponentNames(source);
    if (source->HasInformation())
    {
      copy->GetInformation()->Copy(source->GetInformation(), 1);
    }
    return copy;
  }

  void export_fields(vtkFieldData* source, vtkFieldData* copy)
  {
    copy->Initialize();
    if (!source)
    {
      return;
    }
    for (int i = 0; i < source->GetNumberOfArrays(); ++i)
    {
      copy->AddArray(export_array(source->GetAbstractArray(i)));
    }
    auto* attributes = vtkDataSetAttributes::SafeDownCast(source);
    auto* copied_attributes = vtkDataSetAttributes::SafeDownCast(copy);
    if (attributes && copied_attributes)
    {
      int indices[vtkDataSetAttributes::NUM_ATTRIBUTES];
      attributes->GetAttributeIndices(indices);
      for (int i = 0; i < vtkDataSetAttributes::NUM_ATTRIBUTES; ++i)
      {
        if (indices[i] >= 0)
        {
          copied_attributes->SetActiveAttribute(indices[i], i);
        }
      }
    }
  }

  vtkSmartPointer<vtkCellArray> export_cells(vtkCellArray* source)
  {
    if (!source)
    {
      return nullptr;
    }
    auto copy = vtkSmartPointer<vtkCellArray>::New();
    auto offsets = export_array(source->GetOffsetsArray());
    auto connectivity = export_array(source->GetConnectivityArray());
    if (!copy->SetData(vtkDataArray::SafeDownCast(offsets), vtkDataArray::SafeDownCast(connectivity)))
    {
      copy->DeepCopy(source);
    }
    return copy;
  }

  // Invoked under the preparation mutex. Published source objects are never
  // passed to writers. Copies own their metadata, points and connectivity
  // wrappers; immutable numeric buffers remain owned for the whole export.
  vtkSmartPointer<vtkDataObject> export_view(vtkDataObject* source, std::string& error)
  {
    if (!source)
    {
      error = "Missing VTK export object.";
      return nullptr;
    }
    vtkSmartPointer<vtkDataObject> copy;
    if (auto* grid = vtkUnstructuredGrid::SafeDownCast(source))
    {
      auto view = vtkSmartPointer<vtkUnstructuredGrid>::New();
      if (grid->GetPoints())
      {
        auto points = vtkSmartPointer<vtkPoints>::New();
        points->SetData(vtkDataArray::SafeDownCast(export_array(grid->GetPoints()->GetData())));
        view->SetPoints(points);
      }
      if (grid->GetCells())
      {
        auto types = export_array(grid->GetCellTypesArray());
        view->SetPolyhedralCells(vtkUnsignedCharArray::SafeDownCast(types),
          export_cells(grid->GetCells()), export_cells(grid->GetPolyhedronFaceLocations()),
          export_cells(grid->GetPolyhedronFaces()));
      }
      export_fields(grid->GetPointData(), view->GetPointData());
      export_fields(grid->GetCellData(), view->GetCellData());
      copy = view;
    }
    else if (auto* collection = vtkPartitionedDataSetCollection::SafeDownCast(source))
    {
      auto view = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
      view->SetNumberOfPartitionedDataSets(collection->GetNumberOfPartitionedDataSets());
      for (unsigned int i = 0; i < collection->GetNumberOfPartitionedDataSets(); ++i)
      {
        auto partition = export_view(collection->GetPartitionedDataSet(i), error);
        if (!partition)
        {
          return nullptr;
        }
        view->SetPartitionedDataSet(i, vtkPartitionedDataSet::SafeDownCast(partition));
        if (collection->HasMetaData(i))
        {
          view->GetMetaData(i)->Copy(collection->GetMetaData(i), 1);
        }
      }
      if (collection->GetDataAssembly())
      {
        auto assembly = vtkSmartPointer<vtkDataAssembly>::New();
        assembly->DeepCopy(collection->GetDataAssembly());
        view->SetDataAssembly(assembly);
      }
      copy = view;
    }
    else if (auto* partitions = vtkPartitionedDataSet::SafeDownCast(source))
    {
      auto view = vtkSmartPointer<vtkPartitionedDataSet>::New();
      view->SetNumberOfPartitions(partitions->GetNumberOfPartitions());
      for (unsigned int i = 0; i < partitions->GetNumberOfPartitions(); ++i)
      {
        if (auto* partition = partitions->GetPartitionAsDataObject(i))
        {
          auto child = export_view(partition, error);
          if (!child)
          {
            return nullptr;
          }
          view->SetPartition(i, child);
        }
        if (partitions->HasMetaData(i))
        {
          view->GetMetaData(i)->Copy(partitions->GetMetaData(i), 1);
        }
      }
      copy = view;
    }
    else if (auto* table = vtkTable::SafeDownCast(source))
    {
      auto view = vtkSmartPointer<vtkTable>::New();
      export_fields(table->GetRowData(), view->GetRowData());
      copy = view;
    }
    else
    {
      error = "Unsupported VTK export object.";
      return nullptr;
    }
    copy->GetInformation()->Copy(source->GetInformation(), 1);
    export_fields(source->GetFieldData(), copy->GetFieldData());
    return copy;
  }

  struct ExportTask
  {
    vtkDataObject* data;
    fs::path path;
  };

  struct Workers
  {
    std::atomic<bool> stop{false};
    std::vector<std::thread> threads;

    ~Workers()
    {
      stop.store(true, std::memory_order_relaxed);
      for (auto& thread : threads)
      {
        thread.join();
      }
    }
  };

  bool write(vtkDataObject* data, const fs::path& path, std::string& error)
  {
    vtkSmartPointer<vtkXMLWriterBase> writer;
    if (vtkUnstructuredGrid::SafeDownCast(data))
    {
      writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
    }
    else if (vtkPartitionedDataSetCollection::SafeDownCast(data))
    {
      auto collection_writer = vtkSmartPointer<vtkXMLPartitionedDataSetCollectionWriter>::New();
      // All partitions are already resident in this process. Never inherit a
      // host-global distributed controller for this local export operation.
      collection_writer->SetController(nullptr);
      writer = collection_writer;
    }
    else if (vtkTable::SafeDownCast(data))
    {
      writer = vtkSmartPointer<vtkXMLTableWriter>::New();
    }
    else
    {
      error = "Unsupported or missing VTK export object.";
      return false;
    }
    writer->SetFileName(path.u8string().c_str());
    writer->SetInputDataObject(data);
    writer->SetDataModeToAppended();
    if (writer->Write() != 1 || writer->GetErrorCode() != 0)
    {
      error = "VTK failed to write " + path.u8string();
      return false;
    }
    return true;
  }

  bool write_tasks(const std::vector<ExportTask>& tasks, std::size_t count,
    const std::function<bool(double)>& observe, std::string& error)
  {
    if (!observe(0.0))
    {
      return false;
    }
    if (count == 1)
    {
      for (std::size_t i = 0; i < tasks.size(); ++i)
      {
        auto view = export_view(tasks[i].data, error);
        if (!view || !write(view, tasks[i].path, error) ||
          !observe(static_cast<double>(i + 1) / tasks.size()))
        {
          return false;
        }
      }
      return true;
    }

    std::mutex preparation;
    std::mutex state;
    std::condition_variable ready;
    std::size_t next = 0;
    std::size_t completed = 0;
    std::size_t finished = 0;
    std::string worker_error;
    // Declared after worker state so joining also precedes its destruction
    // during cancellation, exceptions, or a partial thread-start failure.
    Workers workers;
    const auto fail = [&](const std::string& message)
    {
      std::lock_guard<std::mutex> lock(state);
      if (worker_error.empty())
      {
        worker_error = message;
      }
      workers.stop.store(true, std::memory_order_relaxed);
    };
    workers.threads.reserve(count);
    for (std::size_t i = 0; i < count; ++i)
    {
      workers.threads.emplace_back([&, fail]()
      {
        try
        {
          while (!workers.stop.load(std::memory_order_relaxed))
          {
            std::size_t task_index;
            {
              std::lock_guard<std::mutex> lock(state);
              if (next == tasks.size() || workers.stop.load(std::memory_order_relaxed))
              {
                break;
              }
              task_index = next++;
            }
            std::string task_error;
            vtkSmartPointer<vtkDataObject> view;
            {
              std::lock_guard<std::mutex> lock(preparation);
              if (workers.stop.load(std::memory_order_relaxed))
              {
                break;
              }
              view = export_view(tasks[task_index].data, task_error);
            }
            if (workers.stop.load(std::memory_order_relaxed))
            {
              break;
            }
            if (!view || !write(view, tasks[task_index].path, task_error))
            {
              fail(task_error.empty() ? "VTK export worker failed." : task_error);
              break;
            }
            {
              std::lock_guard<std::mutex> lock(state);
              ++completed;
            }
            ready.notify_one();
          }
        }
        catch (const std::exception& failure)
        {
          fail(failure.what());
        }
        catch (...)
        {
          fail("Unexpected VTK export worker failure.");
        }
        {
          std::lock_guard<std::mutex> lock(state);
          ++finished;
        }
        ready.notify_one();
      });
    }
    std::size_t observed = 0;
    while (true)
    {
      std::unique_lock<std::mutex> lock(state);
      ready.wait_for(lock, std::chrono::milliseconds(20), [&]()
      {
        return completed != observed || finished == count || !worker_error.empty();
      });
      if (!worker_error.empty())
      {
        error = worker_error;
        return false;
      }
      const double fraction = static_cast<double>(completed) / tasks.size();
      observed = completed;
      const bool done = finished == count;
      lock.unlock();
      if (!observe(fraction))
      {
        return false;
      }
      if (done)
      {
        return true;
      }
    }
  }
}

bool SimResults::export_vtk_result(const VtkResultsData& result, const std::string& destination,
  std::string& index_path, std::string& error, std::size_t workers,
  const std::function<bool(double)>& poll, std::size_t* workers_used)
{
  error.clear();
  if (workers_used)
  {
    *workers_used = 0;
  }
  try
  {
    if (result.increments.empty() || destination.empty() || workers == 0)
    {
      error = "A complete result, a new export directory and at least one export worker are required.";
      return false;
    }
    const auto target = fs::absolute(fs::u8path(destination)).lexically_normal();
    if (fs::exists(target) || target.filename().empty())
    {
      error = "The VTK export destination must be a new directory.";
      return false;
    }
    fs::create_directories(target.parent_path());
    Staging stage;
    const auto token = std::chrono::steady_clock::now().time_since_epoch().count();
    for (int attempt = 0; attempt < 32; ++attempt)
    {
      const auto candidate = target.parent_path() / (target.filename().u8string() + ".tmp." + std::to_string(token) + "." + std::to_string(attempt));
      if (fs::create_directory(candidate))
      {
        stage.path = candidate;
        break;
      }
    }
    if (stage.path.empty())
    {
      error = "Cannot reserve a VTK export staging directory.";
      return false;
    }
    std::ostringstream entries;
    std::vector<ExportTask> tasks;
    entries.imbue(std::locale::classic());
    entries << std::setprecision(17);
    const auto entry = [&](const std::string& file, double time, const std::string& group, std::size_t part)
    {
      entries << "    <DataSet timestep=\"" << time << "\" group=\"" << xml(group)
              << "\" part=\"" << part << "\" file=\"" << xml(file) << "\"/>\n";
    };
    const auto observe = [&](double fraction)
    {
      if (poll && !poll(fraction))
      {
        error = "VTK export cancelled.";
        return false;
      }
      return true;
    };
    for (std::size_t i = 0; i < result.increments.size(); ++i)
    {
      if (!observe(0.0))
      {
        return false;
      }
      const auto& increment = result.increments[i];
      if (!std::isfinite(increment.time) || !increment.dataset ||
        (result.layout == DatasetLayout::SingleGrid && !vtkUnstructuredGrid::SafeDownCast(increment.dataset.get())) ||
        (result.layout == DatasetLayout::Collection && !vtkPartitionedDataSetCollection::SafeDownCast(increment.dataset.get())))
      {
        error = "Invalid VTK increment layout or time.";
        return false;
      }
      const auto stem = "increment_" + std::to_string(i + 1);
      const auto file = stem + (result.layout == DatasetLayout::SingleGrid ? ".vtu" : ".vtpc");
      tasks.push_back({increment.dataset.get(), stage.path / file});
      // Distinct modes can have equal frequency/time. Retain separate PVD
      // groups rather than giving them the same time/group/part identity.
      const std::string identity = increment.mode < 0 ? std::string{} :
        "_Mode_" + std::to_string(increment.mode) + "_Analysis_" + std::to_string(increment.analysis_type) +
        "_Diameter_" + std::to_string(increment.nodal_diameter) + "_Subcase_" + std::to_string(increment.subcase);
      entry(file, increment.time, "Mesh" + identity, 0);
      for (std::size_t t = 0; t < increment.tables.size(); ++t)
      {
        const auto table = stem + "_table_" + std::to_string(t + 1) + ".vtt";
        if (!increment.tables[t])
        {
          error = "Missing VTK increment table.";
          return false;
        }
        tasks.push_back({increment.tables[t].get(), stage.path / table});
        entry(table, increment.time, "IncrementTables" + identity, t);
      }
    }
    for (std::size_t t = 0; t < result.tables.size(); ++t)
    {
      if (!observe(0.0))
      {
        return false;
      }
      const auto table = "summary_table_" + std::to_string(t + 1) + ".vtt";
      if (!result.tables[t])
      {
        error = "Missing VTK summary table.";
        return false;
      }
      tasks.push_back({result.tables[t].get(), stage.path / table});
      entry(table, result.increments.front().time, "SummaryTables", t);
    }
    const auto count = std::min(workers, tasks.size());
    if (workers_used)
    {
      *workers_used = count;
    }
    if (!write_tasks(tasks, count, observe, error))
    {
      return false;
    }
    std::ofstream index(stage.path / "results.pvd", std::ios::binary);
    index << "<?xml version=\"1.0\"?>\n<VTKFile type=\"Collection\" version=\"0.1\" byte_order=\"LittleEndian\">\n  <Collection>\n"
          << entries.str() << "  </Collection>\n</VTKFile>\n";
    index.close();
    if (!index)
    {
      error = "Cannot write the VTK time-series index.";
      return false;
    }
    if (!observe(1.0))
    {
      return false;
    }
    fs::rename(stage.path, target);
    stage.path.clear();
    index_path = (target / "results.pvd").u8string();
    return true;
  }
  catch (const std::exception& failure)
  {
    error = "VTK export failed: " + std::string(failure.what());
    return false;
  }
  catch (...)
  {
    error = "Unexpected VTK export failure.";
    return false;
  }
}
