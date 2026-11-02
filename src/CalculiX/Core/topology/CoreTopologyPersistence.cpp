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
#include <exception>
#include <algorithm>
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

    for (const auto& collection : saved_materials)
    {
      if (topology.topology_cases.count(collection.first) == 0)
      {
        PRINT_ERROR("Cannot read CalculiX topology materials: missing topology case.\n");
        return false;
      }
    }
    for (const auto& assignment : saved_sections)
    {
      if (topology.topology_cases.count(assignment.first) == 0)
      {
        PRINT_ERROR("Cannot read CalculiX topology sections: missing topology case.\n");
        return false;
      }
      for (const int id : assignment.second)
      {
        if (std::none_of(sections.sections_data.begin(), sections.sections_data.end(), [id](const auto& row) { return row.size() >= 3 && row[0] == id && row[1] == 1; }))
        {
          PRINT_ERROR("Cannot read CalculiX topology sections: missing solid section %d.\n", id);
          return false;
        }
      }
    }
    // Data-only validation views: no material interfaces or process handles are copied.
    CoreMaterials saved_material_view;
    saved_material_view.topology_material_data.swap(saved_materials);
    CoreJobs saved_job_view;
    saved_job_view.jobs_data = jobs.jobs_data;
    CoreTopologyJobs::invalidate_saved_processes(topology, saved_job_view);
    if (!CoreTopologyQuery::validate_selected_iterations(topology, saved_material_view, saved_job_view))
    {
      return false;
    }
    materials.topology_material_data.swap(saved_material_view.topology_material_data);
    sections.topology_solid_sections.swap(saved_sections);
    jobs.jobs_data.swap(saved_job_view.jobs_data);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Unable to read topology material/section data: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX topology derived data: %s\n", error.what());
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
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX topology derived data: %s\n", error.what());
    return false;
  }
}

bool CoreTopologyPersistence::read(HDF5Tool& file, CoreTopology& topology, CoreMaterials& materials, CoreSections& sections, CoreJobs& jobs)
{
  try
  {
    CoreTopology saved;
    if (!saved.read_configuration(file) || !read_derived_data(file, saved, materials, sections, jobs))
    {
      return false;
    }
    topology.topology_cases.swap(saved.topology_cases);
    return true;
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX topology: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot read CalculiX topology: %s\n", error.what());
    return false;
  }
}

bool CoreTopologyPersistence::write(HDF5Tool& file, const CoreTopology& topology, CoreMaterials& materials, CoreSections& sections)
{
  try
  {
    if (!topology.save_configuration(file))
    {
      return false;
    }
    return write_derived_data(file, materials, sections);
  }
  catch (const H5::Exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX topology: %s\n", error.getDetailMsg().c_str());
    return false;
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Cannot write CalculiX topology: %s\n", error.what());
    return false;
  }
}
