#include "Core/interactions/CoreInteractionsPersistence.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#include "PersistenceTable.hpp"
#include <exception>

bool CoreInteractionsPersistence::read(HDF5Tool& file, CoreConstraints& constraints)
{
  try
  {

    decltype(constraints.constraints_data) saved_constraints_data;
    decltype(constraints.rigidbody_constraint_data) saved_rigidbody_constraint_data;
    decltype(constraints.tie_constraint_data) saved_tie_constraint_data;
    decltype(constraints.equation_constraint_data) saved_equation_constraint_data;
    decltype(constraints.equation_data) saved_equation_data;
    decltype(constraints.equation_group_constraint_data) saved_equation_group_constraint_data;
    decltype(constraints.equation_group_data) saved_equation_group_data;
    if (file.nameExists("Cubit-CalculiX/Constraints/constraints_data") && !file.read_dataset_int_rank_2("constraints_data", "Cubit-CalculiX/Constraints", saved_constraints_data))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/Constraints/rigidbody_constraint_data") && !file.read_dataset_string_rank_2("rigidbody_constraint_data", "Cubit-CalculiX/Constraints", saved_rigidbody_constraint_data))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/Constraints/tie_constraint_data") && !file.read_dataset_string_rank_2("tie_constraint_data", "Cubit-CalculiX/Constraints", saved_tie_constraint_data))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/Constraints/equation_constraint_data") && !file.read_dataset_string_rank_2("equation_constraint_data", "Cubit-CalculiX/Constraints", saved_equation_constraint_data))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/Constraints/equation_data") && !file.read_dataset_double_rank_2("equation_data", "Cubit-CalculiX/Constraints", saved_equation_data))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/Constraints/equation_group_constraint_data") && !file.read_dataset_string_rank_2("equation_group_constraint_data", "Cubit-CalculiX/Constraints", saved_equation_group_constraint_data))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/Constraints/equation_group_data") && !file.read_dataset_double_rank_2("equation_group_data", "Cubit-CalculiX/Constraints", saved_equation_group_data))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_constraints_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: constraints_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_rigidbody_constraint_data, 5)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: rigidbody_constraint_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_tie_constraint_data, 5)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: tie_constraint_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_equation_constraint_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: equation_constraint_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_equation_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: equation_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_equation_group_constraint_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: equation_group_constraint_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_equation_group_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: equation_group_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_constraints_data, 1, 1, 2, saved_rigidbody_constraint_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: invalid rigidbody_constraint_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_constraints_data, 1, 2, 2, saved_tie_constraint_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: invalid tie_constraint_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_constraints_data, 1, 3, 2, saved_equation_constraint_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: invalid equation_constraint_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_constraints_data, 1, 4, 2, saved_equation_group_constraint_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: invalid equation_group_constraint_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(saved_constraints_data, 1, {1, 2, 3, 4})))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_constraints_data)))
    {
      PRINT_ERROR("Cannot read CalculiX Constraints: invalid or duplicate IDs.\n");
      return false;
    }
    constraints.constraints_data.swap(saved_constraints_data);
    constraints.rigidbody_constraint_data.swap(saved_rigidbody_constraint_data);
    constraints.tie_constraint_data.swap(saved_tie_constraint_data);
    constraints.equation_constraint_data.swap(saved_equation_constraint_data);
    constraints.equation_data.swap(saved_equation_data);
    constraints.equation_group_constraint_data.swap(saved_equation_group_constraint_data);
    constraints.equation_group_data.swap(saved_equation_group_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX Constraints: %s\n", error.what());
    return false;
  }
}

bool CoreInteractionsPersistence::write(HDF5Tool& file, const CoreConstraints& constraints)
{
  try
  {

    if (!(PersistenceTable::keyedRows(constraints.constraints_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: constraints_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(constraints.rigidbody_constraint_data, 5)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: rigidbody_constraint_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(constraints.tie_constraint_data, 5)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: tie_constraint_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(constraints.equation_constraint_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: equation_constraint_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(constraints.equation_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: equation_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(constraints.equation_group_constraint_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: equation_group_constraint_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(constraints.equation_group_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: equation_group_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(constraints.constraints_data, 1, 1, 2, constraints.rigidbody_constraint_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: invalid rigidbody_constraint_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(constraints.constraints_data, 1, 2, 2, constraints.tie_constraint_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: invalid tie_constraint_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(constraints.constraints_data, 1, 3, 2, constraints.equation_constraint_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: invalid equation_constraint_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(constraints.constraints_data, 1, 4, 2, constraints.equation_group_constraint_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: invalid equation_group_constraint_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::types(constraints.constraints_data, 1, {1, 2, 3, 4})))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: unknown feature type.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(constraints.constraints_data)))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/Constraints"))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("constraints_data", "Cubit-CalculiX/Constraints", constraints.constraints_data))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("rigidbody_constraint_data", "Cubit-CalculiX/Constraints", constraints.rigidbody_constraint_data))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("tie_constraint_data", "Cubit-CalculiX/Constraints", constraints.tie_constraint_data))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("equation_constraint_data", "Cubit-CalculiX/Constraints", constraints.equation_constraint_data))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("equation_data", "Cubit-CalculiX/Constraints", constraints.equation_data))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("equation_group_constraint_data", "Cubit-CalculiX/Constraints", constraints.equation_group_constraint_data))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_double_rank_2("equation_group_data", "Cubit-CalculiX/Constraints", constraints.equation_group_data))
    {
      PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX Constraints: %s\n", error.what());
    return false;
  }
}

bool CoreInteractionsPersistence::read(HDF5Tool& file, CoreSurfaceInteractions& interactions)
{
  try
  {

    decltype(interactions.surfaceinteractions_data) saved_surfaceinteractions_data;
    decltype(interactions.surfaceinteraction_name_data) saved_surfaceinteraction_name_data;
    decltype(interactions.exponential_surfacebehavior_data) saved_exponential_surfacebehavior_data;
    decltype(interactions.linear_surfacebehavior_data) saved_linear_surfacebehavior_data;
    decltype(interactions.tabular_surfacebehavior_data) saved_tabular_surfacebehavior_data;
    decltype(interactions.tied_surfacebehavior_data) saved_tied_surfacebehavior_data;
    decltype(interactions.gap_conductance_data) saved_gap_conductance_data;
    decltype(interactions.gap_heat_generation_data) saved_gap_heat_generation_data;
    decltype(interactions.friction_data) saved_friction_data;
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/surfaceinteractions_data") && !file.read_dataset_int_rank_2("surfaceinteractions_data", "Cubit-CalculiX/SurfaceInteractions", saved_surfaceinteractions_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/surfaceinteraction_name_data") && !file.read_dataset_string_rank_2("surfaceinteraction_name_data", "Cubit-CalculiX/SurfaceInteractions", saved_surfaceinteraction_name_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/exponential_surfacebehavior_data") && !file.read_dataset_string_rank_2("exponential_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", saved_exponential_surfacebehavior_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/linear_surfacebehavior_data") && !file.read_dataset_string_rank_2("linear_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", saved_linear_surfacebehavior_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/tabular_surfacebehavior_data") && !file.read_dataset_string_rank_2("tabular_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", saved_tabular_surfacebehavior_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/tied_surfacebehavior_data") && !file.read_dataset_string_rank_2("tied_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", saved_tied_surfacebehavior_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/gap_conductance_data") && !file.read_dataset_string_rank_2("gap_conductance_data", "Cubit-CalculiX/SurfaceInteractions", saved_gap_conductance_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/gap_heat_generation_data") && !file.read_dataset_string_rank_2("gap_heat_generation_data", "Cubit-CalculiX/SurfaceInteractions", saved_gap_heat_generation_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/SurfaceInteractions/friction_data") && !file.read_dataset_string_rank_2("friction_data", "Cubit-CalculiX/SurfaceInteractions", saved_friction_data))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_surfaceinteractions_data, 7)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: surfaceinteractions_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_surfaceinteraction_name_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: surfaceinteraction_name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_exponential_surfacebehavior_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: exponential_surfacebehavior_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_linear_surfacebehavior_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: linear_surfacebehavior_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_tabular_surfacebehavior_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: tabular_surfacebehavior_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_tied_surfacebehavior_data, 2)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: tied_surfacebehavior_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_gap_conductance_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: gap_conductance_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_gap_heat_generation_data, 4)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: gap_heat_generation_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_friction_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: friction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_surfaceinteractions_data, 1, saved_surfaceinteraction_name_data)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: invalid surfaceinteraction_name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_surfaceinteractions_data, 5, saved_gap_heat_generation_data)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: invalid gap_heat_generation_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_surfaceinteractions_data, 6, saved_friction_data)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: invalid friction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_surfaceinteractions_data, 2, 1, 3, saved_exponential_surfacebehavior_data)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: invalid exponential_surfacebehavior_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_surfaceinteractions_data, 2, 2, 3, saved_linear_surfacebehavior_data)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: invalid linear_surfacebehavior_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(saved_surfaceinteractions_data, 2, 4, 3, saved_tied_surfacebehavior_data)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: invalid tied_surfacebehavior_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_surfaceinteractions_data)))
    {
      PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: invalid or duplicate IDs.\n");
      return false;
    }
    interactions.surfaceinteractions_data.swap(saved_surfaceinteractions_data);
    interactions.surfaceinteraction_name_data.swap(saved_surfaceinteraction_name_data);
    interactions.exponential_surfacebehavior_data.swap(saved_exponential_surfacebehavior_data);
    interactions.linear_surfacebehavior_data.swap(saved_linear_surfacebehavior_data);
    interactions.tabular_surfacebehavior_data.swap(saved_tabular_surfacebehavior_data);
    interactions.tied_surfacebehavior_data.swap(saved_tied_surfacebehavior_data);
    interactions.gap_conductance_data.swap(saved_gap_conductance_data);
    interactions.gap_heat_generation_data.swap(saved_gap_heat_generation_data);
    interactions.friction_data.swap(saved_friction_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX SurfaceInteractions: %s\n", error.what());
    return false;
  }
}

bool CoreInteractionsPersistence::write(HDF5Tool& file, const CoreSurfaceInteractions& interactions)
{
  try
  {

    if (!(PersistenceTable::keyedRows(interactions.surfaceinteractions_data, 7)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: surfaceinteractions_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(interactions.surfaceinteraction_name_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: surfaceinteraction_name_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(interactions.exponential_surfacebehavior_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: exponential_surfacebehavior_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(interactions.linear_surfacebehavior_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: linear_surfacebehavior_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(interactions.tabular_surfacebehavior_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: tabular_surfacebehavior_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(interactions.tied_surfacebehavior_data, 2)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: tied_surfacebehavior_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(interactions.gap_conductance_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: gap_conductance_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(interactions.gap_heat_generation_data, 4)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: gap_heat_generation_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(interactions.friction_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: friction_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(interactions.surfaceinteractions_data, 1, interactions.surfaceinteraction_name_data)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: invalid surfaceinteraction_name_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(interactions.surfaceinteractions_data, 5, interactions.gap_heat_generation_data)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: invalid gap_heat_generation_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::references(interactions.surfaceinteractions_data, 6, interactions.friction_data)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: invalid friction_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(interactions.surfaceinteractions_data, 2, 1, 3, interactions.exponential_surfacebehavior_data)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: invalid exponential_surfacebehavior_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(interactions.surfaceinteractions_data, 2, 2, 3, interactions.linear_surfacebehavior_data)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: invalid linear_surfacebehavior_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::selectedReferences(interactions.surfaceinteractions_data, 2, 4, 3, interactions.tied_surfacebehavior_data)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: invalid tied_surfacebehavior_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(interactions.surfaceinteractions_data)))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/SurfaceInteractions"))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("surfaceinteractions_data", "Cubit-CalculiX/SurfaceInteractions", interactions.surfaceinteractions_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("surfaceinteraction_name_data", "Cubit-CalculiX/SurfaceInteractions", interactions.surfaceinteraction_name_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("exponential_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.exponential_surfacebehavior_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("linear_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.linear_surfacebehavior_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("tabular_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.tabular_surfacebehavior_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("tied_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.tied_surfacebehavior_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("gap_conductance_data", "Cubit-CalculiX/SurfaceInteractions", interactions.gap_conductance_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("gap_heat_generation_data", "Cubit-CalculiX/SurfaceInteractions", interactions.gap_heat_generation_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("friction_data", "Cubit-CalculiX/SurfaceInteractions", interactions.friction_data))
    {
      PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX SurfaceInteractions: %s\n", error.what());
    return false;
  }
}

bool CoreInteractionsPersistence::read(HDF5Tool& file, CoreContactPairs& contacts)
{
  try
  {

    decltype(contacts.contactpairs_data) saved_contactpairs_data;
    decltype(contacts.adjust_contactpair_data) saved_adjust_contactpair_data;
    if (file.nameExists("Cubit-CalculiX/ContactPairs/contactpairs_data") && !file.read_dataset_int_rank_2("contactpairs_data", "Cubit-CalculiX/ContactPairs", saved_contactpairs_data))
    {
      PRINT_ERROR("Cannot read CalculiX ContactPairs: %s\n", file.last_error().c_str());
      return false;
    }
    if (file.nameExists("Cubit-CalculiX/ContactPairs/adjust_contactpair_data") && !file.read_dataset_string_rank_2("adjust_contactpair_data", "Cubit-CalculiX/ContactPairs", saved_adjust_contactpair_data))
    {
      PRINT_ERROR("Cannot read CalculiX ContactPairs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_contactpairs_data, 6)))
    {
      PRINT_ERROR("Cannot read CalculiX ContactPairs: contactpairs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(saved_adjust_contactpair_data, 3)))
    {
      PRINT_ERROR("Cannot read CalculiX ContactPairs: adjust_contactpair_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(saved_contactpairs_data, 5, saved_adjust_contactpair_data)))
    {
      PRINT_ERROR("Cannot read CalculiX ContactPairs: invalid adjust_contactpair_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(saved_contactpairs_data)))
    {
      PRINT_ERROR("Cannot read CalculiX ContactPairs: invalid or duplicate IDs.\n");
      return false;
    }
    contacts.contactpairs_data.swap(saved_contactpairs_data);
    contacts.adjust_contactpair_data.swap(saved_adjust_contactpair_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX ContactPairs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX ContactPairs: %s\n", error.what());
    return false;
  }
}

bool CoreInteractionsPersistence::write(HDF5Tool& file, const CoreContactPairs& contacts)
{
  try
  {

    if (!(PersistenceTable::keyedRows(contacts.contactpairs_data, 6)))
    {
      PRINT_ERROR("Cannot write CalculiX ContactPairs: contactpairs_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::keyedRows(contacts.adjust_contactpair_data, 3)))
    {
      PRINT_ERROR("Cannot write CalculiX ContactPairs: adjust_contactpair_data has incomplete rows.\n");
      return false;
    }
    if (!(PersistenceTable::references(contacts.contactpairs_data, 5, contacts.adjust_contactpair_data)))
    {
      PRINT_ERROR("Cannot write CalculiX ContactPairs: invalid adjust_contactpair_data reference.\n");
      return false;
    }
    if (!(PersistenceTable::uniqueIds(contacts.contactpairs_data)))
    {
      PRINT_ERROR("Cannot write CalculiX ContactPairs: invalid or duplicate IDs.\n");
      return false;
    }
    if (!file.ensureGroup("Cubit-CalculiX/ContactPairs"))
    {
      PRINT_ERROR("Cannot write CalculiX ContactPairs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_int_rank_2("contactpairs_data", "Cubit-CalculiX/ContactPairs", contacts.contactpairs_data))
    {
      PRINT_ERROR("Cannot write CalculiX ContactPairs: %s\n", file.last_error().c_str());
      return false;
    }
    if (!file.write_dataset_string_rank_2("adjust_contactpair_data", "Cubit-CalculiX/ContactPairs", contacts.adjust_contactpair_data))
    {
      PRINT_ERROR("Cannot write CalculiX ContactPairs: %s\n", file.last_error().c_str());
      return false;
    }
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX ContactPairs: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX ContactPairs: %s\n", error.what());
    return false;
  }
}
