#include "Core/steps/CoreOutputsPersistence.hpp"
#include "Core/steps/CoreHistoryOutputs.hpp"
#include "Core/steps/CoreFieldOutputs.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreOutputsPersistence::read(HDF5Tool& file, CoreHistoryOutputs& outputs)
{
  try
  {

    decltype(outputs.outputs_data) saved_outputs_data;
    decltype(outputs.name_data) saved_name_data;
    decltype(outputs.node_data) saved_node_data;
    decltype(outputs.element_data) saved_element_data;
    decltype(outputs.contact_data) saved_contact_data;
    decltype(outputs.section_data) saved_section_data;
    if (file.nameExists("Cubit-CalculiX/HistoryOutputs/outputs_data") && !file.read_dataset_int_rank_2("outputs_data", "Cubit-CalculiX/HistoryOutputs", saved_outputs_data))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/HistoryOutputs/name_data") && !file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/HistoryOutputs", saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/HistoryOutputs/node_data") && !file.read_dataset_string_rank_2("node_data", "Cubit-CalculiX/HistoryOutputs", saved_node_data))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/HistoryOutputs/element_data") && !file.read_dataset_string_rank_2("element_data", "Cubit-CalculiX/HistoryOutputs", saved_element_data))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/HistoryOutputs/contact_data") && !file.read_dataset_string_rank_2("contact_data", "Cubit-CalculiX/HistoryOutputs", saved_contact_data))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/HistoryOutputs/section_data") && !file.read_dataset_string_rank_2("section_data", "Cubit-CalculiX/HistoryOutputs", saved_section_data))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_outputs_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: outputs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_node_data, 22)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: node_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_element_data, 24)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: element_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_contact_data, 13)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: contact_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_section_data, 9)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_outputs_data, 1, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_outputs_data, 2, 1, 3, saved_node_data)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: invalid node_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_outputs_data, 2, 2, 3, saved_element_data)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: invalid element_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_outputs_data, 2, 3, 3, saved_contact_data)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: invalid contact_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_outputs_data, 2, 4, 3, saved_section_data)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: invalid section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(saved_outputs_data, 2, {1, 2, 3, 4})))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_outputs_data)))
    {
      PRINT_ERROR("Cannot read CalculiX HistoryOutputs: invalid or duplicate IDs.\n");
      return false;
    }
    outputs.outputs_data.swap(saved_outputs_data);
    outputs.name_data.swap(saved_name_data);
    outputs.node_data.swap(saved_node_data);
    outputs.element_data.swap(saved_element_data);
    outputs.contact_data.swap(saved_contact_data);
    outputs.section_data.swap(saved_section_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX HistoryOutputs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX HistoryOutputs: %s\n", error.what());
    return false;
  }
}

bool CoreOutputsPersistence::write(HDF5Tool& file, const CoreHistoryOutputs& outputs)
{
  try
  {

    if (!(PersistenceTable::keyedRows(outputs.outputs_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: outputs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.node_data, 22)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: node_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.element_data, 24)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: element_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.contact_data, 13)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: contact_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.section_data, 9)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: section_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(outputs.outputs_data, 1, outputs.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(outputs.outputs_data, 2, 1, 3, outputs.node_data)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: invalid node_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(outputs.outputs_data, 2, 2, 3, outputs.element_data)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: invalid element_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(outputs.outputs_data, 2, 3, 3, outputs.contact_data)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: invalid contact_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(outputs.outputs_data, 2, 4, 3, outputs.section_data)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: invalid section_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(outputs.outputs_data, 2, {1, 2, 3, 4})))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(outputs.outputs_data)))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/HistoryOutputs"))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("outputs_data", "Cubit-CalculiX/HistoryOutputs", outputs.outputs_data))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/HistoryOutputs", outputs.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("node_data", "Cubit-CalculiX/HistoryOutputs", outputs.node_data))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("element_data", "Cubit-CalculiX/HistoryOutputs", outputs.element_data))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("contact_data", "Cubit-CalculiX/HistoryOutputs", outputs.contact_data))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("section_data", "Cubit-CalculiX/HistoryOutputs", outputs.section_data))
    {
      PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX HistoryOutputs: %s\n", error.what());
    return false;
  }
}

bool CoreOutputsPersistence::read(HDF5Tool& file, CoreFieldOutputs& outputs)
{
  try
  {

    decltype(outputs.outputs_data) saved_outputs_data;
    decltype(outputs.name_data) saved_name_data;
    decltype(outputs.node_data) saved_node_data;
    decltype(outputs.element_data) saved_element_data;
    decltype(outputs.contact_data) saved_contact_data;
    if (file.nameExists("Cubit-CalculiX/FieldOutputs/outputs_data") && !file.read_dataset_int_rank_2("outputs_data", "Cubit-CalculiX/FieldOutputs", saved_outputs_data))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/FieldOutputs/name_data") && !file.read_dataset_string_rank_2("name_data", "Cubit-CalculiX/FieldOutputs", saved_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/FieldOutputs/node_data") && !file.read_dataset_string_rank_2("node_data", "Cubit-CalculiX/FieldOutputs", saved_node_data))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/FieldOutputs/element_data") && !file.read_dataset_string_rank_2("element_data", "Cubit-CalculiX/FieldOutputs", saved_element_data))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/FieldOutputs/contact_data") && !file.read_dataset_string_rank_2("contact_data", "Cubit-CalculiX/FieldOutputs", saved_contact_data))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_outputs_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: outputs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_node_data, 40)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: node_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_element_data, 35)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: element_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_contact_data, 9)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: contact_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_outputs_data, 1, saved_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_outputs_data, 2, 1, 3, saved_node_data)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: invalid node_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_outputs_data, 2, 2, 3, saved_element_data)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: invalid element_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_outputs_data, 2, 3, 3, saved_contact_data)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: invalid contact_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(saved_outputs_data, 2, {1, 2, 3})))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_outputs_data)))
    {
      PRINT_ERROR("Cannot read CalculiX FieldOutputs: invalid or duplicate IDs.\n");
      return false;
    }
    outputs.outputs_data.swap(saved_outputs_data);
    outputs.name_data.swap(saved_name_data);
    outputs.node_data.swap(saved_node_data);
    outputs.element_data.swap(saved_element_data);
    outputs.contact_data.swap(saved_contact_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX FieldOutputs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX FieldOutputs: %s\n", error.what());
    return false;
  }
}

bool CoreOutputsPersistence::write(HDF5Tool& file, const CoreFieldOutputs& outputs)
{
  try
  {

    if (!(PersistenceTable::keyedRows(outputs.outputs_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: outputs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.node_data, 40)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: node_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.element_data, 35)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: element_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(outputs.contact_data, 9)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: contact_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(outputs.outputs_data, 1, outputs.name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: invalid name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(outputs.outputs_data, 2, 1, 3, outputs.node_data)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: invalid node_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(outputs.outputs_data, 2, 2, 3, outputs.element_data)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: invalid element_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(outputs.outputs_data, 2, 3, 3, outputs.contact_data)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: invalid contact_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(outputs.outputs_data, 2, {1, 2, 3})))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(outputs.outputs_data)))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/FieldOutputs"))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("outputs_data", "Cubit-CalculiX/FieldOutputs", outputs.outputs_data))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("name_data", "Cubit-CalculiX/FieldOutputs", outputs.name_data))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("node_data", "Cubit-CalculiX/FieldOutputs", outputs.node_data))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("element_data", "Cubit-CalculiX/FieldOutputs", outputs.element_data))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("contact_data", "Cubit-CalculiX/FieldOutputs", outputs.contact_data))
    {
      PRINT_ERROR("Cannot write CalculiX FieldOutputs: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX FieldOutputs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX FieldOutputs: %s\n", error.what());
    return false;
  }
}
