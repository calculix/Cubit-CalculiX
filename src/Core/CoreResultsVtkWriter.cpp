#include "CoreResultsVtkWriter.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CoreResultsFrd.hpp"
#include "CoreResultsDat.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"
#include "MeshExportInterface.hpp"

#include <chrono>
#include <cmath>
#include <fstream>
#include <iostream>

CoreResultsVtkWriter::CoreResultsVtkWriter()
{}

CoreResultsVtkWriter::~CoreResultsVtkWriter()
{}

bool CoreResultsVtkWriter::init(int job_id,CoreResultsFrd* frd,CoreResultsDat* dat)
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    this->job_id = job_id;

    std::vector<std::string> job_data = ccx_iface->get_job_data(job_id);
    this->filepath = job_data[1];

    this->frd = frd;
    this->dat = dat;
    this->frd_all = frd;
    this->dat_all = dat;

    progressbar = new ProgressTool();
    me_iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
    me_iface->initialize_export();

    nparts_dat += dat_all->result_block_set.size(); // already looked here, or no check if the dat file can be linked will be made

    is_initialized = true;  
    return true;
  }
}

bool CoreResultsVtkWriter::reset()
{
  clear();
  init(-1,nullptr,nullptr);
  return true;
}

bool CoreResultsVtkWriter::clear()
{
  current_offset = 0;
  current_increment = 0;
  // erase all files
  std::string shellstr;
  shellstr = "rm " + filepath + "*vtu";
  system(shellstr.c_str());
  shellstr = "rm " + filepath + "*vtpc";
  system(shellstr.c_str());

  nparts = 0;
  nparts_dat = 0;
  part_names.clear();

  for (size_t i = 0; i < vec_frd.size(); i++)
  {
    delete vec_frd[i];
  }
  
  vec_frd.clear();
  
  block_ids.clear();
  block_node_ids.clear();
  block_element_ids.clear();
  nodeset_ids.clear();
  nodeset_node_ids.clear();
  sideset_ids.clear();
  sideset_node_ids.clear();
  sideset_elements.clear();
  sideset_elements_type.clear();
  sideset_elements_connectivity.clear();
  return true;
}

bool CoreResultsVtkWriter::clearLinked()
{
  current_offset = 0;
  return true;
}

bool CoreResultsVtkWriter::check_initialized()
{
  return is_initialized;
}

bool CoreResultsVtkWriter::write()
{
  std::string log;

  if (this->checkLinkPossible())
  {
    log = "Linked mode possible.\n";
    PRINT_INFO("%s", log.c_str());
    this->write_linked();
    log = "Finished writing in Linked Mode.\n";
    write_mode = 1;
  }else{
    log = "Linked mode NOT possible.\n";
    PRINT_INFO("%s", log.c_str());
    this->write_vtu_unlinked();
    log = "Finished writing.\n";
    write_mode = 2;
  }
  PRINT_INFO("%s", log.c_str());

  return true;
}

bool CoreResultsVtkWriter::write_linked()
{
  // clear all data before reading and check results
  this->clear();
  this->checkResults();

  // blocks
  block_ids = ccx_iface->get_blocks();
  for (size_t i = 0; i < block_ids.size(); i++)
  {
    block_node_ids.push_back(ccx_iface->get_block_node_ids(block_ids[i]));
    block_element_ids.push_back(ccx_iface->get_block_element_ids(block_ids[i]));
    part_names.push_back("Block: " + ccx_iface->get_block_name(block_ids[i]));
  }
  nparts += block_ids.size();

  // nodesets
  nodeset_ids = CubitInterface::get_nodeset_id_list();
  for (size_t i = 0; i < nodeset_ids.size(); i++)
  {
    nodeset_node_ids.push_back(CubitInterface::get_nodeset_nodes_inclusive(nodeset_ids[i]));
    part_names.push_back("Nodeset: " + ccx_iface->get_nodeset_name(nodeset_ids[i]));
  }
  nparts += nodeset_ids.size();    

  // sidesets
  // prepare sidesets
  this->prepare_sidesets();

  for (size_t i = 0; i < sideset_ids.size(); i++)
  {
    part_names.push_back("Sideset: " + ccx_iface->get_sideset_name(sideset_ids[i]));
  }
  nparts += sideset_ids.size();

  progressbar->start(0,100,"Writing Results to ParaView Format - Linked Mode");
  auto t_start = std::chrono::high_resolution_clock::now();

  //dat file
  nparts += dat_all->result_block_set.size();
  nparts_dat += dat_all->result_block_set.size();
  for (size_t i = 0; i < nparts_dat; i++)
  {
    part_names.push_back(".dat: " + dat_all->result_block_set[i]);
  }
  
  // prepare frd and dat
  for (size_t i = 0; i < nparts; i++)
  {
    CoreResultsFrd* tmp_frd;

    tmp_frd = new CoreResultsFrd();
    tmp_frd->init(job_id);

    vec_frd.push_back(tmp_frd);
  }

  //link nodes
  this->link_nodes();
  //link elements
  this->link_elements();
  //link dat
  this->link_dat();
  progressbar->start(0,100,"Writing Results to ParaView Format - Linked Mode");
  t_start = std::chrono::high_resolution_clock::now();

  current_increment = 0;
  for (size_t i = 0; i < max_increments; i++)
  {
    filepath_vtu.clear();
    part_ids.clear();

    ++current_increment;
    current_part = -1;
    // prepare frd and dat
    for (size_t ii = 0; ii < nparts; ii++)
    {
      ++current_part;

      frd = vec_frd[current_part];

      //std::string log;
      //log = "nparts " + std::to_string(nparts) + " block " + std::to_string(block_ids[ii])+ " block size" + std::to_string(block_ids.size())+" ci " + std::to_string(current_increment) + " \n";
      //ccx_iface->log_str(log);
      //PRINT_INFO("%s", log.c_str());
      
      current_filepath_vtu = filepath + "." + std::to_string(current_part) + "." + this->get_increment() + ".vtu";
      
      this->write_vtu_linked();

      filepath_vtu.push_back(current_filepath_vtu);
      part_ids.push_back(current_part);
    }

    current_filepath_vtpc = filepath + "." + this->get_increment() + ".vtpc";
    this->write_vtpc();

    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //currentDataRow += frd->result_block_data[data_ids[ii]].size();
    if (duration > 500)
    {
      progressbar->percent(double(current_increment)/double(max_increments));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar->end();
  return true;
}

bool CoreResultsVtkWriter::write_vtpc()
{
  std::string output = "";
  
  output.append(this->level_whitespace(0) + "<VTKFile type=\"vtkPartitionedDataSetCollection\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n");
  output.append(this->level_whitespace(1) + "<vtkPartitionedDataSetCollection>\n");
  for (size_t i = 0; i < part_ids.size(); i++)
  {
    output.append(this->level_whitespace(2) + "<Partitions index=\"" + std::to_string(part_ids[i]) +"\" name=\"" + part_names[i] +"\">\n");
    output.append(this->level_whitespace(3) + "<DataSet index=\"0\" file=\"" + filepath_vtu[i] + "\"/>\n");
    output.append(this->level_whitespace(2) + "</Partitions>\n");
  }
  output.append(this->level_whitespace(1) + "</vtkPartitionedDataSetCollection>\n");
  output.append(this->level_whitespace(1) + "<FieldData>\n");
  output.append(this->level_whitespace(2) + "<DataArray type=\"Float64\" Name=\"TimeValue\" NumberOfTuples=\"1\">\n");
  output.append(this->level_whitespace(3) + this->get_increment_time() + "\n");
  output.append(this->level_whitespace(2) + "</DataArray>\n");
  output.append(this->level_whitespace(1) + "</FieldData>\n");
  output.append(this->level_whitespace(0) + "</VTKFile>\n");

  this->write_to_file(current_filepath_vtpc,output);

  return true;
}

bool CoreResultsVtkWriter::write_vtu_linked()
{
  std::string output = "";
  std::string output_nodes_ids = "";
  std::string output_nodes = "";
  std::string output_elements_ids = "";
  std::string output_element_connectivity = "";
  std::string output_element_offsets = "";
  std::string output_element_types = "";
  int min_node_id = -1;
  int max_node_id = -1;
  int min_element_id = -1;
  int max_element_id = -1;
  std::string log;
  //log = "writing results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  //ccx_iface->log_str(log);
  //PRINT_INFO("%s", log.c_str());
  
  // clear all data before reading and check results
  this->clearLinked();
  this->checkResultsLinked();

  // write nodes
  output_nodes.append(this->level_whitespace(3) + "<Points>\n");
  output_nodes.append(this->level_whitespace(4) + "<DataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">\n");
  for (size_t i = 0; i < frd->nodes.size(); i++)
  {
    output_nodes.append(this->level_whitespace(5));
    output_nodes.append(ccx_iface->to_string_scientific(frd->nodes_coords[frd->nodes[i][1]][0]) + " ");
    output_nodes.append(ccx_iface->to_string_scientific(frd->nodes_coords[frd->nodes[i][1]][1]) + " ");
    output_nodes.append(ccx_iface->to_string_scientific(frd->nodes_coords[frd->nodes[i][1]][2]) + "\n");
    
    if (i==0)
    {
      min_node_id = frd->nodes[i][0];
      max_node_id = frd->nodes[i][0];
    }
    if (frd->nodes[i][0]<min_node_id)
    {
      min_node_id=frd->nodes[i][0];
    }
    if (frd->nodes[i][0]>max_node_id)
    {
      max_node_id=frd->nodes[i][0];
    }    

    output_nodes_ids.append(this->level_whitespace(5));
    output_nodes_ids.append(std::to_string(frd->nodes[i][0]) + "\n");
  }
  output_nodes.append(this->level_whitespace(4) + "</DataArray>\n");
  output_nodes.append(this->level_whitespace(3) + "</Points>\n");
  // write elements
  for (size_t i = 0; i < frd->elements.size(); i++)
  {
    if (i==0)
    {
      min_element_id = frd->elements[i][0];
      max_element_id = frd->elements[i][0];
    }
    if (frd->elements[i][0]<min_element_id)
    {
      min_element_id=frd->elements[i][0];
    }
    if (frd->elements[i][0]>max_element_id)
    {
      max_element_id=frd->elements[i][0];
    }

    output_elements_ids.append(this->level_whitespace(5));
    output_elements_ids.append(std::to_string(frd->elements[i][0]) + "\n");

    output_element_connectivity.append(this->level_whitespace(5));
    output_element_connectivity.append(this->get_element_connectivity_vtk_linked(frd->elements[i][2],frd->elements[i][1]) + "\n");

    output_element_offsets.append(this->level_whitespace(5));
    output_element_offsets.append(this->get_element_offset_vtk(frd->elements[i][2]) + "\n");

    output_element_types.append(this->level_whitespace(5));
    output_element_types.append(this->get_element_type_vtk(frd->elements[i][1]) + "\n");
  }
  
  for (size_t i = 0; i < 1; i++)
  { 
    output = "";
    // write header
    output.append(this->level_whitespace(0) + "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n");
    output.append(this->level_whitespace(1) + "<UnstructuredGrid>\n");
    output.append(this->level_whitespace(2) + "<Piece NumberOfPoints=\"" + std::to_string(frd->nodes.size()) + "\" NumberOfCells=\"" + std::to_string(frd->elements.size()) + "\">\n");  
    output.append(this->level_whitespace(3) + "<PointData GlobalIds=\"ids\">\n");
    //node ids
    output.append(this->level_whitespace(4) + "<DataArray type=\"Int64\" IdType=\"1\" Name=\"ids\" format=\"ascii\" RangeMin=\"" + std::to_string(min_node_id) + "\" RangeMax=\"" + std::to_string(max_node_id)+ "\">\n");
    output.append(output_nodes_ids);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
  
     //write result blocks
    std::vector<int> data_ids = this->get_result_blocks_data_ids(); // get data ids for result blocks in current increment
    
    for (size_t ii = 0; ii < data_ids.size(); ii++)
    {
      rangeMin = 0;
      rangeMax = 0;
      std::vector<int> node_data_ids = this->get_result_block_node_data_id(data_ids[ii]);

      // skip if nodes from point data is different than nodes number, like for data from CELS
      if (node_data_ids.size()==frd->nodes.size())
      {
        current_time = frd->total_times[frd->result_blocks[data_ids[ii]][4]];
        // header
        output.append(this->level_whitespace(4) + "<DataArray type=\"Float64\" ");
        output.append("Name=\"" + frd->result_block_type[frd->result_blocks[data_ids[ii]][5]] + "\" ");
        output.append("NumberOfComponents=\"" + std::to_string(frd->result_block_components[frd->result_blocks[data_ids[ii]][6]].size()) + "\" ");
        for (size_t iii = 0; iii < frd->result_block_components[frd->result_blocks[data_ids[ii]][6]].size(); iii++)
        {
          output.append("ComponentName"+ std::to_string(iii) + " =\"" + frd->result_block_components[frd->result_blocks[data_ids[ii]][6]][iii] +"\" ");
        }
        output.append("format=\"ascii\" RangeMin=\"" + std::to_string(rangeMin) + "\" RangeMax=\"" + std::to_string(rangeMin) + "\">\n");
        
        for (size_t iii = 0; iii < node_data_ids.size(); iii++)
        {
          output.append(this->level_whitespace(5) + this->get_result_data(data_ids[ii], node_data_ids[iii]) + "\n");
        }
        // footer
        output.append(this->level_whitespace(4) + "</DataArray>\n");
      }
    }

    output.append(this->level_whitespace(3) + "</PointData>\n");
    //element ids
    output.append(this->level_whitespace(3) + "<CellData GlobalIds=\"ids\">\n");
    output.append(this->level_whitespace(4) + "<DataArray type=\"Int64\" IdType=\"1\" Name=\"ids\" format=\"ascii\" RangeMin=\"" + std::to_string(min_element_id) + "\" RangeMax=\"" + std::to_string(max_element_id)+ "\">\n");
    output.append(output_elements_ids);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
    output.append(this->level_whitespace(3) + "</CellData>\n");
    //append nodes and elements
    output.append(output_nodes);
    output.append(this->level_whitespace(3) + "<Cells>\n");
    output.append(this->level_whitespace(4) + "<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n");
    output.append(output_element_connectivity);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
    output.append(this->level_whitespace(4) + "<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n");
    output.append(output_element_offsets);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
    output.append(this->level_whitespace(4) + "<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n");
    output.append(output_element_types);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
    output.append(this->level_whitespace(3) + "</Cells>\n");

    // write footer
    output.append(this->level_whitespace(2) + "</Piece>\n");  
    output.append(this->level_whitespace(1) + "</UnstructuredGrid>\n");
    output.append(this->level_whitespace(0) + "</VTKFile>\n");

    this->write_to_file(current_filepath_vtu,output);    
  }
  return true;
}

bool CoreResultsVtkWriter::write_vtu_unlinked()
{
  std::string output = "";
  std::string output_nodes_ids = "";
  std::string output_nodes = "";
  std::string output_elements_ids = "";
  std::string output_element_connectivity = "";
  std::string output_element_offsets = "";
  std::string output_element_types = "";
  int min_node_id = -1;
  int max_node_id = -1;
  int min_element_id = -1;
  int max_element_id = -1;
  std::string log;
  //log = "writing results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  //ccx_iface->log_str(log);
  //PRINT_INFO("%s", log.c_str());
  
  // clear all data before reading and check results
  this->clear();
  this->checkResults();

  progressbar->start(0,100,"Writing Results to ParaView Format - Preparing Nodes and Elements");
  auto t_start = std::chrono::high_resolution_clock::now();

  // write nodes
  output_nodes.append(this->level_whitespace(3) + "<Points>\n");
  output_nodes.append(this->level_whitespace(4) + "<DataArray type=\"Float32\" Name=\"Points\" NumberOfComponents=\"3\" format=\"ascii\">\n");
  for (size_t i = 0; i < frd->nodes.size(); i++)
  {
    output_nodes.append(this->level_whitespace(5));
    output_nodes.append(ccx_iface->to_string_scientific(frd->nodes_coords[frd->nodes[i][1]][0]) + " ");
    output_nodes.append(ccx_iface->to_string_scientific(frd->nodes_coords[frd->nodes[i][1]][1]) + " ");
    output_nodes.append(ccx_iface->to_string_scientific(frd->nodes_coords[frd->nodes[i][1]][2]) + "\n");
    
    if (i==0)
    {
      min_node_id = frd->nodes[i][0];
      max_node_id = frd->nodes[i][0];
    }
    if (frd->nodes[i][0]<min_node_id)
    {
      min_node_id=frd->nodes[i][0];
    }
    if (frd->nodes[i][0]>max_node_id)
    {
      max_node_id=frd->nodes[i][0];
    }    

    output_nodes_ids.append(this->level_whitespace(5));
    output_nodes_ids.append(std::to_string(frd->nodes[i][0]) + "\n");

    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    ++currentDataRow;
    if (duration > 500)
    {
      progressbar->percent(double(currentDataRow)/double(maxDataRows));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  output_nodes.append(this->level_whitespace(4) + "</DataArray>\n");
  output_nodes.append(this->level_whitespace(3) + "</Points>\n");
  // write elements
  this->rewrite_connectivity_unlinked(); // correct the node ids from ccx to vtk in the connectivity
  for (size_t i = 0; i < frd->elements.size(); i++)
  {

    if (i==0)
    {
      min_element_id = frd->elements[i][0];
      max_element_id = frd->elements[i][0];
    }
    if (frd->elements[i][0]<min_element_id)
    {
      min_element_id=frd->elements[i][0];
    }
    if (frd->elements[i][0]>max_element_id)
    {
      max_element_id=frd->elements[i][0];
    }

    output_elements_ids.append(this->level_whitespace(5));
    output_elements_ids.append(std::to_string(frd->elements[i][0]) + "\n");

    output_element_connectivity.append(this->level_whitespace(5));
    output_element_connectivity.append(this->get_element_connectivity_vtk(frd->elements[i][2],frd->elements[i][1]) + "\n");

    output_element_offsets.append(this->level_whitespace(5));
    output_element_offsets.append(this->get_element_offset_vtk(frd->elements[i][2]) + "\n");

    output_element_types.append(this->level_whitespace(5));
    output_element_types.append(this->get_element_type_vtk(frd->elements[i][1]) + "\n");
    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    ++currentDataRow;
    if (duration > 500)
    {
      progressbar->percent(double(currentDataRow)/double(maxDataRows));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }

  progressbar->end();
  progressbar->start(0,100,"Writing Results to ParaView Format - Unlinked Mode");
  t_start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < max_increments; i++)
  { 
    output = "";
    ++current_increment;
    // write header
    output.append(this->level_whitespace(0) + "<VTKFile type=\"UnstructuredGrid\" version=\"1.0\" byte_order=\"LittleEndian\" header_type=\"UInt64\">\n");
    output.append(this->level_whitespace(1) + "<UnstructuredGrid>\n");
    output.append(this->level_whitespace(2) + "<Piece NumberOfPoints=\"" + std::to_string(frd->nodes.size()) + "\" NumberOfCells=\"" + std::to_string(frd->elements.size()) + "\">\n");  
    output.append(this->level_whitespace(3) + "<PointData GlobalIds=\"ids\">\n");
    //node ids
    output.append(this->level_whitespace(4) + "<DataArray type=\"Int64\" IdType=\"1\" Name=\"ids\" format=\"ascii\" RangeMin=\"" + std::to_string(min_node_id) + "\" RangeMax=\"" + std::to_string(max_node_id)+ "\">\n");
    output.append(output_nodes_ids);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
  
     //write result blocks
    std::vector<int> data_ids = this->get_result_blocks_data_ids(); // get data ids for result blocks in current increment
    
    for (size_t ii = 0; ii < data_ids.size(); ii++)
    {
      rangeMin = 0;
      rangeMax = 0;
      std::vector<int> node_data_ids = this->get_result_block_node_data_id(data_ids[ii]);

      // skip if nodes from point data is different than nodes number, like for data from CELS
      if (node_data_ids.size()==frd->nodes.size())
      {
        current_time = frd->total_times[frd->result_blocks[data_ids[ii]][4]];
        // header
        output.append(this->level_whitespace(4) + "<DataArray type=\"Float64\" ");
        output.append("Name=\"" + frd->result_block_type[frd->result_blocks[data_ids[ii]][5]] + "\" ");
        output.append("NumberOfComponents=\"" + std::to_string(frd->result_block_components[frd->result_blocks[data_ids[ii]][6]].size()) + "\" ");
        for (size_t iii = 0; iii < frd->result_block_components[frd->result_blocks[data_ids[ii]][6]].size(); iii++)
        {
          output.append("ComponentName"+ std::to_string(iii) + " =\"" + frd->result_block_components[frd->result_blocks[data_ids[ii]][6]][iii] +"\" ");
        }
        output.append("format=\"ascii\" RangeMin=\"" + std::to_string(rangeMin) + "\" RangeMax=\"" + std::to_string(rangeMin) + "\">\n");
        
        for (size_t iii = 0; iii < node_data_ids.size(); iii++)
        {
          output.append(this->level_whitespace(5) + this->get_result_data(data_ids[ii], node_data_ids[iii]) + "\n");
        }

        // footer
        output.append(this->level_whitespace(4) + "</DataArray>\n");

        //update progress bar
        const auto t_end = std::chrono::high_resolution_clock::now();
        int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        //currentDataRow += frd->result_block_data[data_ids[ii]].size();
        if (duration > 500)
        {
          progressbar->percent(double(current_increment)/double(max_increments));
          progressbar->check_interrupt();
          t_start = std::chrono::high_resolution_clock::now();
        }
      }
    }

    output.append(this->level_whitespace(3) + "</PointData>\n");
    //element ids
    output.append(this->level_whitespace(3) + "<CellData GlobalIds=\"ids\">\n");
    output.append(this->level_whitespace(4) + "<DataArray type=\"Int64\" IdType=\"1\" Name=\"ids\" format=\"ascii\" RangeMin=\"" + std::to_string(min_element_id) + "\" RangeMax=\"" + std::to_string(max_element_id)+ "\">\n");
    output.append(output_elements_ids);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
    output.append(this->level_whitespace(3) + "</CellData>\n");
    //append nodes and elements
    output.append(output_nodes);
    output.append(this->level_whitespace(3) + "<Cells>\n");
    output.append(this->level_whitespace(4) + "<DataArray type=\"Int64\" Name=\"connectivity\" format=\"ascii\">\n");
    output.append(output_element_connectivity);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
    output.append(this->level_whitespace(4) + "<DataArray type=\"Int64\" Name=\"offsets\" format=\"ascii\">\n");
    output.append(output_element_offsets);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
    output.append(this->level_whitespace(4) + "<DataArray type=\"UInt8\" Name=\"types\" format=\"ascii\">\n");
    output.append(output_element_types);
    output.append(this->level_whitespace(4) + "</DataArray>\n");
    output.append(this->level_whitespace(3) + "</Cells>\n");

    // write footer
    output.append(this->level_whitespace(2) + "</Piece>\n");  
    output.append(this->level_whitespace(1) + "</UnstructuredGrid>\n");
    output.append(this->level_whitespace(0) + "</VTKFile>\n");

    if (max_increments==1)
    {
      this->write_to_file(filepath + ".vtu",output);
    }else{
      this->write_to_file(filepath + "." + this->get_increment() +  ".vtu",output);
    }
  }
  
  progressbar->end();

  return true;
}

bool CoreResultsVtkWriter::write_to_file(std::string filepath, std::string &content)
{
  std::ofstream output_file;
  output_file.open(filepath);
  output_file << content;
  output_file.close();
  return true;
}

int CoreResultsVtkWriter::getMaxDataRows()
{
  int dataRows = 0;
  
  dataRows += frd->nodes.size();
  dataRows += frd->elements.size();

  /* 
  for (size_t i = 0; i < frd->result_block_data.size(); i++)
  {
    dataRows += frd->result_block_data[i].size();
  }
  
  for (size_t i = 0; i < dat->result_block_data.size(); i++)
  {
    dataRows += dat->result_block_data[i].size();
  }
  */

  return dataRows;
}

std::string CoreResultsVtkWriter::level_whitespace(int level)
{
  std::string whitespace = "";
  for (size_t i = 0; i < level; i++)
  {
    whitespace.append("  ");
  }
  return whitespace;
}

std::string CoreResultsVtkWriter::get_increment()
{
  std::string increment = std::to_string(current_increment);
  std::string zeros = "";
  if(increment.length()!=3)
  {  
    for (size_t i = increment.length(); i < 4; i++)
    {
      zeros.append("0");
    }
    increment = zeros + std::to_string(current_increment);
  }
  
  return increment;
}

std::string CoreResultsVtkWriter::get_increment_time()
{
  std::string time = std::to_string(current_time); 
  return time;
}

int CoreResultsVtkWriter::get_step_increment(double total_time)
{
  int increment = 0;
  double last_total_time = 0;
  for (size_t i = 0; i < dat_all->total_times.size(); i++)
  {
    if (last_total_time!=dat_all->total_times[i])
    {
      ++increment;
    }
    if (dat_all->total_times[i]==total_time)
    {
      return increment;
    }
    last_total_time = dat_all->total_times[i];
  }
  return -1;
}

int CoreResultsVtkWriter::get_max_step_increment()
{
  int increment = 0;
  double last_total_time = 0;
  for (size_t i = 0; i < dat_all->total_times.size(); i++)
  {
    if (last_total_time!=dat_all->total_times[i])
    {
      ++increment;
    }
    last_total_time = dat_all->total_times[i];
  }
  return increment;
}

bool CoreResultsVtkWriter::rewrite_connectivity_unlinked()
{  
  for (size_t i = 0; i < frd->nodes.size(); i++)
  {
    for (size_t ii = 0; ii < frd->elements_connectivity.size(); ii++)
    {
      for (size_t iii = 0; iii < frd->elements_connectivity[ii].size(); iii++)
      {
        if (frd->elements_connectivity[ii][iii]==frd->nodes[i][0])
        {
          frd->elements_connectivity[ii][iii] = i;
        }
      }
    }
  }
  return true;
}

std::string CoreResultsVtkWriter::get_element_connectivity_vtk(int element_connectivity_data_id, int element_type) // gets the connectivity already converted to vtk format
{
  std::string str_result = "";

  std::vector<int> result_connectivity;

  if (element_type == 4)
  {
    result_connectivity = frd->elements_connectivity[element_connectivity_data_id];
    //switch positions    
    result_connectivity[12] = frd->elements_connectivity[element_connectivity_data_id][16];
    result_connectivity[13] = frd->elements_connectivity[element_connectivity_data_id][17];
    result_connectivity[14] = frd->elements_connectivity[element_connectivity_data_id][18];
    result_connectivity[15] = frd->elements_connectivity[element_connectivity_data_id][19];
    result_connectivity[16] = frd->elements_connectivity[element_connectivity_data_id][12];
    result_connectivity[17] = frd->elements_connectivity[element_connectivity_data_id][13];
    result_connectivity[18] = frd->elements_connectivity[element_connectivity_data_id][14];
    result_connectivity[19] = frd->elements_connectivity[element_connectivity_data_id][15];
  }else{
    result_connectivity = frd->elements_connectivity[element_connectivity_data_id];
  }
  
  for (size_t i = 0; i < result_connectivity.size(); i++)
  {
    str_result.append(std::to_string(result_connectivity[i]));
    if (i!=result_connectivity.size()-1)
    {
      str_result.append(" ");
    }
  }
  return str_result;
}

std::string CoreResultsVtkWriter::get_element_connectivity_vtk_linked(int element_connectivity_data_id, int element_type) // gets the connectivity already converted to vtk format
{
  std::string str_result = "";

  std::vector<int> result_connectivity;

  if (element_type == 4)
  {
    result_connectivity = frd->elements_connectivity[element_connectivity_data_id];
    //switch positions    
    result_connectivity[12] = frd->elements_connectivity[element_connectivity_data_id][16];
    result_connectivity[13] = frd->elements_connectivity[element_connectivity_data_id][17];
    result_connectivity[14] = frd->elements_connectivity[element_connectivity_data_id][18];
    result_connectivity[15] = frd->elements_connectivity[element_connectivity_data_id][19];
    result_connectivity[16] = frd->elements_connectivity[element_connectivity_data_id][12];
    result_connectivity[17] = frd->elements_connectivity[element_connectivity_data_id][13];
    result_connectivity[18] = frd->elements_connectivity[element_connectivity_data_id][14];
    result_connectivity[19] = frd->elements_connectivity[element_connectivity_data_id][15];
  }else{
    result_connectivity = frd->elements_connectivity[element_connectivity_data_id];
  }
  
  for (size_t i = 0; i < result_connectivity.size(); i++)
  {
    result_connectivity[i] = this->getParaviewNode(result_connectivity[i]);
    str_result.append(std::to_string(result_connectivity[i]));
    if (i!=result_connectivity.size()-1)
    {
      str_result.append(" ");
    }
  }
  return str_result;
}

std::string CoreResultsVtkWriter::get_element_type_vtk(int element_type) // gets the element type already converted to vtk format
{
  std::string str_result = "type ";
  std::vector<int> ccx_type = { 1, 2, 3, 4, 5, 6,7, 8,9,10,11,12,99}; // type 99 is a sphere, used for display of nodesets and integration points
  std::vector<int> vtk_type = {12,13,10,25,26,24,5,22,9,23, 3,21, 1};

  for (size_t i = 0; i < ccx_type.size(); i++)
  {
    if (element_type == ccx_type[i])
    {
      str_result = std::to_string(vtk_type[i]);
    }
  }
  return str_result;
}

std::string CoreResultsVtkWriter::get_element_offset_vtk(int element_connectivity_data_id) // gets the element type already converted to vtk format
{
  std::string str_result = "offset ";
  this->current_offset += frd->elements_connectivity[element_connectivity_data_id].size();
  
  str_result = std::to_string(this->current_offset);

  return str_result;
}

int CoreResultsVtkWriter::getParaviewNode(int frd_node_id)
{
  // if dat file integration point data, no correct node will be found
  if (current_part > nparts-nparts_dat - 1)
  {
    return frd_node_id;
  }

  for (size_t i = 0; i < frd->nodes.size(); i++)
  {
    if (frd->nodes[i][0] == frd_node_id)
    {
      return i;
    }
  }
  
  return -1;
}

bool CoreResultsVtkWriter::checkResults()
{
  maxDataRows = 0;
  currentDataRow = 0;
  max_increments = 0;
  current_increment = 0;

  //get max increments
  for (size_t i = 0; i < frd_all->result_blocks.size(); i++)
  {
    if (max_increments<frd_all->result_blocks[i][3])
    {
      max_increments = frd_all->result_blocks[i][3];
    }
  }

  // get # of maxDataRows which needs to be processed
  maxDataRows = this->getMaxDataRows();

  return true;
}

bool CoreResultsVtkWriter::checkResultsLinked()
{
  maxDataRows = 0;
  currentDataRow = 0;

  // get # of maxDataRows which needs to be processed
  maxDataRows = this->getMaxDataRows();

  return true;
}

bool CoreResultsVtkWriter::checkLinkPossible()
{
  std::string log;

  if (frd->nodes.size()==0)
  {
    return false;
  }
  
  for (size_t i = 0; i < frd->nodes.size(); i++)
  {
    if (!CubitInterface::get_node_exists(frd->nodes[i][0]))
    {
      log = "Linking Failed! Node " + std::to_string(frd->nodes[i][0]) + " does'nt exist in Cubit.\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
  }
  for (size_t i = 0; i < frd->elements.size(); i++)
  {
    if (!CubitInterface::get_element_exists(frd->elements[i][0]))
    { 
      log = "Linking Failed! Element " + std::to_string(frd->elements[i][0]) + " does'nt exist in Cubit.\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
  }
  if (CubitInterface::get_node_count()!=frd->nodes.size()+CubitInterface::get_list_of_free_ref_entities("vertex").size())
  {
    log = "Linking Failed! Wrong number of Nodes.\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }
  if (CubitInterface::get_element_count()!=frd->elements.size()+CubitInterface::get_list_of_free_ref_entities("vertex").size())
  {
    log = "Linking Failed! Wrong number of Elements.\n";
    PRINT_INFO("%s", log.c_str());
    return false;
  }

  if (nparts_dat!=0) // if dat exists in .dat file
  {
    if (dat_all->result_blocks.size()!=0)
    {
      //check if for every frd timestep that there is a dat timestep
      for (size_t i = 0; i < frd_all->total_times.size(); i++)
      {
        if (!checkDatTimeValueExists(frd_all->total_times[i]))
        {
          log = "Linking Failed! Not for every .frd time value a .dat time value exists.\n";
          log.append("Time " + std::to_string(frd_all->total_times[i]) + " wasn't found in the .dat file.\n");
          PRINT_INFO("%s", log.c_str());
          return false;
        }
      }
    }
    max_increments = 0;
    //get max increments
    for (size_t i = 0; i < frd_all->result_blocks.size(); i++)
    {
      if (max_increments < frd_all->result_blocks[i][3])
      {
        max_increments = frd_all->result_blocks[i][3];
      }
    }

    if (max_increments!=this->get_max_step_increment())
    {
      log = "Linking Failed! Different number of increments in .frd (" + std::to_string(max_increments) + ") and .dat (" + std::to_string(get_max_step_increment()) + ") \n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
  }
  
  return true;
}

bool CoreResultsVtkWriter::checkDatTimeValueExists(double total_time)
{
  for (size_t i = 0; i < dat_all->total_times.size(); i++)
  {
    // .dat times have different accuracy than in the .frd file *facepalm*
    if (ccx_iface->to_string_scientific(dat_all->total_times[i])==ccx_iface->to_string_scientific(total_time))
    {
      return true;
    }
  }
  return false;
}

bool CoreResultsVtkWriter::checkFrdBlockDispExists(std::string block_name)
{
  std::string block_name_cubit;
  // blocks
  for (size_t i = 0; i < block_ids.size(); i++)
  {
    block_name_cubit = ccx_iface->get_block_name(block_ids[i]);

    if (block_name_cubit.length()!=block_name.length())
    {
      return false;
    }
    
    // make all lower case
    for(auto& c : block_name)
    {
      c = tolower(c);
    }
    for(auto& c : block_name_cubit)
    {
      c = tolower(c);
    }

    if (block_name_cubit == block_name)
    {
      // search for disp in current increment
      frd = vec_frd[i];
      std::vector<int> data_ids = this->get_result_blocks_data_ids();
      for (size_t ii = 0; ii < data_ids.size(); ii++)
      {
        if (frd->result_block_type[frd->result_blocks[data_ids[ii]][5]]=="DISP")
        {
          return true;
        }
      }
    }
  }

  return false;
}

int CoreResultsVtkWriter::getFrdBlockId(std::string block_name)
{
  std::string block_name_cubit;
  // blocks
  for (size_t i = 0; i < block_ids.size(); i++)
  {
    block_name_cubit = ccx_iface->get_block_name(block_ids[i]);

    if (block_name_cubit.length()!=block_name.length())
    {
      return false;
    }
    
    // make all lower case
    for(auto& c : block_name)
    {
      c = tolower(c);
    }
    for(auto& c : block_name_cubit)
    {
      c = tolower(c);
    }

    if (block_name_cubit == block_name)
    {
      return i;
    }
  }

  return -1;
}

std::vector<int> CoreResultsVtkWriter::get_currentincrement_result_blocks_data()
{
  std::vector<int> result_blocks_data;
  // result_blocks[0][1] step
  // result_blocks[0][2] step increment
  // result_blocks[0][3] total increment

  for (size_t i = 0; i < frd->result_blocks.size(); i++)
  {
    if (current_increment == frd->result_blocks[i][3])
    {
      result_blocks_data.push_back(frd->result_blocks[i][1]);
      result_blocks_data.push_back(frd->result_blocks[i][2]);
      result_blocks_data.push_back(frd->result_blocks[i][3]);

      return result_blocks_data;
    }
  }
  return {0,0,0};
}

std::vector<int> CoreResultsVtkWriter::get_result_blocks_data_ids()
{
  std::vector<int> data_ids;
  
  for (size_t i = 0; i < frd->result_blocks.size(); i++)
  {
    if (current_increment == frd->result_blocks[i][3])
    {
      data_ids.push_back(i);
    }
  }
  return data_ids;
}

std::vector<int> CoreResultsVtkWriter::get_result_blocks_data_ids_linked()
{
  std::vector<int> data_ids;
  
  for (size_t i = 0; i < frd_all->result_blocks.size(); i++)
  {
    if (current_increment == frd_all->result_blocks[i][3])
    {
      data_ids.push_back(i);
    }
  }
  return data_ids;
}

std::vector<int> CoreResultsVtkWriter::get_dat_result_blocks_data_ids_linked(int set_id)
{
  std::vector<int> data_ids;
  
  for (size_t i = 0; i < dat_all->result_blocks.size(); i++)
  {
    if ((current_increment == this->get_step_increment(dat_all->total_times[dat_all->result_blocks[i][1]]))&&(set_id==dat_all->result_blocks[i][3]))
    {
      data_ids.push_back(i);
    }
  }
  return data_ids;
}

std::vector<int> CoreResultsVtkWriter::get_result_block_node_data_id(int result_blocks_data_id)
{
  std::vector<int> data_ids;
  int current_node_id = 0;
  int last_node_id = 0;
  
  for (size_t i = 0; i < frd->result_block_node_data[result_blocks_data_id].size(); i++)
  {
    current_node_id = frd->result_block_node_data[result_blocks_data_id][i][0];
    if(i==0)
    {
      last_node_id = frd->result_block_node_data[result_blocks_data_id][i][0];
      data_ids.push_back(frd->result_block_node_data[result_blocks_data_id][i][1]);
    }else{
      if (last_node_id == current_node_id)
      {
        data_ids[data_ids.size()-1] = frd->result_block_node_data[result_blocks_data_id][i][1];
      }else{
        last_node_id = frd->result_block_node_data[result_blocks_data_id][i][0];
        data_ids.push_back(frd->result_block_node_data[result_blocks_data_id][i][1]);
      }
    }
  }
  return data_ids;
}

std::vector<int> CoreResultsVtkWriter::get_result_block_node_data_id_linked(int result_blocks_data_id)
{
  std::vector<int> data_ids;
  int current_node_id = 0;
  int last_node_id = 0;
  
  for (size_t i = 0; i < frd_all->result_block_node_data[result_blocks_data_id].size(); i++)
  {
    current_node_id = frd_all->result_block_node_data[result_blocks_data_id][i][0];
    if(i==0)
    {
      last_node_id = frd_all->result_block_node_data[result_blocks_data_id][i][0];
      data_ids.push_back(frd_all->result_block_node_data[result_blocks_data_id][i][1]);
    }else{
      if (last_node_id == current_node_id)
      {
        data_ids[data_ids.size()-1] = frd_all->result_block_node_data[result_blocks_data_id][i][1];
      }else{
        last_node_id = frd_all->result_block_node_data[result_blocks_data_id][i][0];
        data_ids.push_back(frd_all->result_block_node_data[result_blocks_data_id][i][1]);
      }
    }
  }
  return data_ids;
}

std::string CoreResultsVtkWriter::get_result_data(int data_id, int node_data_id)
{
  std::string str_result = "";
  std::vector<double> result_component;

  result_component = frd->result_block_data[data_id][node_data_id];
  
  for (size_t i = 0; i < result_component.size(); i++)
  {
    if (result_component[i]<rangeMin)
    {
      rangeMin=result_component[i];
    }
    if (result_component[i]>rangeMax)
    {
      rangeMax=result_component[i];
    }

    str_result.append(ccx_iface->to_string_scientific(result_component[i]));
    if (i!=result_component.size()-1)
    {
      str_result.append(" ");
    }
  }
  return str_result;  
}

bool CoreResultsVtkWriter::link_nodes()
{
  current_part = 0;
  std::vector<std::vector<int>> tmp_block_node_ids = block_node_ids;
  std::vector<std::vector<int>> tmp_nodeset_node_ids = nodeset_node_ids;
  std::vector<std::vector<int>> tmp_sideset_node_ids = sideset_node_ids;

  progressbar->start(0,100,"Writing Results to ParaView Format - Linking Nodes");
  auto t_start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < frd_all->nodes.size(); i++)
  {
    current_part = -1;
    //blocks
    for (size_t ii = 0; ii < block_ids.size(); ii++)
    {
      ++current_part;
      for (size_t iii = 0; iii < tmp_block_node_ids[ii].size(); iii++)
      {
        if (frd_all->nodes[i][0]==tmp_block_node_ids[ii][iii])
        {
          vec_frd[current_part]->nodes.push_back(frd_all->nodes[i]);
          vec_frd[current_part]->nodes_coords.push_back(frd_all->nodes_coords[i]);
          vec_frd[current_part]->nodes[vec_frd[current_part]->nodes.size()-1][1] = vec_frd[current_part]->nodes_coords.size()-1;
          tmp_block_node_ids[ii].erase(tmp_block_node_ids[ii].begin() + iii);
          --iii;
        }
      }
    }
    //nodesets
    for (size_t ii = 0; ii < nodeset_ids.size(); ii++)
    {
      ++current_part;
      for (size_t iii = 0; iii < tmp_nodeset_node_ids[ii].size(); iii++)
      {
        if (frd_all->nodes[i][0]==tmp_nodeset_node_ids[ii][iii])
        {
          vec_frd[current_part]->nodes.push_back(frd_all->nodes[i]);
          vec_frd[current_part]->nodes_coords.push_back(frd_all->nodes_coords[i]);
          vec_frd[current_part]->nodes[vec_frd[current_part]->nodes.size()-1][1] = vec_frd[current_part]->nodes_coords.size()-1;
          tmp_nodeset_node_ids[ii].erase(tmp_nodeset_node_ids[ii].begin() + iii);
          --iii;
        }
      }
    }
    //sidesets
    for (size_t ii = 0; ii < sideset_ids.size(); ii++)
    {
      ++current_part;
      for (size_t iii = 0; iii < tmp_sideset_node_ids[ii].size(); iii++)
      {
        if (frd_all->nodes[i][0]==tmp_sideset_node_ids[ii][iii])
        {
          vec_frd[current_part]->nodes.push_back(frd_all->nodes[i]);
          vec_frd[current_part]->nodes_coords.push_back(frd_all->nodes_coords[i]);
          vec_frd[current_part]->nodes[vec_frd[current_part]->nodes.size()-1][1] = vec_frd[current_part]->nodes_coords.size()-1;
          tmp_sideset_node_ids[ii].erase(tmp_sideset_node_ids[ii].begin() + iii);
          --iii;
        }
      }
    }
    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //currentDataRow += frd->result_block_data[data_ids[ii]].size();
    if (duration > 500)
    {
      progressbar->percent(double(i)/double(frd_all->nodes.size()-1));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar->end();

  for (size_t i = 0; i < nparts-nparts_dat; i++)
  {
    vec_frd[i]->result_blocks = frd_all->result_blocks;
    vec_frd[i]->total_times = frd_all->total_times;
    vec_frd[i]->result_block_components = frd_all->result_block_components;
    vec_frd[i]->result_block_type = frd_all->result_block_type;
  }

  progressbar->start(0,100,"Writing Results to ParaView Format - Linking Nodal Results");
  t_start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < max_increments; i++)
  {
    ++current_increment;
    std::vector<int> data_ids = this->get_result_blocks_data_ids_linked(); // get data ids for result blocks
    for (size_t ii = 0; ii < data_ids.size(); ii++)
    {
      tmp_block_node_ids = block_node_ids;
      tmp_nodeset_node_ids = nodeset_node_ids;
      tmp_sideset_node_ids = sideset_node_ids;

      std::vector<int> node_data_ids = this->get_result_block_node_data_id_linked(data_ids[ii]);
      for (size_t iii = 0; iii < nparts-nparts_dat; iii++)
      {
        std::vector<std::vector<double>> tmp_result_block_data;
        std::vector<std::vector<int>> tmp_result_block_node_data;   
        vec_frd[iii]->result_block_data.push_back(tmp_result_block_data);
        vec_frd[iii]->result_block_node_data.push_back(tmp_result_block_node_data);
      }

      for (size_t iii = 0; iii < node_data_ids.size(); iii++)
      { 
        current_part = -1;
        //blocks    
        for (size_t iv = 0; iv < block_ids.size(); iv++)
        {
          ++current_part;
          for (size_t v = 0; v < tmp_block_node_ids[iv].size(); v++)
          {
            if (frd_all->result_block_node_data[data_ids[ii]][node_data_ids[iii]][0]==tmp_block_node_ids[iv][v])
            {
              vec_frd[current_part]->result_block_data[data_ids[ii]].push_back(frd_all->result_block_data[data_ids[ii]][node_data_ids[iii]]);
              vec_frd[current_part]->result_block_node_data[data_ids[ii]].push_back(frd_all->result_block_node_data[data_ids[ii]][node_data_ids[iii]]);
              vec_frd[current_part]->result_block_node_data[data_ids[ii]][vec_frd[current_part]->result_block_node_data[data_ids[ii]].size()-1][1] = vec_frd[current_part]->result_block_data[data_ids[ii]].size()-1;
              tmp_block_node_ids[iv].erase(tmp_block_node_ids[iv].begin() + v);
              --v;
            }
          }
        }
        //nodesets
        for (size_t iv = 0; iv < nodeset_ids.size(); iv++)
        {
          ++current_part;
          for (size_t v = 0; v < tmp_nodeset_node_ids[iv].size(); v++)
          {
            if (frd_all->result_block_node_data[data_ids[ii]][node_data_ids[iii]][0]==tmp_nodeset_node_ids[iv][v])
            {
              vec_frd[current_part]->result_block_data[data_ids[ii]].push_back(frd_all->result_block_data[data_ids[ii]][node_data_ids[iii]]);
              vec_frd[current_part]->result_block_node_data[data_ids[ii]].push_back(frd_all->result_block_node_data[data_ids[ii]][node_data_ids[iii]]);
              vec_frd[current_part]->result_block_node_data[data_ids[ii]][vec_frd[current_part]->result_block_node_data[data_ids[ii]].size()-1][1] = vec_frd[current_part]->result_block_data[data_ids[ii]].size()-1;
              tmp_nodeset_node_ids[iv].erase(tmp_nodeset_node_ids[iv].begin() + v);
              --v;
            }
          }
        }
        //sidesets
        for (size_t iv = 0; iv < sideset_ids.size(); iv++)
        {
          ++current_part;
          for (size_t v = 0; v < tmp_sideset_node_ids[iv].size(); v++)
          {
            if (frd_all->result_block_node_data[data_ids[ii]][node_data_ids[iii]][0]==tmp_sideset_node_ids[iv][v])
            {
              vec_frd[current_part]->result_block_data[data_ids[ii]].push_back(frd_all->result_block_data[data_ids[ii]][node_data_ids[iii]]);
              vec_frd[current_part]->result_block_node_data[data_ids[ii]].push_back(frd_all->result_block_node_data[data_ids[ii]][node_data_ids[iii]]);
              vec_frd[current_part]->result_block_node_data[data_ids[ii]][vec_frd[current_part]->result_block_node_data[data_ids[ii]].size()-1][1] = vec_frd[current_part]->result_block_data[data_ids[ii]].size()-1;
              tmp_sideset_node_ids[iv].erase(tmp_sideset_node_ids[iv].begin() + v);
              --v;
            }
          }
        }
      }
    }
    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //currentDataRow += frd->result_block_data[data_ids[ii]].size();
    if (duration > 500)
    {
      progressbar->percent(double(current_increment)/double(max_increments));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar->end();
  current_increment = 0;
  
  return true;
}

bool CoreResultsVtkWriter::link_elements()
{
  current_part = 0;
  std::vector<std::vector<int>> tmp_block_element_ids = block_element_ids;

  progressbar->start(0,100,"Writing Results to ParaView Format - Linking Blocks");
  auto t_start = std::chrono::high_resolution_clock::now();             

  for (size_t i = 0; i < frd_all->elements.size(); i++)
  {
    current_part = -1;
    //blocks
    for (size_t ii = 0; ii < block_ids.size(); ii++)
    {
      ++current_part;
      for (size_t iii = 0; iii < tmp_block_element_ids[ii].size(); iii++)
      {
        if (frd_all->elements[i][0]==tmp_block_element_ids[ii][iii])
        {
          vec_frd[current_part]->elements.push_back(frd_all->elements[i]);
          vec_frd[current_part]->elements_connectivity.push_back(frd_all->elements_connectivity[i]);
          vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
          tmp_block_element_ids[ii].erase(tmp_block_element_ids[ii].begin() + iii);
          --iii;
        }
      }
    }
    
    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //currentDataRow += frd->result_block_data[data_ids[ii]].size();
    if (duration > 500)
    {
      progressbar->percent(double(i)/double(frd_all->elements.size()-1));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar->end();

  progressbar->start(0,100,"Writing Results to ParaView Format - Linking Nodesets");
  t_start = std::chrono::high_resolution_clock::now();             

  //nodesets
  for (size_t i = 0; i < nodeset_ids.size(); i++)
  {
    ++current_part;
    for (size_t ii = 0; ii < nodeset_node_ids[i].size(); ii++)
    {
      vec_frd[current_part]->elements.push_back({nodeset_node_ids[i][ii],99,int(ii),0});
      vec_frd[current_part]->elements_connectivity.push_back({nodeset_node_ids[i][ii]});
      vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
    }
    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //currentDataRow += frd->result_block_data[data_ids[ii]].size();
    if (duration > 500)
    {
      progressbar->percent(double(i)/double(frd->elements.size()-1));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar->end();

  progressbar->start(0,100,"Writing Results to ParaView Format - Linking Sidesets");
  t_start = std::chrono::high_resolution_clock::now();             

  //std::string log;
  //log = "geht " + std::to_string(i) + " \n";
  //ccx_iface->log_str(log);

  //sidesets
  for (size_t i = 0; i < sideset_ids.size(); i++)
  {
    ++current_part;
    for (size_t ii = 0; ii < sideset_elements_connectivity[i].size(); ii++)
    {
      if (sideset_elements_connectivity[i][ii].size()==2) // line
      {
        vec_frd[current_part]->elements.push_back({sideset_node_ids[i][ii],11,int(ii),0});  
        vec_frd[current_part]->elements_connectivity.push_back(sideset_elements_connectivity[i][ii]);
        vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
      }else if (sideset_elements_connectivity[i][ii].size()==3) // line
      {
        if (((sideset_elements_type[i][ii]>=28) && (sideset_elements_type[i][ii]<=33))||((sideset_elements_type[i][ii]>=11) && (sideset_elements_type[i][ii]<=18))||((sideset_elements_type[i][ii]>=48) && (sideset_elements_type[i][ii]<=50))) // triangle
        {
          vec_frd[current_part]->elements.push_back({sideset_node_ids[i][ii],12,int(ii),0});
        }else{ // beam
          vec_frd[current_part]->elements.push_back({sideset_node_ids[i][ii],7,int(ii),0});
        }
        vec_frd[current_part]->elements_connectivity.push_back(sideset_elements_connectivity[i][ii]);
        vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
      }else if (sideset_elements_connectivity[i][ii].size()==4) // quad
      {
        vec_frd[current_part]->elements.push_back({sideset_node_ids[i][ii],9,int(ii),0});  
        vec_frd[current_part]->elements_connectivity.push_back(sideset_elements_connectivity[i][ii]);
        vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
      }else if (sideset_elements_connectivity[i][ii].size()==6) // quadratic triangle
      {
        vec_frd[current_part]->elements.push_back({sideset_node_ids[i][ii],8,int(ii),0});  
        vec_frd[current_part]->elements_connectivity.push_back(sideset_elements_connectivity[i][ii]);
        vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
      }else if (sideset_elements_connectivity[i][ii].size()==8) // quadratic quad
      {
        vec_frd[current_part]->elements.push_back({sideset_node_ids[i][ii],10,int(ii),0});  
        vec_frd[current_part]->elements_connectivity.push_back(sideset_elements_connectivity[i][ii]);
        vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
      }
    }

    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //currentDataRow += frd->result_block_data[data_ids[ii]].size();
    if (duration > 500)
    {
      progressbar->percent(double(i)/double(frd->elements.size()-1));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar->end();

  return true;
}

bool CoreResultsVtkWriter::link_dat()
{
  std::vector<std::vector<int>> ip_nodes;
  //ip_nodes[0][0] ip number
  //ip_nodes[0][0] element id
  std::vector<std::vector<double>> ip_nodes_coords;
  
  element_id_type_connectivity = ccx_iface->get_element_id_type_connectivity();
  std::vector<std::vector<int>> tmp_element_id_type_connectivity;

  progressbar->start(0,100,"Writing Results to ParaView Format - Linking .dat: Nodes and Integrationpoints");
  auto t_start = std::chrono::high_resolution_clock::now();

  //get nodes and elements
  current_part = nparts - nparts_dat - 1;
  for (size_t i = 0 ; i < nparts_dat; i++)
  {
    ++current_part;
    ip_nodes.clear();
    ip_nodes_coords.clear();
    for (size_t ii = 0; ii < dat_all->result_blocks.size(); ii++)
    {
      if (dat_all->result_blocks[ii][3]==i)
      {
        // create nodes
        if (dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]][0][2] == 1) // nodeset
        {
          for (size_t iii = 0; iii < dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]].size(); iii++)
          {
            vec_frd[current_part]->nodes.push_back({dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]][iii][0],0});
            std::array<double,3> node_coords = CubitInterface::get_nodal_coordinates(dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]][iii][0]);
            vec_frd[current_part]->nodes_coords.push_back({node_coords[0],node_coords[1],node_coords[2]});
            vec_frd[current_part]->nodes[vec_frd[current_part]->nodes.size()-1][1] = vec_frd[current_part]->nodes_coords.size()-1;
          }
        }else{  // element set
          int ip_max = 0;
          int ip = 0;
          int ip_last = 0;
          int element_id = 0;
          int element_id_last = 0;
          bool flush_data = false;
          bool only_one_ip = true;
          std::vector<std::vector<int>> tmp_ip_nodes;
          std::vector<std::vector<double>> tmp_ip_nodes_coords;
          tmp_ip_nodes.clear();
          tmp_ip_nodes_coords.clear();
          tmp_element_id_type_connectivity = element_id_type_connectivity;

          element_id_last = int(dat_all->result_block_data[dat_all->result_blocks[ii][4]][dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]][0][1]][0]);

          for (size_t iii = 0; iii < dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]].size(); iii++)
          {
            ip = int(dat_all->result_block_data[dat_all->result_blocks[ii][4]][dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]][iii][1]][1]);
            if (ip!=1)
            {
              only_one_ip = false;
            }
          }

          for (size_t iii = 0; iii < dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]].size(); iii++)
          {
            ip = int(dat_all->result_block_data[dat_all->result_blocks[ii][4]][dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]][iii][1]][1]);
            element_id = int(dat_all->result_block_data[dat_all->result_blocks[ii][4]][dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]][iii][1]][0]);

            //std::string log;
            //log = " iteration " + std::to_string(iii) + " element_id " + std::to_string(element_id) + " element_id_last " + std::to_string(element_id_last) + " ip " + std::to_string(ip) + " ip_last " + std::to_string(ip_last) + " \n";
            //ccx_iface->log_str(log);
            //PRINT_INFO("%s", log.c_str());

            if (!only_one_ip)
            {
              if (ip_last == ip)
              {
                ip_max = ip;
                flush_data = true; // only 1 ip point
              }else if (ip < ip_last) // element changed
              {
                //ip_max = ip;
                flush_data = true;
              }else if (iii == (dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]].size()-1)) // last one
              {
                flush_data = true;
                ip_max = ip;
                ip_nodes.push_back({ip,element_id});
                tmp_ip_nodes.push_back({ip,element_id});
                tmp_ip_nodes_coords.push_back({0,0,0});
                ip_last = ip;
              }else{
                ip_max = ip;
              }
            }else{
              ip = 1;
              ip_max = 1;
              ip_nodes.push_back({ip,element_id});
              tmp_ip_nodes.push_back({ip,element_id});
              tmp_ip_nodes_coords.push_back({0,0,0});
            }
            
            if ((flush_data)||only_one_ip)
            {
              //get element type and connectivity
              std::vector<int> element_type_connectivity;

              for (size_t v = 0; v < tmp_element_id_type_connectivity.size(); v++)
              {
                if (tmp_element_id_type_connectivity[v][0]==tmp_ip_nodes[0][1])
                {
                  for (size_t vi = 1; vi < tmp_element_id_type_connectivity[v].size(); vi++)
                  {
                    element_type_connectivity.push_back(tmp_element_id_type_connectivity[v][vi]);
                  }
                  tmp_element_id_type_connectivity.erase(tmp_element_id_type_connectivity.begin() + v);
                }
              }
              
              std::vector<std::vector<double>> nodes_coords;
              for (size_t v = 1; v < element_type_connectivity.size(); v++)
              {
                std::array<double,3> node_coords = CubitInterface::get_nodal_coordinates(element_type_connectivity[v]);
                nodes_coords.push_back({node_coords[0],node_coords[1],node_coords[2]});
              }

              // element changed, flush current element data, reset and one step back              
              for (size_t iv = 0; iv < tmp_ip_nodes.size(); iv++)
              {
                //std::string log;
                //log = " inner element_id " + std::to_string(tmp_ip_nodes[iv][1]) + " ip " + std::to_string(tmp_ip_nodes[iv][0]) + " ";
                //ccx_iface->log_str(log);
                //PRINT_INFO("%s", log.c_str());
                tmp_ip_nodes_coords[iv] = this->get_integration_point_coordinates(element_type_connectivity[0], tmp_ip_nodes[iv][0], ip_max, nodes_coords);
                ip_nodes_coords.push_back(tmp_ip_nodes_coords[iv]);
                vec_frd[current_part]->nodes.push_back({tmp_ip_nodes[iv][0],0});
                vec_frd[current_part]->nodes_coords.push_back(tmp_ip_nodes_coords[iv]);
                vec_frd[current_part]->nodes[vec_frd[current_part]->nodes.size()-1][1] = vec_frd[current_part]->nodes_coords.size()-1;
              }
              //reset
              ip_max = 0;
              ip_last = 0;
              tmp_ip_nodes.clear();
              tmp_ip_nodes_coords.clear();
              flush_data = false;
            }
            if (iii == (dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]].size()-1)) // last one
            {
            }else{
              if (!only_one_ip)
              {
                ip_nodes.push_back({ip,element_id});
                tmp_ip_nodes.push_back({ip,element_id});
                tmp_ip_nodes_coords.push_back({0,0,0});
                ip_last = ip;
                element_id_last = element_id;
                ip_max = ip;
              }
            }
          }
        }
      
        // elements
        if (dat_all->result_block_c1_data[dat_all->result_blocks[ii][4]][0][2] == 1) // nodeset
        {
          for (size_t iii = 0; iii < vec_frd[current_part]->nodes.size(); iii++)
          {
            vec_frd[current_part]->elements.push_back({vec_frd[current_part]->nodes[iii][0],99,int(iii),0});
            //vec_frd[current_part]->elements_connectivity.push_back({vec_frd[current_part]->nodes[iii][0]});
            vec_frd[current_part]->elements_connectivity.push_back({int(iii)});
            vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
          }
        }else{       // element set
          for (size_t iii = 0; iii < vec_frd[current_part]->nodes.size(); iii++)
          {
            vec_frd[current_part]->elements.push_back({ip_nodes[iii][1],99,int(iii),0});
            vec_frd[current_part]->elements_connectivity.push_back({int(iii)});
            vec_frd[current_part]->elements[vec_frd[current_part]->elements.size()-1][2] = vec_frd[current_part]->elements_connectivity.size()-1;
          }
        }
        // to skip rest and go for the next set
        ii = dat_all->result_blocks.size();
      }
    }
    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //currentDataRow += frd->result_block_data[data_ids[ii]].size();
    if (duration > 500)
    {
      progressbar->percent(double(i)/double(nparts_dat));
      progressbar->check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar->end();

  progressbar->start(0,100,"Writing Results to ParaView Format - Linking .dat: processing result data");
  t_start = std::chrono::high_resolution_clock::now();

  //get the data for the sets
  current_part = nparts - nparts_dat - 1;
  for (size_t i = 0 ; i < nparts_dat; i++)
  {
    ++current_part;
    current_increment = 0;
    for (size_t ii = 0; ii < max_increments; ii++)
    {
      ++current_increment;
      std::vector<int> data_ids = this->get_dat_result_blocks_data_ids_linked(i);

      for (size_t iii = 0; iii < data_ids.size(); iii++)
      { 
        //std::string log;
        //log = " part i " + std::to_string(i) + " increment ii " + std::to_string(ii) + " max_increment " + std::to_string(max_increments) + " - " + std::to_string(this->get_max_step_increment()) + " data_ids iii " + std::to_string(iii) + " data_ids " + std::to_string(data_ids[iii]) + " data_ids.size()-1 " + std::to_string(data_ids.size()-1) + " ";
        //ccx_iface->log_str(log);
        //PRINT_INFO("%s", log.c_str());
        std::vector<int> tmp_dummy = this->get_currentincrement_result_blocks_data();
        //insert the dummies
        std::vector<int> tmp_result_blocks(7);
        vec_frd[current_part]->result_blocks.push_back(tmp_result_blocks);
        // result_blocks[0][0] result block
        vec_frd[current_part]->result_blocks[vec_frd[current_part]->result_blocks.size()-1][0] = vec_frd[current_part]->result_blocks.size()-1;
        // result_blocks[0][1] step
        // result_blocks[0][2] step increment
        // result_blocks[0][3] total increment
        vec_frd[current_part]->result_blocks[vec_frd[current_part]->result_blocks.size()-1][1] = tmp_dummy[0];
        vec_frd[current_part]->result_blocks[vec_frd[current_part]->result_blocks.size()-1][2] = tmp_dummy[1];
        vec_frd[current_part]->result_blocks[vec_frd[current_part]->result_blocks.size()-1][3] = tmp_dummy[2];
        // result_blocks[0][4] total time data id
        vec_frd[current_part]->total_times.push_back(dat_all->total_times[dat_all->result_blocks[data_ids[iii]][1]]);
        vec_frd[current_part]->result_blocks[vec_frd[current_part]->result_blocks.size()-1][4] = vec_frd[current_part]->total_times.size()-1;
        // result_blocks[0][5] result block type
        if (iii == 0)
        {
          for (size_t iv = 0; iv < dat_all->result_block_type.size(); iv++)
          {
            vec_frd[current_part]->result_block_type.push_back(".dat: " + dat_all->result_block_type[iv]);
          }
        }
        vec_frd[current_part]->result_blocks[vec_frd[current_part]->result_blocks.size()-1][5] = dat_all->result_blocks[data_ids[iii]][2];
        //std::vector<std::vector<std::string>> result_block_components;
        std::vector<std::string> tmp_comp_data;
        for (size_t iv = 0; iv < dat_all->result_block_components[data_ids[iii]].size(); iv++)
        { 
          if (dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[iii]][4]][0][2] == 1) // nodeset
          { 
            tmp_comp_data.push_back(dat_all->result_block_components[data_ids[iii]][iv]);
          }else{
            if (iv > 1)
            {
              tmp_comp_data.push_back(dat_all->result_block_components[data_ids[iii]][iv]);
            }
          }
        }
        vec_frd[current_part]->result_block_components.push_back(tmp_comp_data);
        // result_blocks[0][6] result block data id
        std::vector<std::vector<double>> tmp_result_block_data;
        std::vector<std::vector<int>> tmp_result_block_node_data;
        vec_frd[current_part]->result_block_data.push_back(tmp_result_block_data);
        vec_frd[current_part]->result_block_node_data.push_back(tmp_result_block_node_data);
        vec_frd[current_part]->result_blocks[vec_frd[current_part]->result_blocks.size()-1][6] = vec_frd[current_part]->result_block_data.size()-1;
        // nodes data
        if (dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[iii]][4]][0][2] == 1) // nodeset
        {
          for (size_t iv = 0; iv < dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[iii]][4]].size(); iv++)
          {
            std::vector<double> tmp_node_data;
            for (size_t v = 0; v < dat_all->result_block_data[data_ids[iii]][dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[iii]][4]][iv][1]].size(); v++)
            {
              tmp_node_data.push_back(dat_all->result_block_data[data_ids[iii]][dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[iii]][4]][iv][1]][v]);
            }
            vec_frd[current_part]->result_block_data[vec_frd[current_part]->result_block_data.size()-1].push_back(tmp_node_data);
            vec_frd[current_part]->result_block_node_data[vec_frd[current_part]->result_block_data.size()-1].push_back({int(iv),int(iv)});
          }
        }else{  // elements data
          for (size_t iv = 0; iv < dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[iii]][4]].size(); iv++)
          {
            std::vector<double> tmp_node_data;
            for (size_t v = 2; v < dat_all->result_block_data[data_ids[iii]][dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[iii]][4]][iv][1]].size(); v++)
            {
              tmp_node_data.push_back(dat_all->result_block_data[data_ids[iii]][dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[iii]][4]][iv][1]][v]);
            }
            vec_frd[current_part]->result_block_data[vec_frd[current_part]->result_block_data.size()-1].push_back(tmp_node_data);
            vec_frd[current_part]->result_block_node_data[vec_frd[current_part]->result_block_data.size()-1].push_back({int(iv),int(iv)});
          }
        }
      }
      
      // add displacement result block for elements
      bool create_disp = false;
      // check if was element results and if for the block there is displacement data
      if ((dat_all->result_block_c1_data[dat_all->result_blocks[data_ids[data_ids.size()-1]][4]][0][2] == 2)&&(this->checkFrdBlockDispExists(dat_all->result_block_set[i])))
      {
        create_disp = true;        
      }else{
        create_disp = false;
      }

      if (create_disp)       
      {
        int current_result_block_data_id;
        //inser dummies
        std::vector<int> tmp_dummy = this->get_currentincrement_result_blocks_data();
        //insert the dummies
        std::vector<int> tmp_result_blocks(7);
        vec_frd[current_part]->result_blocks.push_back(tmp_result_blocks);
        // result_blocks[0][0] result block
        vec_frd[current_part]->result_blocks[vec_frd[current_part]->result_blocks.size()-1][0] = vec_frd[current_part]->result_blocks.size()-1;
        current_result_block_data_id = vec_frd[current_part]->result_blocks.size()-1;
        // result_blocks[0][1] step
        // result_blocks[0][2] step increment
        // result_blocks[0][3] total increment
        vec_frd[current_part]->result_blocks[current_result_block_data_id][1] = tmp_dummy[0];
        vec_frd[current_part]->result_blocks[current_result_block_data_id][2] = tmp_dummy[1];
        vec_frd[current_part]->result_blocks[current_result_block_data_id][3] = tmp_dummy[2];
        // result_blocks[0][4] total time data id
        vec_frd[current_part]->total_times.push_back(dat_all->total_times[dat_all->result_blocks[data_ids[data_ids.size()-1]][1]]);
        vec_frd[current_part]->result_blocks[current_result_block_data_id][4] = vec_frd[current_part]->total_times.size()-1;
        // result_blocks[0][5] result block type
        vec_frd[current_part]->result_block_type.push_back(".dat: disp. int. pts.");
        vec_frd[current_part]->result_blocks[current_result_block_data_id][5] = vec_frd[current_part]->result_block_type.size()-1;
        //std::vector<std::vector<std::string>> result_block_components;
        vec_frd[current_part]->result_block_components.push_back({"D1","D2","D3"});
        // result_blocks[0][6] result block data id
        std::vector<std::vector<double>> tmp_result_block_data;
        std::vector<std::vector<int>> tmp_result_block_node_data;
        vec_frd[current_part]->result_block_data.push_back(tmp_result_block_data);
        vec_frd[current_part]->result_block_node_data.push_back(tmp_result_block_node_data);
        vec_frd[current_part]->result_blocks[current_result_block_data_id][6] = vec_frd[current_part]->result_block_data.size()-1;
        // elements data
        std::vector<std::vector<double>> displacements;
        displacements = this->compute_integration_points_displacements(i);
        for (size_t iii = 0; iii < displacements.size(); iii++)
        {
          vec_frd[current_part]->result_block_data[vec_frd[current_part]->result_block_data.size()-1].push_back({displacements[iii][0],displacements[iii][1],displacements[iii][2]});
          vec_frd[current_part]->result_block_node_data[vec_frd[current_part]->result_block_data.size()-1].push_back({int(iii),int(iii)});
        }
      }
      //update progress bar
      const auto t_end = std::chrono::high_resolution_clock::now();
      int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
      //currentDataRow += frd->result_block_data[data_ids[ii]].size();
      if (duration > 500)
      {
        progressbar->percent(double(current_increment)/double(max_increments));
        progressbar->check_interrupt();
        t_start = std::chrono::high_resolution_clock::now();
      }  
    }
  }
  progressbar->end();

  return true;
}

bool CoreResultsVtkWriter::prepare_sidesets()
{
  // Get the list of sidesets
  std::vector<SidesetHandle> sidesets;
  me_iface->get_sideset_list(sidesets);

  // Get a batch of elements from the sideset
  int buf_size = 100;

  // loop over the sidesets
  for (size_t i = 0; i < sidesets.size(); i++)
  {

    SidesetHandle sideset = sidesets[i];

    sideset_ids.push_back(me_iface->id_from_handle(sideset));
    std::vector<int> tmp_sideset_node_ids;
    sideset_node_ids.push_back(tmp_sideset_node_ids);
    std::vector<int> tmp_sideset_elements;
    sideset_elements.push_back(tmp_sideset_elements);
    std::vector<int> tmp_sideset_elements_type;
    sideset_elements_type.push_back(tmp_sideset_elements_type);
    std::vector<std::vector<int>> tmp_sideset_elements_connectivity; 
    sideset_elements_connectivity.push_back(tmp_sideset_elements_connectivity);

    int num_elems;
    int start_index = 0;
    int element_count = 0;

    std::vector<ElementHandle> element_ids;
    std::vector<int> side_ids;
    std::vector<ElementType> element_type;
    std::string cubit_element_type_entity;

    // count all elements in sideset
    while( (num_elems = me_iface->get_sideset_sides(sideset,start_index,buf_size,element_ids,element_type,side_ids)) > 0)
    {
      start_index += num_elems;
      element_count += num_elems;
    }

    int element_id;
    std::vector<int> side_ids_all(element_count);
    std::vector<int> ccx_side_ids_all(element_count);
    std::vector<int> element_ids_all(element_count);
    std::vector<int> element_type_all(element_count);

    start_index = 0;
    while( (num_elems = me_iface->get_sideset_sides(sideset,start_index,buf_size,element_ids,element_type,side_ids)) > 0)
    {
      // get all elements in sideset
      for (int j = 0; j < num_elems; j++)
      {
        me_iface->get_element_id(element_ids[j],element_id);
        //element_ids_all[start_index + j] = element_id;
        element_type_all[start_index + j] = element_type[j];
        side_ids_all[start_index + j] = side_ids[j];
        ccx_side_ids_all[start_index + j] = ccx_iface->get_ccx_element_side(element_type[j], side_ids[j]);

        // convert ids from element_id to global_element_id
        cubit_element_type_entity = me_iface->get_element_type_name(element_type[j]);
        cubit_element_type_entity = ccx_iface->get_cubit_element_type_entity(cubit_element_type_entity);
        element_ids_all[start_index + j] = CubitInterface::get_global_element_id(cubit_element_type_entity,element_id);

        // prepare data for linking
        int num_node = 0;

        sideset_elements[i].push_back(element_id);
        sideset_elements_type[i].push_back(element_type[j]);
        
        me_iface->get_nodes_per_side(element_type[j],side_ids[j]-1,num_node);
        
        std::vector<int> indices(num_node);
        me_iface->get_node_indices_per_side(element_type[j],side_ids[j]-1,indices);

        int nodes_per_element;
        me_iface->get_nodes_per_element(element_type[j],nodes_per_element);
        std::vector<int> conn(nodes_per_element);        
        me_iface->get_connectivity(element_ids[j], conn);
        
        std::vector<int> tmp_connectivity; 

        for (size_t k = 0; k < indices.size(); k++)
        {
          tmp_connectivity.push_back(conn[indices[k]]); 
        }
        sideset_elements_connectivity[i].push_back(tmp_connectivity);
      }
    start_index += num_elems;
    }
    // filter out all nodes
    std::vector<int>::iterator it;
    for (size_t ii = 0; ii < sideset_elements_connectivity[i].size(); ii++)
    {
      for (size_t iii = 0; iii < sideset_elements_connectivity[i][ii].size(); iii++)
      {       
        it = std::find(sideset_node_ids[i].begin(), sideset_node_ids[i].end(), sideset_elements_connectivity[i][ii][iii]);
        if (it==sideset_node_ids[i].end())
        {  
          sideset_node_ids[i].push_back(sideset_elements_connectivity[i][ii][iii]);
        }
      }
    }
  }
 
  return true;
}

std::vector<double> CoreResultsVtkWriter::get_integration_point_coordinates(int element_type, int ip, int ipmax, std::vector<std::vector<double>> nodes_coords)
{
  std::vector<double> ip_coords(3);
  ip_coords[0] = 0;
  ip_coords[1] = 0;
  ip_coords[2] = 0;
  std::vector<std::vector<double>> ip_iso_coords;
  std::vector<double> shape_functions;
  double xi = 0;
  double eta = 0;
  double zeta = 0;

  //std::string log;
  //log = "get_integration_point_coordinates element_type " + std::to_string(element_type) + " ip " + std::to_string(ip) + " ipmax " + std::to_string(ipmax) + " \n";
  //ccx_iface->log_str(log);
  //PRINT_INFO("%s", log.c_str());
  
  //get integration point in iso space

  if ((element_type>=23) && (element_type<=27)) //quad 
  { 
    if (ipmax == 1)
    {
      ip_iso_coords.push_back({0,0,0});
    }else if (ipmax == 4)
    {
      ip_iso_coords.push_back({-0.577350269189626,-0.577350269189626,0});
      ip_iso_coords.push_back({0.577350269189626,-0.577350269189626,0});
      ip_iso_coords.push_back({-0.577350269189626,0.577350269189626,0});
      ip_iso_coords.push_back({0.577350269189626,0.577350269189626,0});
    }else if (ipmax == 9)
    {
      ip_iso_coords.push_back({-0.774596669241483,-0.774596669241483,0});
      ip_iso_coords.push_back({-0,-0.774596669241483,0});
      ip_iso_coords.push_back({0.774596669241483,-0.774596669241483,0});
      ip_iso_coords.push_back({-0.774596669241483,0,0});
      ip_iso_coords.push_back({-0,0,0});
      ip_iso_coords.push_back({0.774596669241483,0,0});
      ip_iso_coords.push_back({-0.774596669241483,0.774596669241483,0});
      ip_iso_coords.push_back({-0,0.774596669241483,0});
      ip_iso_coords.push_back({0.774596669241483,0.774596669241483,0});
    }else if (ipmax == 8)
    {
      ip_iso_coords.push_back({-0.577350269189626,-0.577350269189626,-0.577350269189626});
      ip_iso_coords.push_back({0.577350269189626,-0.577350269189626,-0.577350269189626});
      ip_iso_coords.push_back({-0.577350269189626,0.577350269189626,-0.577350269189626});
      ip_iso_coords.push_back({0.577350269189626,0.577350269189626,-0.577350269189626});
      ip_iso_coords.push_back({-0.577350269189626,-0.577350269189626,0.577350269189626});
      ip_iso_coords.push_back({0.577350269189626,-0.577350269189626,0.577350269189626});
      ip_iso_coords.push_back({-0.577350269189626,0.577350269189626,0.577350269189626});
      ip_iso_coords.push_back({0.577350269189626,0.577350269189626,0.577350269189626});
    }else if (ipmax == 27)
    {
      ip_iso_coords.push_back({-0.774596669241483,-0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({0,-0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,-0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,0,-0.774596669241483});
      ip_iso_coords.push_back({0,0,-0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,0,-0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({0,0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,-0.774596669241483,0});
      ip_iso_coords.push_back({0,-0.774596669241483,0});
      ip_iso_coords.push_back({0.774596669241483,-0.774596669241483,0});
      ip_iso_coords.push_back({-0.774596669241483,0,0});
      ip_iso_coords.push_back({0,0,0});
      ip_iso_coords.push_back({0.774596669241483,0,0});
      ip_iso_coords.push_back({-0.774596669241483,0.774596669241483,0});
      ip_iso_coords.push_back({0,0.774596669241483,0});
      ip_iso_coords.push_back({0.774596669241483,0.774596669241483,0});
      ip_iso_coords.push_back({-0.774596669241483,-0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({0,-0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,-0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,0,0.774596669241483});
      ip_iso_coords.push_back({0,0,0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,0,0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({0,0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,0.774596669241483,0.774596669241483});
    }
  }else if ((element_type>=11) && (element_type<=14)) // triangle
  {
    if (ipmax == 1)
    {
      ip_iso_coords.push_back({0.333333333333333,0.333333333333333,0});
    }else if (ipmax == 3)
    {
      ip_iso_coords.push_back({0.166666666666667,0.166666666666667,0});
      ip_iso_coords.push_back({0.666666666666667,0.166666666666667,0});
      ip_iso_coords.push_back({0.166666666666667,0.666666666666667,0});
    }else if (ipmax == 7)
    {
      ip_iso_coords.push_back({0.333333333333333,0.333333333333333,0});
      ip_iso_coords.push_back({0.797426985353087,0.101286507323456,0});
      ip_iso_coords.push_back({0.101286507323456,0.797426985353087,0});
      ip_iso_coords.push_back({0.101286507323456,0.101286507323456,0});
      ip_iso_coords.push_back({0.470142064105115,0.059715871789770,0});
      ip_iso_coords.push_back({0.059715871789770,0.470142064105115,0});
      ip_iso_coords.push_back({0.470142064105115,0.470142064105115,0});
    }else if (ipmax == 2)
    {
      ip_iso_coords.push_back({0.333333333333333,0.333333333333333,-0.577350269189626});
      ip_iso_coords.push_back({0.333333333333333,0.333333333333333,0.577350269189626});
    }else if (ipmax == 9)
    {
      ip_iso_coords.push_back({0.166666666666667,0.166666666666667,-0.774596669241483});
      ip_iso_coords.push_back({0.666666666666667,0.166666666666667,-0.774596669241483});
      ip_iso_coords.push_back({0.166666666666667,0.666666666666667,-0.774596669241483});
      ip_iso_coords.push_back({0.166666666666667,0.166666666666667,0});
      ip_iso_coords.push_back({0.666666666666667,0.166666666666667,0});
      ip_iso_coords.push_back({0.166666666666667,0.666666666666667,0});
      ip_iso_coords.push_back({0.166666666666667,0.166666666666667,0.774596669241483});
      ip_iso_coords.push_back({0.666666666666667,0.166666666666667,0.774596669241483});
      ip_iso_coords.push_back({0.166666666666667,0.666666666666667,0.774596669241483});
    }
  }else if ((element_type>=39) && (element_type<=43)) // hex
  {
    if (ipmax == 1)
    {
      ip_iso_coords.push_back({0,0,0});
    }else if (ipmax == 8)
    {
      ip_iso_coords.push_back({-0.577350269189626,-0.577350269189626,-0.577350269189626});
      ip_iso_coords.push_back({0.577350269189626,-0.577350269189626,-0.577350269189626});
      ip_iso_coords.push_back({-0.577350269189626,0.577350269189626,-0.577350269189626});
      ip_iso_coords.push_back({0.577350269189626,0.577350269189626,-0.577350269189626});
      ip_iso_coords.push_back({-0.577350269189626,-0.577350269189626,0.577350269189626});
      ip_iso_coords.push_back({0.577350269189626,-0.577350269189626,0.577350269189626});
      ip_iso_coords.push_back({-0.577350269189626,0.577350269189626,0.577350269189626});
      ip_iso_coords.push_back({0.577350269189626,0.577350269189626,0.577350269189626});
    }else if (ipmax == 27)
    {
      ip_iso_coords.push_back({-0.774596669241483,-0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({0,-0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,-0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,0,-0.774596669241483});
      ip_iso_coords.push_back({0,0,-0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,0,-0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({0,0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,0.774596669241483,-0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,-0.774596669241483,0});
      ip_iso_coords.push_back({0,-0.774596669241483,0});
      ip_iso_coords.push_back({0.774596669241483,-0.774596669241483,0});
      ip_iso_coords.push_back({-0.774596669241483,0,0});
      ip_iso_coords.push_back({0,0,0});
      ip_iso_coords.push_back({0.774596669241483,0,0});
      ip_iso_coords.push_back({-0.774596669241483,0.774596669241483,0});
      ip_iso_coords.push_back({0,0.774596669241483,0});
      ip_iso_coords.push_back({0.774596669241483,0.774596669241483,0});
      ip_iso_coords.push_back({-0.774596669241483,-0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({0,-0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,-0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,0,0.774596669241483});
      ip_iso_coords.push_back({0,0,0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,0,0.774596669241483});
      ip_iso_coords.push_back({-0.774596669241483,0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({0,0.774596669241483,0.774596669241483});
      ip_iso_coords.push_back({0.774596669241483,0.774596669241483,0.774596669241483});
    }
  }else if ((element_type>=28) && (element_type<=33)) // tet
  {
    if (ipmax == 1)
    {
      ip_iso_coords.push_back({0.25,0.25,0.25});
    }else if (ipmax == 4)
    {
      ip_iso_coords.push_back({0.138196601125011,0.138196601125011,0.138196601125011});
      ip_iso_coords.push_back({0.585410196624968,0.138196601125011,0.138196601125011});
      ip_iso_coords.push_back({0.138196601125011,0.585410196624968,0.138196601125011});
      ip_iso_coords.push_back({0.138196601125011,0.138196601125011,0.585410196624968});
    }else if (ipmax == 15)
    {
      ip_iso_coords.push_back({0.25,0.25,0.25});
      ip_iso_coords.push_back({0.091971078052723,0.091971078052723,0.091971078052723});
      ip_iso_coords.push_back({0.724086765841831,0.091971078052723,0.091971078052723});
      ip_iso_coords.push_back({0.091971078052723,0.724086765841831,0.091971078052723});
      ip_iso_coords.push_back({0.091971078052723,0.091971078052723,0.724086765841831});
      ip_iso_coords.push_back({0.319793627829630,0.319793627829630,0.319793627829630});
      ip_iso_coords.push_back({0.040619116511110,0.319793627829630,0.319793627829630});
      ip_iso_coords.push_back({0.319793627829630,0.040619116511110,0.319793627829630});
      ip_iso_coords.push_back({0.319793627829630,0.319793627829630,0.040619116511110});
      ip_iso_coords.push_back({0.056350832689629,0.056350832689629,0.443649167310371});
      ip_iso_coords.push_back({0.443649167310371,0.056350832689629,0.056350832689629});
      ip_iso_coords.push_back({0.443649167310371,0.443649167310371,0.056350832689629});
      ip_iso_coords.push_back({0.056350832689629,0.443649167310371,0.443649167310371});
      ip_iso_coords.push_back({0.056350832689629,0.443649167310371,0.056350832689629});
      ip_iso_coords.push_back({0.443649167310371,0.056350832689629,0.443649167310371});  
    }
  }else if ((element_type>=48) && (element_type<=50)) // wedge
  {
    if (ipmax == 2)
    {
      ip_iso_coords.push_back({0.333333333333333,0.333333333333333,-0.577350269189626});
      ip_iso_coords.push_back({0.333333333333333,0.333333333333333,0.577350269189626});
    }else if (ipmax == 9)
    {
      ip_iso_coords.push_back({0.166666666666667,0.166666666666667,-0.774596669241483});
      ip_iso_coords.push_back({0.666666666666667,0.166666666666667,-0.774596669241483});
      ip_iso_coords.push_back({0.166666666666667,0.666666666666667,-0.774596669241483});
      ip_iso_coords.push_back({0.166666666666667,0.166666666666667,0});
      ip_iso_coords.push_back({0.666666666666667,0.166666666666667,0});
      ip_iso_coords.push_back({0.166666666666667,0.666666666666667,0});
      ip_iso_coords.push_back({0.166666666666667,0.166666666666667,0.774596669241483});
      ip_iso_coords.push_back({0.666666666666667,0.166666666666667,0.774596669241483});
      ip_iso_coords.push_back({0.166666666666667,0.666666666666667,0.774596669241483});
    }
  }else{
    for (size_t i = 0; i < ip; i++)
    {
      ip_iso_coords.push_back({0,0,0});
    }
  }

  xi = ip_iso_coords[ip-1][0];
  eta = ip_iso_coords[ip-1][1];
  zeta = ip_iso_coords[ip-1][2];
  
  /*
  if ((element_type>=39) && (element_type<=43)) {
    // HEX
  } else if ((element_type>=28) && (element_type<=33)) {
    // TETRA
  } else if ((element_type>=11) && (element_type<=14)) {
    // TRI
  } else if ((element_type>=15) && (element_type<=18)) {
    // TRISHELL
  } else if ((element_type>=19) && (element_type<=22)) {
    // SHELL
  } else if ((element_type>=23) && (element_type<=27)) {
    // QUAD
  } else if ((element_type>=48) && (element_type<=50)) {
    // WEDGE
  }
  */

  //compute shape functions IMPORTANT FROM CCX SOURCE CODE YOU HAVE TO MATCH THE NODE ORDER LATER!!!
  if ((element_type == 11)||(element_type == 12)) // tri linear
  {
    shape_functions.push_back(1-xi-eta);
    shape_functions.push_back(xi);
    shape_functions.push_back(eta);
  }else if ((element_type == 23)||(element_type == 24)) // quad linear
  {
    shape_functions.push_back((1-xi)*(1-eta)/4);
    shape_functions.push_back((1+xi)*(1-eta)/4);
    shape_functions.push_back((1+xi)*(1+eta)/4);
    shape_functions.push_back((1-xi)*(1+eta)/4);
  }else if ((element_type == 28)||(element_type == 29)) // tet linear
  {
    shape_functions.push_back(1-xi-eta-zeta);
    shape_functions.push_back(xi);
    shape_functions.push_back(eta);
    shape_functions.push_back(zeta);
  }else if (element_type == 13) // tri quadratic
  {
    shape_functions.push_back(2*(0.5-xi-eta)*(1-xi-eta));
    shape_functions.push_back(xi*(2*xi-1));
    shape_functions.push_back(eta*(2*eta-1));
    shape_functions.push_back(4*xi*(1-xi-eta));
    shape_functions.push_back(4*xi*eta);
    shape_functions.push_back(4*eta*(1-xi-eta));
  }else if ((element_type == 48)||(element_type == 49)) // wedge linear
  {
    shape_functions.push_back(0.5*(1-xi-eta)*(1-zeta));
    shape_functions.push_back(0.5*xi*(1-zeta));
    shape_functions.push_back(0.5*eta*(1-zeta));
    shape_functions.push_back(0.5*(1-xi-eta)*(1+zeta));
    shape_functions.push_back(0.5*xi*(1+zeta));
    shape_functions.push_back(0.5*eta*(1+zeta));
  }else if ((element_type == 39)||(element_type == 40)) // hex linear
  {
    shape_functions.push_back((1-xi)*(1-eta)*(1-zeta)/8);
    shape_functions.push_back((1+xi)*(1-eta)*(1-zeta)/8);
    shape_functions.push_back((1+xi)*(1+eta)*(1-zeta)/8);
    shape_functions.push_back((1-xi)*(1+eta)*(1-zeta)/8);
    shape_functions.push_back((1-xi)*(1-eta)*(1+zeta)/8);
    shape_functions.push_back((1+xi)*(1-eta)*(1+zeta)/8);
    shape_functions.push_back((1+xi)*(1+eta)*(1+zeta)/8);
    shape_functions.push_back((1-xi)*(1+eta)*(1+zeta)/8);
  }else if (element_type == 26) // quad quadratic
  {
    shape_functions.push_back((1-xi)*(1-eta)*(-xi-eta-1)/4);
    shape_functions.push_back((1+xi)*(1-eta)*(xi-eta-1)/4);
    shape_functions.push_back((1+xi)*(1+eta)*(xi+eta-1)/4);
    shape_functions.push_back((1-xi)*(1+eta)*(-xi+eta-1)/4);
    shape_functions.push_back((1+xi)*(1-xi)*(1-eta)/2);
    shape_functions.push_back((1+xi)*(1+eta)*(1-eta)/2);
    shape_functions.push_back((1+xi)*(1-xi)*(1+eta)/2);
    shape_functions.push_back((1-xi)*(1+eta)*(1-eta)/2);
  }else if (element_type == 31) // tet quadratic
  {
    shape_functions.push_back((2*(1-xi-eta-zeta)-1)*(1-xi-eta-zeta));
    shape_functions.push_back(xi*(2*xi-1));
    shape_functions.push_back(eta*(2*eta-1));
    shape_functions.push_back(zeta*(2*zeta-1));
    shape_functions.push_back(4*xi*(1-xi-eta-zeta));
    shape_functions.push_back(4*xi*eta);
    shape_functions.push_back(4*eta*(1-xi-eta-zeta));
    shape_functions.push_back(4*zeta*(1-xi-eta-zeta));
    shape_functions.push_back(4*xi*zeta);
    shape_functions.push_back(4*eta*zeta);
  }else if (element_type == 50) // wedge quadratic
  {
    shape_functions.push_back(0.5*(1-xi-eta)*(1-zeta)*(2*xi+2*eta+zeta));
    shape_functions.push_back(0.5*xi*(1-zeta)*(2*xi-2-zeta));
    shape_functions.push_back(0.5*eta*(1-zeta)*(2*eta-2-zeta));
    shape_functions.push_back(0.5*(1-xi-eta)*(1+zeta)*(2*xi+2*eta-zeta));
    shape_functions.push_back(0.5*xi*(1+zeta)*(2*xi-2+zeta));
    shape_functions.push_back(0.5*eta*(1+zeta)*(2*eta-2+zeta));
    shape_functions.push_back(2*xi*(1-xi-eta)*(1-zeta));
    shape_functions.push_back(2*xi*eta*(1-zeta));
    shape_functions.push_back(2*eta*(1-xi-eta)*(1-zeta));
    shape_functions.push_back(2*xi*(1-xi-eta)*(1+zeta));
    shape_functions.push_back(2*xi*eta*(1+zeta));
    shape_functions.push_back(2*eta*(1-xi-eta)*(1+zeta));
    shape_functions.push_back((1-xi-eta)*(1-zeta*zeta));
    shape_functions.push_back(xi*(1-zeta*zeta));
    shape_functions.push_back(eta*(1-zeta*zeta));
  }else if (element_type == 42) // hex quadratic
  {
    shape_functions.push_back(-(1-xi)*(1-eta)*(1-zeta)*((1+xi)+(1+eta)+zeta)/8);
    shape_functions.push_back(-(1+xi)*(1-eta)*(1-zeta)*((1-xi)+(1+eta)+zeta)/8);
    shape_functions.push_back(-(1+xi)*(1+eta)*(1-zeta)*((1-xi)+(1-eta)+zeta)/8);
    shape_functions.push_back(-(1-xi)*(1+eta)*(1-zeta)*((1+xi)+(1-eta)+zeta)/8);
    shape_functions.push_back(-(1-xi)*(1-eta)*(1+zeta)*((1+xi)+(1+eta)-zeta)/8);
    shape_functions.push_back(-(1+xi)*(1-eta)*(1+zeta)*((1-xi)+(1+eta)-zeta)/8);
    shape_functions.push_back(-(1+xi)*(1+eta)*(1+zeta)*((1-xi)+(1-eta)-zeta)/8);
    shape_functions.push_back(-(1-xi)*(1+eta)*(1+zeta)*((1+xi)+(1-eta)-zeta)/8);
    shape_functions.push_back((1-xi)*(1+xi)/4*(1-eta)*(1-zeta));
    shape_functions.push_back((1-eta)*(1+eta)/4*(1+xi)*(1-zeta));
    shape_functions.push_back((1-xi)*(1+xi)/4*(1+eta)*(1-zeta));
    shape_functions.push_back((1-eta)*(1+eta)/4*(1-xi)*(1-zeta));
    shape_functions.push_back((1-xi)*(1+xi)/4*(1-eta)*(1+zeta));
    shape_functions.push_back((1-eta)*(1+eta)/4*(1+xi)*(1+zeta));
    shape_functions.push_back((1-xi)*(1+xi)/4*(1+eta)*(1+zeta));
    shape_functions.push_back((1-eta)*(1+eta)/4*(1-xi)*(1+zeta));
    shape_functions.push_back((1-zeta)*(1+zeta)/4*(1-xi)*(1-eta));
    shape_functions.push_back((1-zeta)*(1+zeta)/4*(1+xi)*(1-eta));
    shape_functions.push_back((1-zeta)*(1+zeta)/4*(1+xi)*(1+eta));
    shape_functions.push_back((1-zeta)*(1+zeta)/4*(1-xi)*(1+eta));
  }

  // MATCHING NODE ORDER!!!
  std::vector<std::vector<double>> conn = nodes_coords;

  for (int j = 0; j < nodes_coords.size(); j++)
  {
    // different node numbering for hex20
    if (element_type == 42) {
      if (j >= 12 && j<=15) {
        nodes_coords[j] = conn[j+4];
      } else if (j >= 16 && j<=19) {
        nodes_coords[j] = conn[j-4];
      } else {
        nodes_coords[j] = conn[j];
      }
    } else if (element_type == 50) {  // different node numbering for wedge15
      if (j >= 9 && j<=11) {
        nodes_coords[j] = conn[j+3];
      } else if (j >= 12 && j<=14) {
        nodes_coords[j] = conn[j-3];
      } else {
        nodes_coords[j] = conn[j];
      }
    }
  }

  if ((shape_functions.size()!=0)&&(shape_functions.size()==nodes_coords.size()))
  {
    ip_coords[0] = 0;
    ip_coords[1] = 0;
    ip_coords[2] = 0;

    for (size_t i = 0; i < shape_functions.size(); i++)
    {
      ip_coords[0] += shape_functions[i]*nodes_coords[i][0];
      ip_coords[1] += shape_functions[i]*nodes_coords[i][1];
      ip_coords[2] += shape_functions[i]*nodes_coords[i][2];
    }
  }else{
    ip_coords[0] = 0;
    ip_coords[1] = 0;
    ip_coords[2] = 0;
  }
  
  return ip_coords;
}

std::vector<std::vector<double>> CoreResultsVtkWriter::compute_integration_points_displacements(int set_id)
{
  int block_id;
  std::vector<std::vector<int>> ip_nodes;
  //ip_nodes[0][0] ip number
  //ip_nodes[0][0] element id
  std::vector<std::vector<double>> ip_nodes_coords;
  std::vector<std::vector<double>> ip_nodes_coords_displaced;
  std::vector<std::vector<double>> displacements;
  std::vector<std::vector<double>> nodes_displacements;
  std::vector<std::vector<int>> tmp_element_id_type_connectivity;

  block_id = this->getFrdBlockId(dat_all->result_block_set[set_id]);
  // quit if no block is found...should not happen but hey
  if (block_id == -1)
  {
    return displacements;
  }
  
  frd = vec_frd[block_id];

  //first get all nodes coords and their displacements
  
  std::vector<int> data_ids = this->get_result_blocks_data_ids(); // get data ids for result blocks in current increment  
  for (size_t i = 0; i < data_ids.size(); i++)
  {
    std::vector<int> node_data_ids = this->get_result_block_node_data_id(data_ids[i]);
    // skip if nodes from point data is different than nodes number, like for data from CELS
    if (node_data_ids.size()==frd->nodes.size())
    {
      if (frd->result_block_type[frd->result_blocks[data_ids[i]][5]]=="DISP")
      {
        for (size_t ii = 0; ii < node_data_ids.size(); ii++)
        {
          std::vector<double> result_component;
          result_component = frd->result_block_data[data_ids[i]][node_data_ids[ii]];
          nodes_displacements.push_back(result_component);
        }
      }
    }
  }

  // get a result block for the elements and ips
  data_ids = this->get_dat_result_blocks_data_ids_linked(set_id);
  int data_id = data_ids[0];

  //compute the ip coords
  int ip_max = 0;
  int ip = 0;
  int ip_last = 0;
  int element_id = 0;
  int element_id_last = 0;
  bool flush_data = false;
  bool only_one_ip = true;
  std::vector<std::vector<int>> tmp_ip_nodes;
  std::vector<std::vector<double>> tmp_ip_nodes_coords;
  std::vector<std::vector<double>> tmp_ip_nodes_coords_displaced;
  tmp_ip_nodes.clear();
  tmp_ip_nodes_coords.clear();
  tmp_ip_nodes_coords_displaced.clear();
  tmp_element_id_type_connectivity = element_id_type_connectivity;

  element_id_last = int(dat_all->result_block_data[dat_all->result_blocks[data_id][4]][dat_all->result_block_c1_data[dat_all->result_blocks[data_id][4]][0][1]][0]);

  for (size_t i = 0; i < dat_all->result_block_c1_data[dat_all->result_blocks[data_id][4]].size(); i++)
  {
    ip = int(dat_all->result_block_data[dat_all->result_blocks[data_id][4]][dat_all->result_block_c1_data[dat_all->result_blocks[data_id][4]][i][1]][1]);
    if (ip!=1)
    {
      only_one_ip = false;
    }
  }

  for (size_t i = 0; i < dat_all->result_block_c1_data[dat_all->result_blocks[data_id][4]].size(); i++)
  {
    ip = int(dat_all->result_block_data[dat_all->result_blocks[data_id][4]][dat_all->result_block_c1_data[dat_all->result_blocks[data_id][4]][i][1]][1]);
    element_id = int(dat_all->result_block_data[dat_all->result_blocks[data_id][4]][dat_all->result_block_c1_data[dat_all->result_blocks[data_id][4]][i][1]][0]);

    //std::string log;
    //log = " iteration " + std::to_string(iii) + " element_id " + std::to_string(element_id) + " element_id_last " + std::to_string(element_id_last) + " ip " + std::to_string(ip) + " ip_last " + std::to_string(ip_last) + " \n";
    //ccx_iface->log_str(log);
    //PRINT_INFO("%s", log.c_str());

    if (!only_one_ip)
    {
      if (ip_last == ip)
      {
        ip_max = ip;
        flush_data = true; // only 1 ip point
      }else if (ip < ip_last) // element changed
      {
        //ip_max = ip;
        flush_data = true;
      }else if (i == (dat_all->result_block_c1_data[dat_all->result_blocks[data_id][4]].size()-1)) // last one
      {
        flush_data = true;
        ip_max = ip;
        ip_nodes.push_back({ip,element_id});
        tmp_ip_nodes.push_back({ip,element_id});
        tmp_ip_nodes_coords.push_back({0,0,0});
        tmp_ip_nodes_coords_displaced.push_back({0,0,0});
        ip_last = ip;
      }else{
        ip_max = ip;
      }
    }else{
      ip = 1;
      ip_max = 1;
      ip_nodes.push_back({ip,element_id});
      tmp_ip_nodes.push_back({ip,element_id});
      tmp_ip_nodes_coords.push_back({0,0,0});
      tmp_ip_nodes_coords_displaced.push_back({0,0,0});
    }
    
    if ((flush_data)||only_one_ip)
    {
      //get element type and connectivity
      std::vector<int> element_type_connectivity;

      for (size_t ii = 0; ii < tmp_element_id_type_connectivity.size(); ii++)
      {
        if (tmp_element_id_type_connectivity[ii][0]==tmp_ip_nodes[0][1])
        {
          for (size_t iii = 1; iii < tmp_element_id_type_connectivity[ii].size(); iii++)
          {
            element_type_connectivity.push_back(tmp_element_id_type_connectivity[ii][iii]);
          }
          tmp_element_id_type_connectivity.erase(tmp_element_id_type_connectivity.begin() + ii);
        }
      }
      
      std::vector<std::vector<double>> nodes_coords;
      std::vector<std::vector<double>> nodes_coords_displaced;
      for (size_t ii = 1; ii < element_type_connectivity.size(); ii++)
      {
        std::array<double,3> node_coords = CubitInterface::get_nodal_coordinates(element_type_connectivity[ii]);
        nodes_coords.push_back({node_coords[0],node_coords[1],node_coords[2]});
        nodes_coords_displaced.push_back({node_coords[0],node_coords[1],node_coords[2]});
        //connect with displacements
        for (size_t iii = 0; iii < frd->nodes.size(); iii++)
        {
          if (frd->nodes[iii][0]==element_type_connectivity[ii])
          {
            nodes_coords_displaced[nodes_coords_displaced.size()-1][0] += nodes_displacements[iii][0];
            nodes_coords_displaced[nodes_coords_displaced.size()-1][1] += nodes_displacements[iii][1];
            nodes_coords_displaced[nodes_coords_displaced.size()-1][2] += nodes_displacements[iii][2];
            iii = frd->nodes.size();
          }
        }
      }

      // element changed, flush current element data, reset and one step back              
      for (size_t ii = 0; ii < tmp_ip_nodes.size(); ii++)
      {
        //std::string log;
        //log = " inner element_id " + std::to_string(tmp_ip_nodes[iv][1]) + " ip " + std::to_string(tmp_ip_nodes[iv][0]) + " ";
        //ccx_iface->log_str(log);
        //PRINT_INFO("%s", log.c_str());
        tmp_ip_nodes_coords[ii] = this->get_integration_point_coordinates(element_type_connectivity[0], tmp_ip_nodes[ii][0], ip_max, nodes_coords);
        ip_nodes_coords.push_back(tmp_ip_nodes_coords[ii]);
        tmp_ip_nodes_coords_displaced[ii] = this->get_integration_point_coordinates(element_type_connectivity[0], tmp_ip_nodes[ii][0], ip_max, nodes_coords_displaced);
        ip_nodes_coords_displaced.push_back(tmp_ip_nodes_coords_displaced[ii]);
      }
      //reset
      ip_max = 0;
      ip_last = 0;
      tmp_ip_nodes.clear();
      tmp_ip_nodes_coords.clear();
      tmp_ip_nodes_coords_displaced.clear();
      flush_data = false;
    }
    if (i == (dat_all->result_block_c1_data[dat_all->result_blocks[data_id][4]].size()-1)) // last one
    {
    }else{
      if (!only_one_ip)
      {
        ip_nodes.push_back({ip,element_id});
        tmp_ip_nodes.push_back({ip,element_id});
        tmp_ip_nodes_coords.push_back({0,0,0});
        tmp_ip_nodes_coords_displaced.push_back({0,0,0});
        ip_last = ip;
        element_id_last = element_id;
        ip_max = ip;
      }
    }
  } 

  std::vector<double> tmp_disp(3);
  // compute actual displacements
  for (size_t i = 0; i < ip_nodes_coords.size(); i++)
  {
    tmp_disp[0] = ip_nodes_coords_displaced[i][0] - ip_nodes_coords[i][0];
    tmp_disp[1] = ip_nodes_coords_displaced[i][1] - ip_nodes_coords[i][1];
    tmp_disp[2] = ip_nodes_coords_displaced[i][2] - ip_nodes_coords[i][2];
    displacements.push_back(tmp_disp);
  }
  

  return displacements;
}