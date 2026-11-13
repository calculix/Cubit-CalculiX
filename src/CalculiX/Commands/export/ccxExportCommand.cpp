#include "ccxExportCommand.hpp"
#include <algorithm>
#include <string>
#include <vector>
#include "Core/amplitudes/CalculiXAmplitudesInterface.hpp"
#include "Core/application/CalculiXApplicationInterface.hpp"
#include "Core/bcs/CalculiXBCsInterface.hpp"
#include "Core/export/CalculiXExportInterface.hpp"
#include "Core/interactions/CalculiXInteractionsInterface.hpp"
#include "Core/materials/CalculiXMaterialsInterface.hpp"
#include "Core/mesh/CalculiXMeshInterface.hpp"
#include "Core/sections/CalculiXSectionsInterface.hpp"
#include "Core/steps/CalculiXStepsInterface.hpp"
#include "Core/topology/CalculiXTopologyInterface.hpp"
#include "ElementNodeOrdering.hpp"
#include "MeshExportInterface.hpp"
#include "MaterialInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"
#include "AppUtil.hpp"
#include <fstream>
#include <exception>
#include <QScopeGuard>

#ifdef WIN32
 //#include <windows.h>
 #include <io.h>
#else
 #include <unistd.h>
#endif

ccxExportCommand::ccxExportCommand()
{
  progressbar = CubitInterface::app_util().get()->progress_tool();;
}

ccxExportCommand::~ccxExportCommand()
{
  // AppUtil owns the progress tool.
}

std::vector<std::string> ccxExportCommand::get_syntax()
{
  // Define the syntax for the command. Note the syntax is a modified BNF
  // format. Full documentation on the command specification syntax can be
  // found in the documentation.
  std::string syntax =
    "export ccx <string:label='filename',help='<filename>'> "
    "[topology <value:label='topology_id'> "
    "iteration <value:label='iteration_id'>] "
    "[overwrite]";

  std::vector<std::string> syntax_list;
  syntax_list.push_back(syntax);

  return syntax_list;
}

std::vector<std::string> ccxExportCommand::get_syntax_help()
{
  std::vector<std::string> help;
  return help;
}

std::vector<std::string> ccxExportCommand::get_help()
{
  std::vector<std::string> help;
  return help;
}

bool ccxExportCommand::execute(CubitCommandData &data)
{
  CalculiXApplicationInterface ccx_application_iface;
  CalculiXExportInterface ccx_export_iface;
  ccx_export_iface.trace_export("export.execute.enter");

  std::ofstream output_file;
  MeshExportInterface *iface;
  MaterialInterface *material_iface;

  // The command syntax specified that we should have a string labeled "filename" in the
  // the command. We would not have gotten to this point without having that required
  // piece of data. Retrieve it here
  std::string filename;
  int topology_id = -1;
  int iteration_id = -1;
  data.get_string("filename", filename);

  // The syntax also indicated that we could have an optional piece of data asking if we
  // wanted to overwrite an existing file. This will return true if the overwrite option
  // was specified in the command

  bool overwrite = data.find_keyword("overwrite");

  if (data.find_keyword("topology") && data.find_keyword("iteration"))
  {
    data.get_value("topology_id", topology_id);
    data.get_value("iteration_id", iteration_id);
  }

  // check if file already exists
  if (!overwrite)
  {
    #ifdef WIN32
      if (_access(filename.c_str(), 0) == 0)
      {
        PRINT_ERROR("Output File already exists!\n");
        return false;
      }
    #else
      if (access(filename.c_str(), F_OK) == 0)
      {
        PRINT_ERROR("Output File already exists!\n");
        return false;
      }
    #endif
  }

  // Get the MeshExport interface from CubitInterface
  iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
  if(!iface)
  {
    PRINT_ERROR("Unable to get mesh export interface.\n");
    return false;
  }

  const auto mesh_guard = qScopeGuard([&]()
  {
    ccx_export_iface.trace_export("export.mesh_release.enter");
    CubitInterface::release_interface(iface);
    ccx_export_iface.trace_export("export.mesh_release.leave");
  });

  // Get the Material Interface from CubitInterface
  material_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));
  if(!material_iface)
  {
    PRINT_ERROR("Unable to get material interface.\n");
    return false;
  }


  const auto material_guard = qScopeGuard([&]()
  {
    ccx_export_iface.trace_export("export.material_release.enter");
    CubitInterface::release_interface(material_iface);
    ccx_export_iface.trace_export("export.material_release.leave");
  });

  // Now we can use the interface as before
  iface->set_use_sequential_ids(false);

  const bool previous_gui_block = ccx_application_iface.block_gui_update();
  ccx_application_iface.set_block_gui_update(true);
  const auto gui_guard = qScopeGuard([&]()
  {
    ccx_export_iface.trace_export("export.gui_restore.enter");
    ccx_application_iface.set_block_gui_update(previous_gui_block);
    ccx_export_iface.trace_export("export.gui_restore.leave");
  });
  try
  {
    ccx_export_iface.trace_export("export.file_open.enter");
    const bool opened = open_file(filename, output_file);
    ccx_export_iface.trace_export("export.file_open.leave");
    bool result = false;
    if (opened)
    {
      ccx_export_iface.trace_export("export.write_file.enter");
      result = write_file(output_file, iface, material_iface, topology_id, iteration_id);
      ccx_export_iface.trace_export("export.write_file.leave");
    }
    ccx_export_iface.trace_export("export.file_close.enter");
    close_file(output_file);
    ccx_export_iface.trace_export("export.file_close.leave");
    ccx_export_iface.trace_export(result ? "export.execute.success" : "export.execute.failure");
    return result;
  }
  catch (const std::exception& error)
  {
    ccx_export_iface.trace_export("export.execute.exception");
    PRINT_ERROR("CCX input export '%s' failed: %s\n", filename.c_str(), error.what());
  }
  catch (...)
  {
    ccx_export_iface.trace_export("export.execute.unexpected_exception");
    PRINT_ERROR("CCX input export '%s' failed with an unexpected exception.\n", filename.c_str());
  }
  return false;
}

bool ccxExportCommand::open_file(const std::string& file, std::ofstream& output_file)
{
  output_file.open(file.c_str());

  return true;
}

bool ccxExportCommand::write_file(std::ofstream& output_file, MeshExportInterface *iface, MaterialInterface *material_iface, int topology_id, int iteration_id)
{
  CalculiXExportInterface ccx_export_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  bool result;
  if (!output_file.good())
  {
    PRINT_ERROR("Unable to write the output file.\n");
    return false;
  }

  const bool normal_export = topology_id == -1 && iteration_id == -1;

  if (!normal_export && (topology_id <= 0 || iteration_id < 1))
  {
    PRINT_ERROR("Invalid topology export selection.\n");
    return false;
  }

  if (!ccx_topology_iface.check_topology_export(topology_id, iteration_id))
  {
    return false;
  }

  // check if everything is meshed. if no. abort!
  std::vector<std::string> entity_type_list;
  entity_type_list.push_back("vertex");
  entity_type_list.push_back("curve");
  entity_type_list.push_back("surface");
  entity_type_list.push_back("volume");
  std::vector<int> entity_id_list;
  for (size_t i = 0; i < entity_type_list.size(); i++)
  {
    entity_id_list = CubitInterface::get_entities(entity_type_list[i]);
    for (size_t ii = 0; ii < entity_id_list.size(); ii++)
    {
      if (!(CubitInterface::is_meshed(entity_type_list[i],entity_id_list[ii])))
      {
        if (entity_type_list[i]=="volume")
        {
          if (!(CubitInterface::is_sheet_body(entity_id_list[ii])))
          {
            PRINT_WARNING("\nEvery Entity has to be meshed before export!\n");
            return false;
          }
        }else{
          PRINT_WARNING("\nEvery Entity has to be meshed before export!\n");
          return false;
        }
      }
    }
    entity_id_list.clear();
  }

  if (progressbar == nullptr)
  {
    PRINT_ERROR("The CCX export progress tool is unavailable.\n");
    return false;
  }
  const char* stage = "opening export data";
  bool cleanup_pending = false;
  bool progress_started = false;
  const auto cleanup_guard = qScopeGuard([&]()
  {
    if (cleanup_pending)
    {
      try
      {
        ccx_export_iface.trace_export("export.interrupted_cleanup.enter");
        if (!ccx_export_iface.clean_export())
        {
          PRINT_ERROR("Failed to clean up interrupted CCX export data.\n");
        }
        ccx_export_iface.trace_export("export.interrupted_cleanup.leave");
      }
      catch (...)
      {
        ccx_export_iface.trace_export("export.interrupted_cleanup.exception");
        PRINT_ERROR("Exception while cleaning up interrupted CCX export data.\n");
      }
    }
    if (progress_started)
    {
      try
      {
        ccx_export_iface.trace_export("export.interrupted_progress_end.enter");
        progressbar->end();
        ccx_export_iface.trace_export("export.interrupted_progress_end.leave");
      }
      catch (...)
      {
        ccx_export_iface.trace_export("export.interrupted_progress_end.exception");
        PRINT_ERROR("Unable to end the CCX export progress display.\n");
      }
    }
  });
  try
  {
    // first blood ... no i mean first line
    std::vector<std::string> customline = ccx_export_iface.get_customline_data("BEGIN","EXPORT",-1);
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      output_file << customline[icl] << "\n";
    }

    //output_file << "*HEADING \n";
    //output_file << "Cubit2CalculiX with CalculiX Plugin \n";
    //output_file << "** \n";

    progress_started = true;
    progressbar->start(0,18,"Writing CCX .inp File");

    // prepare export data
    // trajectory: sidesets,amplitude,dflux
    stage = "preparing export data";
    cleanup_pending = true;
    result = ccx_export_iface.prepare_export();
    if (!result)
    {
      stage = "cleaning export data";
      cleanup_pending = false;
      const bool cleanup_ok = ccx_export_iface.clean_export();
      progress_started = false;
      progressbar->end();
      PRINT_ERROR("Export stopped: trajectory preparation failed.\n");
      if (!cleanup_ok)
      {
        PRINT_ERROR("Failed to clean up prepared export data.\n");
      }
      return false;
    }
    progressbar->step();

    // Initialize the exporter
    stage = "initializing the mesh exporter";
    if (!iface->initialize_export())
    {
      PRINT_ERROR("Unable to initialize the CCX mesh exporter.\n");
      return false;
    }

    // Write the nodes
    stage = "write_nodes";
    result = result && write_nodes(output_file, iface);
    progressbar->step();

    // Write the elements/connectivity
    stage = "write_connectivity";
    result = result && write_connectivity(output_file, iface);
    progressbar->step();

    // Write the blocks
    //result = write_blocks(output_file, iface);

    // Write the nodesets
    stage = "write_nodesets";
    result = result && write_nodesets(output_file, iface);
    progressbar->step();

    // Write the sidesets
    stage = "write_sidesets";
    result = result && write_sidesets(output_file, iface);
    progressbar->step();

    // Write the materials and sections
    stage = "write_materials";
    result = result && write_materials(output_file, topology_id, iteration_id);
    progressbar->step();
    stage = "write_orientations";
    result = result && write_orientations(output_file);
    progressbar->step();
    stage = "write_sections";
    result = result && write_sections(output_file, topology_id, iteration_id);
    progressbar->step();

    stage = "write_damping";
    result = result && write_damping(output_file);
    progressbar->step();
    stage = "write_physicalconstants";
    result = result && write_physicalconstants(output_file);
    progressbar->step();
    stage = "write_constraints";
    result = result && write_constraints(output_file);
    progressbar->step();
    stage = "write_surfaceinteractions";
    result = result && write_surfaceinteractions(output_file);
    progressbar->step();
    stage = "write_contactpairs";
    result = result && write_contactpairs(output_file);
    progressbar->step();
    stage = "write_amplitudes";
    result = result && write_amplitudes(output_file);
    progressbar->step();
    stage = "write_initialconditions";
    result = result && write_initialconditions(output_file);
    progressbar->step();
    stage = "write_hbcs";
    result = result && write_hbcs(output_file);
    progressbar->step();
    stage = "write_steps";
    result = result && write_steps(output_file);
    progressbar->step();

    //clean export data
    if (result && output_file.good())
    {
      output_file.flush();
      if (output_file.good())
      {
        ccx_export_iface.freeze_job_setup();
      }
    }
    stage = "cleaning export data";
    cleanup_pending = false;
    ccx_export_iface.trace_export("export.cleanup.enter");
    const bool cleanup_ok = ccx_export_iface.clean_export();
    ccx_export_iface.trace_export("export.cleanup.leave");
    if (!cleanup_ok)
    {
      PRINT_ERROR("Failed to clean up prepared export data.\n");
    }
    ccx_export_iface.trace_export("export.final_progress_step.enter");
    progressbar->step();
    ccx_export_iface.trace_export("export.final_progress_step.leave");

    stage = "ending export progress";
    progress_started = false;
    ccx_export_iface.trace_export("export.progress_end.enter");
    progressbar->end();
    ccx_export_iface.trace_export("export.progress_end.leave");

    if (result && cleanup_ok && output_file.good())
    {
      stage = "writing export end custom lines";
      ccx_export_iface.trace_export("export.end_custom_lines.enter");
      customline = ccx_export_iface.get_customline_data("END","EXPORT",-1);
      for (size_t icl = 0; icl < customline.size(); icl++)
      {
        output_file << customline[icl] << "\n";
      }
      ccx_export_iface.trace_export("export.end_custom_lines.leave");
    }

    return result && cleanup_ok && output_file.good();
  }
  catch (const std::exception& error)
  {
    ccx_export_iface.trace_export("export.write_file.exception");
    PRINT_ERROR("CCX input export failed during %s: %s\n", stage, error.what());
  }
  catch (...)
  {
    ccx_export_iface.trace_export("export.write_file.unexpected_exception");
    PRINT_ERROR("CCX input export failed during %s with an unexpected exception.\n", stage);
  }
  return false;
}

bool ccxExportCommand::close_file(std::ofstream& output_file)
{
  output_file.close();
  return true;
}

bool ccxExportCommand::write_nodes(std::ofstream& output_file,MeshExportInterface *iface)
{
  int max_node_id = -1;
  // Fetch and output the coordinates
  int number_nodes = iface->get_num_nodes();

  if(!number_nodes)
  {
    PRINT_WARNING("No nodes in model...\n");
    return false;
  }

  //output_file << "Number of Nodes:  " << number_nodes << "\n";
  //output_file << "List of Nodes:\n";

  // to get a beautiful order of the nodes for the output we have to first get all nodes and sort them.

  int buf_size = 100;
  std::vector<double> xcoords_tmp(buf_size), ycoords_tmp(buf_size), zcoords_tmp(buf_size);
  std::vector<int> node_ids_tmp(buf_size);

  std::vector<double> xcoords(number_nodes), ycoords(number_nodes), zcoords(number_nodes);
  std::vector<int> node_ids(number_nodes);

  // Declaring vector of pairs for sorting
  std::vector< std::pair <int,int> > vect_sort;

  int start = 0, number_found = 0;
  while ((number_found = iface->get_coords(start, buf_size, xcoords_tmp, ycoords_tmp, zcoords_tmp, node_ids_tmp)))
  {
    // Write out the coordinates
    for(int i = 0; i < number_found; i++)
    {
      if (max_node_id<node_ids_tmp[i])
      {
        max_node_id = node_ids_tmp[i];
      }
      node_ids[start+i] = node_ids_tmp[i];
      xcoords[start+i] = xcoords_tmp[i];
      ycoords[start+i] = ycoords_tmp[i];
      zcoords[start+i] = zcoords_tmp[i];
      // Entering values in vector of pairs for sorting
      vect_sort.push_back( std::make_pair(node_ids_tmp[i],start+i));
    }
    start += number_found;
  }

  // sorting of the vectors
  // Using simple sort() function to sort vect_sort, first column will be sorted
  sort(vect_sort.begin(), vect_sort.end());

  // output all the ordered nodes
  output_file << "********************************** N O D E S ********************************** \n";
  output_file << "*NODE, NSET=ALLNODES \n";

  // Write out the coordinates
  for(int i = 0; i < number_nodes; i++)
  {
    output_file.precision(6);
    output_file << std::scientific;
    output_file << vect_sort[i].first << ",  " <<
                   xcoords[vect_sort[i].second] << ",  " <<
                   ycoords[vect_sort[i].second] << ",  " <<
                   zcoords[vect_sort[i].second] << "\n";
  }

  output_file << "** \n";

  return true;
}

bool ccxExportCommand::write_connectivity(std::ofstream& output_file,MeshExportInterface *iface)
{
  CalculiXExportInterface ccx_export_iface;
  CalculiXMeshInterface ccx_mesh_iface;

  output_file << "********************************** E L E M E N T S **************************** \n";

  // Get Block id's
  std::vector<int> blocks;
  blocks = ccx_mesh_iface.get_blocks();

  std::string current_element_type = ""; // to detect changes of element type in blocks. especially hex and wedge

  // Get a batch of elements in an initialized block
  int buf_size = 100;
  std::vector<ElementType>   element_type(buf_size);
  std::vector<ElementHandle> handles(buf_size);

  // define name variable
  std::string element_type_name;
  std::string cubit_element_type_entity;
  std::string block_name;

  // Elements in a buffer set will be of the same element type and in the same block
  for (size_t i = 0; i < blocks.size(); i++)
  {
    int num_elems;
    int start_index = 0;
    //BlockHandle block = blocks[i];
    BlockHandle block;
    iface->get_block_handle(blocks[i], block);
    current_element_type = CubitInterface::get_block_element_type(blocks[i]);


    if (current_element_type != "SPHERE") // check if cubit element type from block is sphere
    {
      // write only once per block
      int block_id = blocks[i];
      block_name = ccx_mesh_iface.get_block_name(block_id);
      element_type_name = ccx_mesh_iface.get_ccx_element_type(block_id);

      // CUSTOMLINE START
      std::vector<std::string> customline = ccx_export_iface.get_customline_data("BEFORE","ELSET",block_id);
      for (size_t icl = 0; icl < customline.size(); icl++)
      {
        output_file << customline[icl] << "\n";
      }
      // CUSTOMLINE END

      output_file << "*ELEMENT, TYPE=" << element_type_name << ", ELSET=" << block_name << "\n";

      while( (num_elems = iface->get_block_elements(start_index, buf_size, block, element_type, handles)) > 0)
      {
        // Get ids for the element handles
        std::vector<int> ids(num_elems);
        iface->get_element_ids(num_elems, handles, ids);

        // skip if element type is 0 (SPHERE), that element type doesn´t exist in CalculiX
        // skip if block element type is different from requested elements, hex and wedge mix
        if ((element_type[0] != 0) && (current_element_type==iface->get_element_type_name(element_type[0])))
        {
          // convert ids from element_id to global_element_id
          cubit_element_type_entity = iface->get_element_type_name(element_type[0]);
          cubit_element_type_entity = ccx_mesh_iface.get_cubit_element_type_entity(cubit_element_type_entity);

          //rewrite to global element ids
          for (size_t ii = 0; ii < ids.size(); ii++)
          {
            ids[ii] = CubitInterface::get_global_element_id(cubit_element_type_entity,ids[ii]);
          }

          // Write out the connectivity
          for (int ii = 0; ii < num_elems; ii++)
          {
            std::vector<int> conn(27);
            const int num_nodes = iface->get_connectivity(handles[ii], conn);

            if (num_nodes <= 0 || static_cast<std::size_t>(num_nodes) > conn.size())
            {
              PRINT_ERROR("Invalid connectivity for element %d.\n", ids[ii]);
              return false;
            }

            output_file << ids[ii] << ", ";

            for (int j = 0; j < num_nodes; ++j)
            {
              const std::size_t source = ElementNodeOrdering::cubit_source_index(static_cast<int>(element_type[0]), static_cast<std::size_t>(j));

              if (source >= static_cast<std::size_t>(num_nodes))
              {
                PRINT_ERROR("Invalid node ordering for element %d.\n", ids[ii]);
                return false;
              }

              output_file << conn[source];

              if (j != num_nodes - 1)
              {
                output_file << ", ";
              }

              if (j == 14 && element_type[0] == 44)
              {
                output_file << "\n";
              }
            }

            output_file << std::endl;
          }
        }
        start_index += num_elems;
      }
      // CUSTOMLINE START
      customline = ccx_export_iface.get_customline_data("AFTER","ELSET",block_id);
      for (size_t icl = 0; icl < customline.size(); icl++)
      {
        output_file << customline[icl] << "\n";
      }
      // CUSTOMLINE END
    }
  }

  output_file << "** \n";

  return true;

}

bool ccxExportCommand::write_nodesets(std::ofstream& output_file,MeshExportInterface *iface)
{
  CalculiXExportInterface ccx_export_iface;


  output_file << "********************************** N O D E S E T S ********************************** \n";

  // Get the list of nodesets
  std::vector<NodesetHandle> nodesets;
  iface->get_nodeset_list(nodesets);

  // Get a batch of nodes from the nodeset
  int buf_size = 100;
  std::string nodeset_name;

  // loop over the nodesets
  for (size_t i = 0; i < nodesets.size(); i++)
  {
    NodesetHandle nodeset = nodesets[i];
    nodeset_name = iface->get_nodeset_name(nodeset);
    if (nodeset_name == "") {
      nodeset_name = "Nodeset_" + std::to_string(iface->id_from_handle(nodeset));
    }

    // CUSTOMLINE START
    std::vector<std::string> customline = ccx_export_iface.get_customline_data("BEFORE","NSET",iface->id_from_handle(nodeset));
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      output_file << customline[icl] << "\n";
    }
    // CUSTOMLINE END

    output_file << "*NSET, NSET=" << nodeset_name << " \n";

    int num_nodes;
    int start_index = 0;

    std::vector<int> ids;

    int ic;
    ic=0;

    while( (num_nodes = iface->get_nodeset_node_ids(nodeset,start_index,buf_size,ids)) > 0)
    {
      //counter to know when the next line should begin

      for (int j = 0; j < num_nodes; j++)
      {
        ic += 1;
        if (j!=num_nodes-1){
          output_file << ids[j] << ", ";
        } else {
          output_file << ids[j];
        }
        if (ic == 5) {
          output_file << "\n";
          ic = 0;
        }
      }
      //output_file << "\n---------> " << start_index << "\n";
      start_index += num_nodes;
    }
    output_file << std::endl;

    // CUSTOMLINE START
    customline = ccx_export_iface.get_customline_data("AFTER","NSET",iface->id_from_handle(nodeset));
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      output_file << customline[icl] << "\n";
    }
    // CUSTOMLINE END
  }

  output_file << "** \n";
  return true;
}

bool ccxExportCommand::write_sidesets(std::ofstream& output_file, MeshExportInterface *iface)
{
  CalculiXExportInterface ccx_export_iface;
  CalculiXMeshInterface ccx_mesh_iface;


  output_file << "********************************** S I D E S E T S ********************************** \n";

  // Get the list of sidesets
  std::vector<SidesetHandle> sidesets;
  iface->get_sideset_list(sidesets);

  // Get a batch of elements from the sideset
  int buf_size = 100;

  // variables
  std::string sideset_name;
  // loop over the sidesets
  for (size_t i = 0; i < sidesets.size(); i++)
  {
    SidesetHandle sideset = sidesets[i];

    sideset_name = iface->get_sideset_name(sideset);
    // sidesetname or id
    if (sideset_name == "") {
      sideset_name = "Surface_" + std::to_string(iface->id_from_handle(sideset));
    }

    // CUSTOMLINE START
    std::vector<std::string> customline = ccx_export_iface.get_customline_data("BEFORE","SIDESET",iface->id_from_handle(sideset));
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      output_file << customline[icl] << "\n";
    }
    // CUSTOMLINE END

    int num_elems;
    int start_index = 0;
    int element_count = 0;

    std::vector<ElementHandle> element_ids;
    std::vector<int> side_ids;
    std::vector<ElementType> element_type;
    std::string cubit_element_type_entity;
    std::vector<int>::iterator it;


    // count all elements in sideset
    while( (num_elems = iface->get_sideset_sides(sideset,start_index,buf_size,element_ids,element_type,side_ids)) > 0)
    {
      start_index += num_elems;
      element_count += num_elems;
    }

    int element_id;
    std::vector<int> side_ids_all(element_count);
    std::vector<int> ccx_side_ids_all(element_count);
    std::vector<int> element_ids_all(element_count);
    std::vector<int> element_type_all(element_count);
    std::vector<int> sideset_element_types;

    start_index = 0;
    while( (num_elems = iface->get_sideset_sides(sideset,start_index,buf_size,element_ids,element_type,side_ids)) > 0)
    {
      // get all elements in sideset
      for (int j = 0; j < num_elems; j++)
      {
        iface->get_element_id(element_ids[j],element_id);
        //element_ids_all[start_index + j] = element_id;
        element_type_all[start_index + j] = element_type[j];
        side_ids_all[start_index + j] = side_ids[j];
        ccx_side_ids_all[start_index + j] = ccx_mesh_iface.get_ccx_element_side(element_type[j], side_ids[j]);

        // convert ids from element_id to global_element_id
        cubit_element_type_entity = iface->get_element_type_name(element_type[j]);
        cubit_element_type_entity = ccx_mesh_iface.get_cubit_element_type_entity(cubit_element_type_entity);
        element_ids_all[start_index + j] = CubitInterface::get_global_element_id(cubit_element_type_entity,element_id);

        // check if element type is already recognized for this sideset
        it = std::find(sideset_element_types.begin(), sideset_element_types.end(), element_type[j]);
        if (it == sideset_element_types.end())
        {
          sideset_element_types.push_back(element_type[j]);
        }
      }
    start_index += num_elems;
    }

    //counter to know when the next line should begin
    int ic;
    //std::vector<int>::iterator it;
    bool bool_first;
    bool_first = true;
    bool bool_one_element_type;
    if (sideset_element_types.size()==1)
    {
      bool_one_element_type = true;
    }else{
      bool_one_element_type = false;
    }

    // check how many different element types there are
    // an loop over them
    for (size_t i_types = 0; i_types < sideset_element_types.size(); i_types++)
    {
      // begin loop element_types
      // loop for every possible ccx side
      for (int s = 1; s < 7; s++)
      {
        // begin loop sides
        // Check if side number is in vector
        it = std::find(ccx_side_ids_all.begin(), ccx_side_ids_all.end(), s);
        if ((it != ccx_side_ids_all.end()))
        {
          ic=0;
          for (int j = 0; j < element_count; j++)
          {
            //only write when it´s the right side and the right element type
            if ((s==ccx_side_ids_all[j])&&(sideset_element_types[i_types]==element_type_all[j])) {
              if (bool_first) {
                bool_first=false;
                // add elset to storage, for later use by cards like dload
                if (bool_one_element_type)
                {
                  output_file << "*ELSET, ELSET=" << sideset_name << ("_S" + std::to_string(s)) << " \n";
                  ccx_mesh_iface.add_sideset_face(std::to_string(iface->id_from_handle(sideset)), sideset_name + "_S" + std::to_string(s), std::to_string(s),std::to_string(element_type_all[0]));
                }else{
                  output_file << "*ELSET, ELSET=" << sideset_name << ("_" + std::to_string(sideset_element_types[i_types]) + "_S" + std::to_string(s)) << " \n";
                  ccx_mesh_iface.add_sideset_face(std::to_string(iface->id_from_handle(sideset)), sideset_name + "_" + std::to_string(sideset_element_types[i_types]) + "_S" + std::to_string(s), std::to_string(s),std::to_string(sideset_element_types[i_types]));
                }
              }
              ic += 1;
              //output_file << element_ids_all[j] << ", " << side_ids_all[j] << "\n ";
              output_file << element_ids_all[j] << ", ";
              if (ic == 6) {
                output_file << "\n";
                ic = 0;
              }
            }
          }
          if (ic!=0){
            output_file << std::endl;
          }
        }
        bool_first=true;
        // end loop sides
      }
      // end loop element_types
    }

    std::vector<std::vector<std::string>> temp_list;
    temp_list = ccx_mesh_iface.get_sideset_face(iface->id_from_handle(sideset));

    // write surface card for sideset
    //output_file << "*SURFACE, NAME=";

    for (size_t i = 0; i < temp_list.size(); i++)
    {
      if (bool_first) {
        output_file << "*SURFACE, NAME=" << sideset_name << " \n";
        bool_first=false;
      }
      output_file << temp_list[i][1] << ", " << "S" << temp_list[i][2] << " \n";
    }

    // CUSTOMLINE START
    customline = ccx_export_iface.get_customline_data("AFTER","SIDESET",iface->id_from_handle(sideset));
    for (size_t icl = 0; icl < customline.size(); icl++)
    {
      output_file << customline[icl] << "\n";
    }
    // CUSTOMLINE END
    output_file << std::endl;
  }

  output_file << "** \n";
  return true;
}

bool ccxExportCommand::write_materials(std::ofstream& output_file, int topology_id, int iteration_id)
{
  CalculiXMaterialsInterface ccx_materials_iface;

  const std::string export_data = ccx_materials_iface.get_material_export_data(topology_id, iteration_id);

  if (export_data.empty())
  {
    PRINT_ERROR("Export stopped: material export failed.\n");
    return false;
  }

  output_file << export_data;

  if (!output_file.good())
  {
    PRINT_ERROR("Failed to write materials to the output file.\n");
    return false;
  }

  return true;
}


bool ccxExportCommand::write_sections(std::ofstream& output_file, int topology_id, int iteration_id)
{
  CalculiXSectionsInterface ccx_sections_iface;

  const std::string export_data = ccx_sections_iface.get_section_export_data(topology_id, iteration_id);

  if (export_data.empty())
  {
    PRINT_ERROR("Export stopped: section export failed.\n");
    return false;
  }

  output_file << export_data;

  if (!output_file.good())
  {
    PRINT_ERROR("Failed to write sections to the output file.\n");
    return false;
  }

  return true;
}

bool ccxExportCommand::write_constraints(std::ofstream& output_file)
{
  CalculiXInteractionsInterface ccx_interactions_iface;

  output_file << ccx_interactions_iface.get_constraint_export_data();
  return true;
}

bool ccxExportCommand::write_surfaceinteractions(std::ofstream& output_file)
{
  CalculiXInteractionsInterface ccx_interactions_iface;

  output_file << ccx_interactions_iface.get_surfaceinteraction_export_data();
  return true;
}

bool ccxExportCommand::write_contactpairs(std::ofstream& output_file)
{
  CalculiXInteractionsInterface ccx_interactions_iface;

  output_file << ccx_interactions_iface.get_contactpair_export_data();
  return true;
}

bool ccxExportCommand::write_amplitudes(std::ofstream& output_file)
{
  CalculiXAmplitudesInterface ccx_amplitudes_iface;

  output_file << ccx_amplitudes_iface.get_amplitude_export_data();
  return true;
}

bool ccxExportCommand::write_orientations(std::ofstream& output_file)
{
  CalculiXSectionsInterface ccx_sections_iface;

  output_file << ccx_sections_iface.get_orientation_export_data();
  return true;
}

bool ccxExportCommand::write_damping(std::ofstream& output_file)
{
  CalculiXStepsInterface ccx_steps_iface;

  output_file << ccx_steps_iface.get_damping_export_data();
  return true;
}

bool ccxExportCommand::write_physicalconstants(std::ofstream& output_file)
{
  CalculiXStepsInterface ccx_steps_iface;

  output_file << ccx_steps_iface.get_physicalconstants_export_data();
  return true;
}

bool ccxExportCommand::write_initialconditions(std::ofstream& output_file)
{
  CalculiXStepsInterface ccx_steps_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  if (ccx_topology_iface.is_topology_adjoint_export())
  {
    return true;
  }
  output_file << ccx_steps_iface.get_initialcondition_export_data();
  return true;
}

bool ccxExportCommand::write_hbcs(std::ofstream& output_file)
{
  CalculiXBCsInterface ccx_bcs_iface;
  CalculiXTopologyInterface ccx_topology_iface;

  if (ccx_topology_iface.is_topology_adjoint_export())
  {
    return true;
  }
  output_file << ccx_bcs_iface.get_hbc_export_data();
  return true;
}

bool ccxExportCommand::write_steps(std::ofstream& output_file)
{
  CalculiXStepsInterface ccx_steps_iface;
  CalculiXTopologyInterface ccx_topology_iface;
  std::string data;

  try
  {
    data = ccx_steps_iface.get_step_export_data();
  }
  catch (const std::exception& error)
  {
    PRINT_ERROR("Failed exporting step data: %s\n", error.what());
    return false;
  }
  catch (const char* error)
  {
    PRINT_ERROR("Failed exporting step data: %s\n", error ? error : "Null exception message");
    return false;
  }
  catch (...)
  {
    PRINT_ERROR("Failed exporting step data: unknown exception type.\n");
    return false;
  }

  if (data.empty())
  {
    if (ccx_topology_iface.is_topology_adjoint_export())
    {
      PRINT_ERROR("Unable to export the topology adjoint step.\n");
    }
    else
    {
      PRINT_ERROR("Unable to export step data.\n");
    }

    return false;
  }

  output_file << data;
  return output_file.good();
}
