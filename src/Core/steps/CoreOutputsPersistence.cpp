#include "Core/steps/CoreOutputsPersistence.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "HDF5Tool.hpp"

void CoreOutputsPersistence::read(HDF5Tool& file, CoreHistoryOutputs& outputs)
{
  file.read_dataset_int_rank_2("outputs_data", "Cubit-CalculiX/HistoryOutputs", outputs.outputs_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/HistoryOutputs", outputs.name_data);
  file.read_dataset_string_rank_2("node_data", "Cubit-CalculiX/HistoryOutputs", outputs.node_data);
  file.read_dataset_string_rank_2("element_data", "Cubit-CalculiX/HistoryOutputs", outputs.element_data);
  file.read_dataset_string_rank_2("contact_data", "Cubit-CalculiX/HistoryOutputs", outputs.contact_data);
  file.read_dataset_string_rank_2("section_data", "Cubit-CalculiX/HistoryOutputs", outputs.section_data);
}

void CoreOutputsPersistence::write(HDF5Tool& file, const CoreHistoryOutputs& outputs)
{
  file.createGroup("Cubit-CalculiX/HistoryOutputs");
  file.write_dataset_int_rank_2("outputs_data", "Cubit-CalculiX/HistoryOutputs", outputs.outputs_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/HistoryOutputs", outputs.name_data);
  file.write_dataset_string_rank_2("node_data", "Cubit-CalculiX/HistoryOutputs", outputs.node_data);
  file.write_dataset_string_rank_2("element_data", "Cubit-CalculiX/HistoryOutputs", outputs.element_data);
  file.write_dataset_string_rank_2("contact_data", "Cubit-CalculiX/HistoryOutputs", outputs.contact_data);
  file.write_dataset_string_rank_2("section_data", "Cubit-CalculiX/HistoryOutputs", outputs.section_data);
}

void CoreOutputsPersistence::read(HDF5Tool& file, CoreFieldOutputs& outputs)
{
  file.read_dataset_int_rank_2("outputs_data", "Cubit-CalculiX/FieldOutputs", outputs.outputs_data);
  file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/FieldOutputs", outputs.name_data);
  file.read_dataset_string_rank_2("node_data", "Cubit-CalculiX/FieldOutputs", outputs.node_data);
  file.read_dataset_string_rank_2("element_data", "Cubit-CalculiX/FieldOutputs", outputs.element_data);
  file.read_dataset_string_rank_2("contact_data", "Cubit-CalculiX/FieldOutputs", outputs.contact_data);
}

void CoreOutputsPersistence::write(HDF5Tool& file, const CoreFieldOutputs& outputs)
{
  file.createGroup("Cubit-CalculiX/FieldOutputs");
  file.write_dataset_int_rank_2("outputs_data", "Cubit-CalculiX/FieldOutputs", outputs.outputs_data);
  file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/FieldOutputs", outputs.name_data);
  file.write_dataset_string_rank_2("node_data", "Cubit-CalculiX/FieldOutputs", outputs.node_data);
  file.write_dataset_string_rank_2("element_data", "Cubit-CalculiX/FieldOutputs", outputs.element_data);
  file.write_dataset_string_rank_2("contact_data", "Cubit-CalculiX/FieldOutputs", outputs.contact_data);
}
