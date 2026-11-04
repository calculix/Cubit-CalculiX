#include "VtkResultsExporter.hpp"
#include <vtkDataObject.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLPartitionedDataSetCollectionWriter.h>
#include <vtkXMLTableWriter.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLWriterBase.h>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <locale>
#include <sstream>
#include <exception>

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
}

bool FeaResults::export_vtk_result(const VtkResultsData& result, const std::string& destination,
  std::string& index_path, std::string& error, const std::function<bool(double)>& poll)
{
  error.clear();
  try
  {
    if (result.increments.empty() || destination.empty())
    {
      error = "A complete result and a new export directory are required.";
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
      if (!observe(static_cast<double>(i) / result.increments.size()))
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
      if (!write(increment.dataset.get(), stage.path / file, error))
      {
        return false;
      }
      entry(file, increment.time, "Mesh", 0);
      for (std::size_t t = 0; t < increment.tables.size(); ++t)
      {
        const auto table = stem + "_table_" + std::to_string(t + 1) + ".vtt";
        if (!write(increment.tables[t].get(), stage.path / table, error))
        {
          return false;
        }
        entry(table, increment.time, "IncrementTables", t);
      }
    }
    for (std::size_t t = 0; t < result.tables.size(); ++t)
    {
      if (!observe(0.99))
      {
        return false;
      }
      const auto table = "summary_table_" + std::to_string(t + 1) + ".vtt";
      if (!write(result.tables[t].get(), stage.path / table, error))
      {
        return false;
      }
      entry(table, result.increments.front().time, "SummaryTables", t);
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
