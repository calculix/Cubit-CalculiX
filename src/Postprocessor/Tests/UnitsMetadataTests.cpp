#include "Framework/Results/VtkResultsMetadataCodec.hpp"
#include "Framework/Results/ResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include "Postprocessor/Core/representations/ResultColoring.hpp"
#include <vtkCellArray.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkFieldData.h>
#include <vtkIntArray.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkStringArray.h>
#include <vtkTable.h>
#include <vtkUnstructuredGrid.h>
#include <vtkXMLUnstructuredGridReader.h>
#include <vtkXMLUnstructuredGridWriter.h>
#include <vtkXMLTableReader.h>
#include <vtkXMLTableWriter.h>
#include <cstdlib>
#include <iostream>
#include <cmath>
#include <string>

int main()
{
  std::string error;
  auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
  auto points = vtkSmartPointer<vtkPoints>::New();
  points->InsertNextPoint(0.0, 0.0, 0.0);
  points->InsertNextPoint(1.0, 0.0, 0.0);
  grid->SetPoints(points);
  const vtkIdType nodes[] = {0, 1};
  grid->InsertNextCell(VTK_LINE, 2, nodes);
  auto stress = vtkSmartPointer<vtkDoubleArray>::New();
  stress->SetName("stress");
  stress->InsertNextValue(123.0);
  stress->InsertNextValue(456.0);
  grid->GetPointData()->AddArray(stress);
  auto material = vtkSmartPointer<vtkIntArray>::New();
  material->SetName("MaterialId");
  material->InsertNextValue(7);
  grid->GetCellData()->AddArray(material);
  SimResults::VtkResultsData source;
  source.source_units.system = "N-mm-s";
  source.source_units.coordinate_unit = "mm";
  source.source_units.time_unit = "s";
  source.source_workspace = "calculix";
  source.source_project = "fixture-project";
  source.solver_launch_id = "fixture-launch";
  source.run_identity = "fixture-run";
  source.result_identity = "fixture-result";
  SimResults::Dataset dataset;
  dataset.material_id_array = "MaterialId";
  dataset.material_workspace = "calculix";
  dataset.material_project = "fixture-project";
  source.datasets.push_back(dataset);
  SimResults::Increment frame;
  grid->Register(nullptr);
  frame.dataset = std::shared_ptr<vtkDataObject>(grid.GetPointer(), [](vtkDataObject* value) {
    value->Delete();
  });
  SimResults::Field field;
  field.array_name = "stress";
  field.name = "Stress";
  field.source = "FRD";
  field.components = {"XX"};
  SimUnits::QuantityUnit quantity;
  if (!(framework_units().system_unit("N-mm-s", "stress", quantity, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  field.component_units.push_back(quantity);
  frame.fields.push_back(field);
  source.increments.push_back(frame);
  if (!(SimResults::VtkResultsMetadataCodec::write(source, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  auto writer = vtkSmartPointer<vtkXMLUnstructuredGridWriter>::New();
  writer->SetInputData(grid);
  writer->SetWriteToOutputString(true);
  writer->SetDataModeToAscii();
  if (!(writer->Write() == 1))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  auto reader = vtkSmartPointer<vtkXMLUnstructuredGridReader>::New();
  reader->SetReadFromInputString(true);
  reader->SetInputString(writer->GetOutputString());
  reader->Update();
  SimResults::VtkResultsData restored;
  restored.datasets.emplace_back();
  SimResults::Increment restoredFrame;
  reader->GetOutput()->Register(nullptr);
  restoredFrame.dataset = std::shared_ptr<vtkDataObject>(reader->GetOutput(), [](vtkDataObject* value) {
    value->Delete();
  });
  restored.increments.push_back(restoredFrame);
  if (!(SimResults::VtkResultsMetadataCodec::read(restored, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restored.source_units.system == "N-mm-s"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restored.solver_launch_id == "fixture-launch"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restored.datasets.front().material_project == "fixture-project"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restored.increments.front().fields.front().component_units.front().unit == "N/mm^2"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(framework_units().set_display_unit("postprocessor", "stress", "Pa", error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  Postprocessor::FieldSelection selection;
  selection.component = "XX";
  std::vector<double> values;
  if (!(Postprocessor::color_values(reader->GetOutput(), restored.increments.front().fields.front(), selection, values, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(std::abs(values.front() - 123000000.0) < 1e-5))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(stress->GetValue(0) == 123.0))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(reader->GetOutput()->GetPointData()->GetArray("stress")->GetComponent(0, 0) == 123.0))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  auto categorical = field;
  categorical.categorical = true;
  double converted = 0.0;
  std::string unit;
  if (!(!SimResults::ResultsUnits::field_value(7.0, categorical, "XX", false, "postprocessor", converted, unit, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  SimUnits::QuantityUnit unknown;
  if (!(SimResults::ResultsUnits::display_value(42.0, unknown, "postprocessor", converted, unit, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(converted == 42.0 && unit == "unknown unit"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(SimResults::ResultsUnits::display_value(123.0, quantity, "report", converted, unit, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(converted == 123.0 && unit == "N/mm^2"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  auto table = vtkSmartPointer<vtkTable>::New();
  table->AddColumn(stress);
  table->Register(nullptr);
  source.tables.emplace_back(table.GetPointer(), [](vtkTable* value) {
    value->Delete();
  });
  SimResults::ResultsTableMetadata tableMetadata;
  tableMetadata.name = "Stress history";
  tableMetadata.columns = {quantity};
  source.table_metadata.push_back(tableMetadata);
  if (!(SimResults::VtkResultsMetadataCodec::write(source, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  auto tableWriter = vtkSmartPointer<vtkXMLTableWriter>::New();
  tableWriter->SetInputData(table);
  tableWriter->SetWriteToOutputString(true);
  if (!(tableWriter->Write() == 1))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  auto tableReader = vtkSmartPointer<vtkXMLTableReader>::New();
  tableReader->SetReadFromInputString(true);
  tableReader->SetInputString(tableWriter->GetOutputString());
  tableReader->Update();
  SimResults::VtkResultsData restoredTable;
  tableReader->GetOutput()->Register(nullptr);
  restoredTable.tables.emplace_back(tableReader->GetOutput(), [](vtkTable* value) {
    value->Delete();
  });
  if (!(SimResults::VtkResultsMetadataCodec::read(restoredTable, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restoredTable.table_metadata.front().name == "Stress history"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restoredTable.table_metadata.front().columns.front().unit == "N/mm^2"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restoredTable.solver_launch_id == "fixture-launch"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  auto partitions = vtkSmartPointer<vtkPartitionedDataSet>::New();
  partitions->SetPartition(0, grid);
  partitions->SetPartition(1, reader->GetOutput());
  if (!(SimResults::VtkResultsMetadataCodec::write_frame(*partitions, source, 0, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  SimResults::VtkResultsData restoredPartitions;
  restoredPartitions.datasets.emplace_back();
  restoredPartitions.increments.emplace_back();
  partitions->Register(nullptr);
  restoredPartitions.increments.front().dataset = std::shared_ptr<vtkDataObject>(partitions.GetPointer(), [](vtkDataObject* value) {
    value->Delete();
  });
  if (!(SimResults::VtkResultsMetadataCodec::read(restoredPartitions, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restoredPartitions.increments.front().fields.size() == 1))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  auto* metadata = vtkStringArray::SafeDownCast(reader->GetOutput()->GetFieldData()->GetAbstractArray("CubitSim.ResultsMetadata"));
  if (!(metadata != nullptr))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  metadata->SetValue(0, "{\"version\":999}");
  if (!(!SimResults::VtkResultsMetadataCodec::read(restoredPartitions, error)))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restoredPartitions.solver_launch_id == "fixture-launch"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  if (!(restoredPartitions.increments.front().fields.front().component_units.front().unit == "N/mm^2"))
  {
    std::cerr << "Units metadata check failed at line " << __LINE__ << ": " << error << '\n';
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}
