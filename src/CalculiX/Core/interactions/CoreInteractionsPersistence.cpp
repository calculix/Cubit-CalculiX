#include "Core/interactions/CoreInteractionsPersistence.hpp"
#include "Core/interactions/CoreConstraints.hpp"
#include "Core/interactions/CoreSurfaceInteractions.hpp"
#include "Core/interactions/CoreContactPairs.hpp"
#include "HDF5Tool.hpp"

void CoreInteractionsPersistence::read(HDF5Tool& file, CoreConstraints& constraints)
{
  file.read_dataset_int_rank_2("constraints_data", "Cubit-CalculiX/Constraints", constraints.constraints_data);
  file.read_dataset_string_rank_2("rigidbody_constraint_data", "Cubit-CalculiX/Constraints", constraints.rigidbody_constraint_data);
  file.read_dataset_string_rank_2("tie_constraint_data", "Cubit-CalculiX/Constraints", constraints.tie_constraint_data);
  file.read_dataset_string_rank_2("equation_constraint_data", "Cubit-CalculiX/Constraints", constraints.equation_constraint_data);
  file.read_dataset_double_rank_2("equation_data", "Cubit-CalculiX/Constraints", constraints.equation_data);
  file.read_dataset_string_rank_2("equation_group_constraint_data", "Cubit-CalculiX/Constraints", constraints.equation_group_constraint_data);
  file.read_dataset_double_rank_2("equation_group_data", "Cubit-CalculiX/Constraints", constraints.equation_group_data);
}

void CoreInteractionsPersistence::write(HDF5Tool& file, const CoreConstraints& constraints)
{
  file.createGroup("Cubit-CalculiX/Constraints");
  file.write_dataset_int_rank_2("constraints_data", "Cubit-CalculiX/Constraints", constraints.constraints_data);
  file.write_dataset_string_rank_2("rigidbody_constraint_data", "Cubit-CalculiX/Constraints", constraints.rigidbody_constraint_data);
  file.write_dataset_string_rank_2("tie_constraint_data", "Cubit-CalculiX/Constraints", constraints.tie_constraint_data);
  file.write_dataset_string_rank_2("equation_constraint_data", "Cubit-CalculiX/Constraints", constraints.equation_constraint_data);
  file.write_dataset_double_rank_2("equation_data", "Cubit-CalculiX/Constraints", constraints.equation_data);
  file.write_dataset_string_rank_2("equation_group_constraint_data", "Cubit-CalculiX/Constraints", constraints.equation_group_constraint_data);
  file.write_dataset_double_rank_2("equation_group_data", "Cubit-CalculiX/Constraints", constraints.equation_group_data);
}

void CoreInteractionsPersistence::read(HDF5Tool& file, CoreSurfaceInteractions& interactions)
{
  file.read_dataset_int_rank_2("surfaceinteractions_data", "Cubit-CalculiX/SurfaceInteractions", interactions.surfaceinteractions_data);
  file.read_dataset_string_rank_2("surfaceinteraction_name_data", "Cubit-CalculiX/SurfaceInteractions", interactions.surfaceinteraction_name_data);
  file.read_dataset_string_rank_2("exponential_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.exponential_surfacebehavior_data);
  file.read_dataset_string_rank_2("linear_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.linear_surfacebehavior_data);
  file.read_dataset_string_rank_2("tabular_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.tabular_surfacebehavior_data);
  file.read_dataset_string_rank_2("tied_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.tied_surfacebehavior_data);
  file.read_dataset_string_rank_2("gap_conductance_data", "Cubit-CalculiX/SurfaceInteractions", interactions.gap_conductance_data);
  file.read_dataset_string_rank_2("gap_heat_generation_data", "Cubit-CalculiX/SurfaceInteractions", interactions.gap_heat_generation_data);
  file.read_dataset_string_rank_2("friction_data", "Cubit-CalculiX/SurfaceInteractions", interactions.friction_data);
}

void CoreInteractionsPersistence::write(HDF5Tool& file, const CoreSurfaceInteractions& interactions)
{
  file.createGroup("Cubit-CalculiX/SurfaceInteractions");
  file.write_dataset_int_rank_2("surfaceinteractions_data", "Cubit-CalculiX/SurfaceInteractions", interactions.surfaceinteractions_data);
  file.write_dataset_string_rank_2("surfaceinteraction_name_data", "Cubit-CalculiX/SurfaceInteractions", interactions.surfaceinteraction_name_data);
  file.write_dataset_string_rank_2("exponential_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.exponential_surfacebehavior_data);
  file.write_dataset_string_rank_2("linear_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.linear_surfacebehavior_data);
  file.write_dataset_string_rank_2("tabular_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.tabular_surfacebehavior_data);
  file.write_dataset_string_rank_2("tied_surfacebehavior_data", "Cubit-CalculiX/SurfaceInteractions", interactions.tied_surfacebehavior_data);
  file.write_dataset_string_rank_2("gap_conductance_data", "Cubit-CalculiX/SurfaceInteractions", interactions.gap_conductance_data);
  file.write_dataset_string_rank_2("gap_heat_generation_data", "Cubit-CalculiX/SurfaceInteractions", interactions.gap_heat_generation_data);
  file.write_dataset_string_rank_2("friction_data", "Cubit-CalculiX/SurfaceInteractions", interactions.friction_data);
}

void CoreInteractionsPersistence::read(HDF5Tool& file, CoreContactPairs& contacts)
{
  file.read_dataset_int_rank_2("contactpairs_data", "Cubit-CalculiX/ContactPairs", contacts.contactpairs_data);
  file.read_dataset_string_rank_2("adjust_contactpair_data", "Cubit-CalculiX/ContactPairs", contacts.adjust_contactpair_data);
}

void CoreInteractionsPersistence::write(HDF5Tool& file, const CoreContactPairs& contacts)
{
  file.createGroup("Cubit-CalculiX/ContactPairs");
  file.write_dataset_int_rank_2("contactpairs_data", "Cubit-CalculiX/ContactPairs", contacts.contactpairs_data);
  file.write_dataset_string_rank_2("adjust_contactpair_data", "Cubit-CalculiX/ContactPairs", contacts.adjust_contactpair_data);
}
