#include "Core/results/vtk/CoreResultsVtkConverter.hpp"
#include "Core/results/vtk/CoreResultsVtkElements.hpp"
#include "Core/results/frd/CoreResultsFrd.hpp"
#include "Core/results/dat/CoreResultsDat.hpp"
#include "Framework/Results/VtkResultsExporter.hpp"
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTable.h>
#include <vtkUnstructuredGrid.h>
#include <vtkVersion.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLTableReader.h>
#include <vtkXMLPartitionedDataSetCollectionReader.h>
#include <atomic>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <iostream>
#include <numeric>

namespace
{
  int failures = 0;
  void check(bool condition, const std::string& message)
  {
    if (!condition)
    {
      std::cerr << "FAIL: " << message << '\n';
      ++failures;
    }
  }

  vtkUnstructuredGrid* grid(const FeaResults::VtkResultsData& result, std::size_t increment, unsigned int domain)
  {
    auto* collection = vtkPartitionedDataSetCollection::SafeDownCast(result.increments[increment].dataset.get());
    return collection ? vtkUnstructuredGrid::SafeDownCast(collection->GetPartition(domain, 0))
      : vtkUnstructuredGrid::SafeDownCast(result.increments[increment].dataset.get());
  }

  CoreResultsVtkPlan fixture(CoreResultsFrd& frd, CoreResultsDat& dat)
  {
    CoreResultsVtkPlan plan;
    plan.job_id = 1;
    plan.run_identity = "test";
    plan.options.workers = 1;
    plan.layout = FeaResults::DatasetLayout::Collection;
    const std::vector<ElementIntegration::Coordinates> nodes = {
      {{-1,-1,-1}}, {{1,-1,-1}}, {{1,1,-1}}, {{-1,1,-1}},
      {{-1,-1,1}}, {{1,-1,1}}, {{1,1,1}}, {{-1,1,1}}};
    for (std::size_t n = 0; n < nodes.size(); ++n)
    {
      plan.mesh.nodes.push_back({static_cast<int>(n + 1), nodes[n]});
    }
    plan.mesh.elements.push_back({11, 1, "C3D8", {1,2,3,4,5,6,7,8}});
    CoreResultsVtkGroup block;
    block.kind = CoreResultsVtkGroupKind::Block;
    block.name = "Block";
    block.ids = {11};
    plan.mesh.groups.push_back(block);
    CoreResultsVtkGroup ip;
    ip.kind = CoreResultsVtkGroupKind::DatIntegrationPoints;
    ip.name = "ALL";
    ip.ids = {11};
    plan.mesh.groups.push_back(ip);
    frd.result_block_type = {"DISP"};
    dat.buckle_data = {{{1.0, 1.5}, {2.0, 2.0}}};
    plan.tables.push_back({CoreResultsVtkTablePlan::Kind::Buckling, 0});
    std::vector<ElementIntegration::IntegrationPoint> rule;
    check(ElementIntegration::get_rule("C3D8", rule), "C3D8 quadrature rule");
    for (int frame = 0; frame < 2; ++frame)
    {
      const double time = frame + 1.0;
      frd.total_times.push_back(time);
      frd.result_blocks.push_back({frame, 1, frame + 1, frame + 1, frame, 0, frame});
      frd.result_block_components.push_back({"D1", "D2", "D3"});
      std::vector<std::vector<double>> displacements;
      std::vector<std::vector<int>> node_rows;
      for (std::size_t n = 0; n < nodes.size(); ++n)
      {
        displacements.push_back({time * (2.0 + nodes[n][0]), time * nodes[n][1], time * nodes[n][2]});
        node_rows.push_back({static_cast<int>(n + 1), static_cast<int>(n)});
      }
      frd.result_block_data.push_back(displacements);
      frd.result_block_node_data.push_back(node_rows);
      std::vector<std::vector<double>> stresses;
      std::vector<std::vector<int>> element_rows;
      for (std::size_t q = 0; q < rule.size(); ++q)
      {
        stresses.push_back({11, static_cast<double>(q + 1), time * (q + 1)});
        element_rows.push_back({11, static_cast<int>(q), 2});
      }
      dat.result_block_data.push_back(stresses);
      dat.result_block_c1_data.push_back(element_rows);
      CoreResultsVtkIncrementPlan increment;
      increment.step = 1;
      increment.increment = frame + 1;
      increment.total_increment = frame + 1;
      increment.time = time;
      CoreResultsVtkFieldPlan displacement;
      displacement.source_data = frame;
      displacement.field.name = "DISP";
      displacement.field.source = "FRD";
      displacement.field.components = {"D1", "D2", "D3"};
      increment.fields.push_back(displacement);
      CoreResultsVtkFieldPlan stress;
      stress.source_data = frame;
      stress.component_offset = 2;
      stress.field.name = "STRESS";
      stress.field.source = "DAT";
      stress.field.set_name = "ALL";
      stress.field.association = FeaResults::Association::IntegrationPoint;
      stress.field.components = {"SXX"};
      increment.fields.push_back(stress);
      plan.increments.push_back(increment);
    }
    return plan;
  }

  void ordering()
  {
    using namespace CoreResultsVtkElements;
    std::string error;
    for (const auto& reference : std::vector<std::pair<std::string, double>>{
      {"C3D4", 1.0 / 6.0}, {"C3D10", 1.0 / 6.0}, {"C3D6", 1.0}, {"C3D15", 1.0},
      {"C3D8", 8.0}, {"C3D8R", 8.0}, {"C3D20", 8.0}, {"C3D20R", 8.0}})
    {
      std::vector<ElementIntegration::IntegrationPoint> points;
      check(ElementIntegration::get_rule(reference.first, points), "quadrature " + reference.first);
      double sum = 0.0;
      for (const auto& point : points)
      {
        sum += point.weight;
      }
      check(std::abs(sum - reference.second) < 1e-12, "reference quadrature volume " + reference.first);
    }
    for (const auto& type : {"C3D20", "C3D15", "C3D10", "B32"})
    {
      Description element;
      check(describe(type, element, error), "describe " + std::string(type));
      std::vector<int> original(element.node_count);
      std::iota(original.begin(), original.end(), 1);
      std::vector<int> vtk;
      check(reorder(element, NodeOrder::Exodus, NodeOrder::Vtk, original, vtk, error), error);
      std::vector<int> back;
      check(reorder(element, NodeOrder::Vtk, NodeOrder::Exodus, vtk, back, error) && back == original, "node-order round trip");
      if (std::string(type) == "C3D20")
      {
        check(vtk[12] == 17 && vtk[16] == 13, "HEX20 vertical/top-edge permutation");
      }
    }
  }
}

int main(int argc, char** argv)
{
  check(std::string(vtkVersion::GetVTKVersion()) == "9.4.2", "runtime VTK version");
  check(sizeof(vtkIdType) == 8, "64-bit vtkIdType");
  ordering();
  CoreResultsFrd frd;
  CoreResultsDat dat;
  auto plan = fixture(frd, dat);
  CoreResultsVtkConverter converter;
  std::atomic<bool> cancelled{false};
  std::string error;
  std::shared_ptr<const FeaResults::VtkResultsData> serial;
  check(converter.build(frd, dat, plan, cancelled, {}, serial, error), error);
  if (!serial)
  {
    return 1;
  }
  check(serial->increments.size() == 2 && serial->datasets.size() == 2, "all increments and datasets resident");
  check(grid(*serial, 0, 0)->GetPoints() == grid(*serial, 1, 0)->GetPoints(), "static points shared across increments");
  check(grid(*serial, 0, 0)->GetCells() == grid(*serial, 1, 0)->GetCells(), "static connectivity shared across increments");
  std::vector<ElementIntegration::IntegrationPoint> rule;
  ElementIntegration::get_rule("C3D8", rule);
  for (std::size_t frame = 0; frame < 2; ++frame)
  {
    auto* points = grid(*serial, frame, 1);
    auto* displacement = points->GetPointData()->GetVectors();
    check(displacement != nullptr, "integration-point displacement exists");
    for (std::size_t q = 0; q < rule.size(); ++q)
    {
      double position[3];
      points->GetPoint(static_cast<vtkIdType>(q), position);
      for (int c = 0; c < 3; ++c)
      {
        check(std::abs(position[c] - rule[q].coordinates[c]) < 1e-12, "physical integration-point coordinate");
        if (displacement)
        {
          const double expected = (frame + 1.0) * (rule[q].coordinates[c] + (c == 0 ? 2.0 : 0.0));
          check(std::abs(displacement->GetComponent(static_cast<vtkIdType>(q), c) - expected) < 1e-12, "affine IP displacement interpolation");
        }
      }
    }
  }
  plan.options.workers = 4;
  plan.options.chunk_rows = 1;
  std::shared_ptr<const FeaResults::VtkResultsData> parallel;
  check(converter.build(frd, dat, plan, cancelled, {}, parallel, error), error);
  if (parallel)
  {
    for (std::size_t frame = 0; frame < 2; ++frame)
    {
      for (unsigned int d = 0; d < 2; ++d)
      {
        auto* left = grid(*serial, frame, d)->GetPointData();
        auto* right = grid(*parallel, frame, d)->GetPointData();
        for (int a = 0; a < left->GetNumberOfArrays(); ++a)
        {
          auto* array = left->GetArray(a);
          auto* other = right->GetArray(array->GetName());
          check(other && other->GetNumberOfTuples() == array->GetNumberOfTuples(), "parallel array shape");
          if (other)
          {
            for (vtkIdType n = 0; n < array->GetNumberOfTuples(); ++n)
            {
              for (int c = 0; c < array->GetNumberOfComponents(); ++c)
              {
                check(array->GetComponent(n,c) == other->GetComponent(n,c), "parallel numeric equality");
              }
            }
          }
        }
      }
    }
  }
  auto retained = serial;
  {
    CoreResultsFrd reduced_frd;
    CoreResultsDat reduced_dat;
    auto reduced_plan = fixture(reduced_frd, reduced_dat);
    reduced_plan.mesh.elements[0].calculix_type = "C3D8R";
    for (std::size_t i = 0; i < reduced_dat.result_block_data.size(); ++i)
    {
      reduced_dat.result_block_data[i].resize(1);
      reduced_dat.result_block_c1_data[i].resize(1);
    }
    std::shared_ptr<const FeaResults::VtkResultsData> reduced;
    check(converter.build(reduced_frd, reduced_dat, reduced_plan, cancelled, {}, reduced, error), error);
    if (reduced)
    {
      auto* cloud = grid(*reduced, 0, 1);
      double center[3];
      cloud->GetPoint(0, center);
      check(cloud->GetNumberOfPoints() == 1 && std::abs(center[0]) + std::abs(center[1]) + std::abs(center[2]) < 1e-12,
        "reduced integration point at physical centroid");
      check(std::abs(cloud->GetPointData()->GetVectors()->GetComponent(0, 0) - 2.0) < 1e-12, "reduced IP displacement");
    }
  }
  int polls = 0;
  check(!converter.build(frd, dat, plan, cancelled, [&](double)
  {
    return ++polls < 30;
  }, serial, error) &&
    serial == retained && cancelled.load(), "mid-conversion cancellation joins workers and retains output");
  cancelled.store(false);
  check(!converter.build(frd, dat, plan, cancelled, [](double)
  {
    return false;
  }, serial, error) && serial == retained, "cancel leaves published result intact");
  cancelled.store(false);
  plan.increments[0].fields[0].source_data = 999;
  check(!converter.build(frd, dat, plan, cancelled, {}, serial, error) && serial == retained, "failure leaves published result intact");
  {
    CoreResultsFrd incomplete_frd;
    CoreResultsDat incomplete_dat;
    auto incomplete_plan = fixture(incomplete_frd, incomplete_dat);
    incomplete_frd.result_block_node_data[0].pop_back();
    std::shared_ptr<const FeaResults::VtkResultsData> incomplete;
    check(converter.build(incomplete_frd, incomplete_dat, incomplete_plan, cancelled, {}, incomplete, error), error);
    if (incomplete)
    {
      auto* physical = grid(*incomplete, 0, 0);
      check(std::isnan(physical->GetPointData()->GetVectors()->GetComponent(7, 0)), "missing node becomes NaN");
      check(physical->GetCellData()->GetArray("Partial")->GetComponent(0, 0) == 1, "incomplete cell marked partial");
      for (const auto& field : incomplete->increments[0].fields)
      {
        if (field.name == "DISP" && field.dataset_id == 0)
        {
          check(physical->GetPointData()->GetArray(field.validity_array.c_str())->GetComponent(7, 0) == 0, "missing tuple validity is zero");
        }
      }
    }
    incomplete_plan.options.include_partial = false;
    check(converter.build(incomplete_frd, incomplete_dat, incomplete_plan, cancelled, {}, incomplete, error), error);
    if (incomplete)
    {
      check(grid(*incomplete, 0, 0)->GetPointData()->GetVectors() == nullptr, "no_partial skips incomplete displacement");
    }
  }
  frd.result_block_data.clear();
  dat.result_block_data.clear();
  check(grid(*retained, 0, 0)->GetPointData()->GetVectors()->GetComponent(0,0) == 1.0, "dataset outlives reader buffers");
  if (argc > 1)
  {
    const auto root = std::filesystem::u8path(argv[1]) / std::to_string(std::chrono::steady_clock::now().time_since_epoch().count());
    std::string index;
    check(FeaResults::export_vtk_result(*retained, (root / "collection").u8string(), index, error), error);
    check(std::filesystem::exists(std::filesystem::u8path(index)), "PVD index published");
    check(std::filesystem::exists(root / "collection/summary_table_1.vtt"), "DAT table exported");
    auto table_reader = vtkSmartPointer<vtkXMLTableReader>::New();
    table_reader->SetFileName((root / "collection/summary_table_1.vtt").u8string().c_str());
    table_reader->Update();
    check(table_reader->GetOutput()->GetNumberOfRows() == 2 && table_reader->GetOutput()->GetNumberOfColumns() == 2, "DAT table round trip");
    auto reader = vtkSmartPointer<vtkXMLPartitionedDataSetCollectionReader>::New();
    reader->SetFileName((root / "collection/increment_1.vtpc").u8string().c_str());
    reader->Update();
    auto* restored = vtkPartitionedDataSetCollection::SafeDownCast(reader->GetOutput());
    check(restored && restored->GetNumberOfPartitionedDataSets() == 2, "VTPC round trip");
    FeaResults::VtkResultsData single = *retained;
    single.layout = FeaResults::DatasetLayout::SingleGrid;
    for (std::size_t i = 0; i < single.increments.size(); ++i)
    {
      single.increments[i].dataset = std::shared_ptr<vtkDataObject>(retained, grid(*retained, i, 0));
    }
    check(FeaResults::export_vtk_result(single, (root / "single").u8string(), index, error), error);
    auto single_reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
    single_reader->SetFileName((root / "single/increment_1.vtu").u8string().c_str());
    single_reader->Update();
    check(single_reader->GetOutput()->GetNumberOfCells() == 1, "VTU round trip");
    check(!FeaResults::export_vtk_result(single, (root / "cancelled").u8string(), index, error, [](double)
    {
      return false;
    }) &&
      !std::filesystem::exists(root / "cancelled"), "cancelled export not published");
    check(!FeaResults::export_vtk_result(single, (root / "single").u8string(), index, error), "existing export preserved");
  }
  std::cout << (failures == 0 ? "All VTK dataset checks passed.\n" : "VTK dataset checks failed.\n");
  return failures == 0 ? 0 : 1;
}
