#include "Core/topology/CoreTopologyPersistence.hpp"
#include "Core/topology/CoreTopology.hpp"
#include "Core/topology/CoreTopologyJobs.hpp"
#include "Core/topology/CoreTopologyQuery.hpp"
#include "Core/materials/CoreMaterials.hpp"
#include "Core/sections/CoreSections.hpp"
#include "Core/jobs/CoreJobs.hpp"
#include "TopologyHdf5.hpp"
#include "TopologyIterationIds.hpp"
#include "CubitMessage.hpp"

#include <string>
#include <vector>

bool CoreTopologyPersistence::read_derived_data(HDF5Tool& file, const CoreTopology& topology, CoreMaterials& materials, CoreSections& sections, CoreJobs& jobs)
{
  try
  {
    decltype(materials.topology_material_data) saved_materials;
    decltype(sections.topology_solid_sections) saved_sections;
    const std::string root = "Cubit-CalculiX/Topology";
    std::vector<int> version;

    if (file.nameExists(root))
    {
      if (!file.read_dataset_int_rank_1("version", root, version) || version.size() != 1)
      {
        PRINT_ERROR("Unable to read topology save version.\n");
        return false;
      }
    }

    if (!version.empty() && version[0] >= 2)
    {
      TopologyHdf5<true> archive(file, "Cubit-CalculiX");

      if (!archive.value("TopologyMaterials", saved_materials) || !archive.value("TopologySections", saved_sections))
      {
        PRINT_ERROR("Unable to read topology material/section data.\n");
        return false;
      }
    }

    if (!version.empty() && version[0] == 2)
    {
      for (auto& collection : saved_materials)
      {
        if (!increment_topology_iteration_keys(collection.second.iteration_factors) || !increment_topology_iteration_keys(collection.second.iteration_material_caps))
        {
          PRINT_ERROR("Unable to migrate topology material factors.\n");
          return false;
        }
      }
    }

    for (const auto& collection : saved_materials)
    {
      if (!valid_topology_iteration_keys(collection.second.iteration_factors) || !valid_topology_iteration_keys(collection.second.iteration_material_caps))
      {
        PRINT_ERROR("Invalid saved topology material iteration ID.\n");
        return false;
      }
    }

    CoreTopologyJobs::invalidate_saved_processes(topology, jobs);

    materials.topology_material_data.swap(saved_materials);
    sections.topology_solid_sections.swap(saved_sections);

    // Dependencies and job results are loaded before checking selections.
    return CoreTopologyQuery::validate_selected_iterations(topology, materials, jobs);
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Unable to read topology material/section data: %s\n", error.getDetailMsg().c_str());
    return false;
  }
}

bool CoreTopologyPersistence::write_derived_data(HDF5Tool& file, CoreMaterials& materials, CoreSections& sections)
{
  try
  {
    TopologyHdf5<false> archive(file, "Cubit-CalculiX");

    if (!archive.value("TopologyMaterials", materials.topology_material_data) || !archive.value("TopologySections", sections.topology_solid_sections))
    {
      PRINT_ERROR("Unable to save topology material/section data.\n");
      return false;
    }

    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Unable to save topology material/section data: %s\n", error.getDetailMsg().c_str());
    return false;
  }
}

bool CoreTopologyPersistence::read(HDF5Tool& file, CoreTopology& topology, CoreMaterials& materials, CoreSections& sections, CoreJobs& jobs)
{
  if (!topology.read_configuration(file))
  {
    PRINT_ERROR("Unable to read topology configurations.\n");
    return false;
  }

  return read_derived_data(file, topology, materials, sections, jobs);
}

bool CoreTopologyPersistence::write(HDF5Tool& file, const CoreTopology& topology, CoreMaterials& materials, CoreSections& sections)
{
  if (!topology.save_configuration(file))
  {
    PRINT_ERROR("Unable to save topology configurations.\n");
    return false;
  }

  return write_derived_data(file, materials, sections);
}
