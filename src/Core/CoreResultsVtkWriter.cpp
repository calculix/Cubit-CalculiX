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
  part_names.clear();

  for (size_t i = 0; i < vec_frd.size(); i++)
  {
    delete vec_frd[i];
  }
  for (size_t i = 0; i < vec_dat.size(); i++)
  {
    delete vec_dat[i];
  } 

  vec_frd.clear();
  vec_dat.clear();

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
  if (this->checkLinkPossible())
  {
    this->write_linked();
  }else{
    this->write_vtu_unlinked();
  }

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
    part_names.push_back(ccx_iface->get_block_name(block_ids[i]));
  }
  nparts += block_ids.size();

  // nodesets
  nodeset_ids = CubitInterface::get_nodeset_id_list();
  for (size_t i = 0; i < nodeset_ids.size(); i++)
  {
    nodeset_node_ids.push_back(CubitInterface::get_nodeset_nodes_inclusive(nodeset_ids[i]));
    part_names.push_back(ccx_iface->get_nodeset_name(nodeset_ids[i]));
  }
  nparts += nodeset_ids.size();    

  // sidesets
  // prepare sidesets
  this->prepare_sidesets();

  for (size_t i = 0; i < sideset_ids.size(); i++)
  {
    part_names.push_back(ccx_iface->get_sideset_name(sideset_ids[i]));
  }
  nparts += sideset_ids.size();

  progressbar->start(0,100,"Writing Results to ParaView Format - Linked Mode");
  auto t_start = std::chrono::high_resolution_clock::now();

  // prepare frd and dat
  for (size_t i = 0; i < nparts; i++)
  {
    CoreResultsFrd* tmp_frd;
    CoreResultsDat* tmp_dat;

    tmp_frd = new CoreResultsFrd();
    tmp_frd->init(job_id);
    tmp_dat = new CoreResultsDat();
    tmp_dat->init(job_id);

    vec_frd.push_back(tmp_frd);
    vec_dat.push_back(tmp_dat);
  }

  //link nodes
  this->link_nodes();
  //link elements
  this->link_elements();

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
      dat = vec_dat[current_part];

      //std::string log;
      //log = "nparts " + std::to_string(nparts) + " block " + std::to_string(block_ids[ii])+ " block size" + std::to_string(block_ids.size())+" ci " + std::to_string(current_increment) + " \n";
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
  //std::string log;
  //log = "writing results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
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
  log = "writing results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  PRINT_INFO("%s", log.c_str());
  
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
    str_result.append(std::to_string(result_connectivity[i]-1)); // node numbers starts with id 0
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
  for (size_t i = 0; i < frd->nodes.size(); i++)
  {
    if (!CubitInterface::get_node_exists(frd->nodes[i][0]))
    {
      return false;
    }
  }
  for (size_t i = 0; i < frd->elements.size(); i++)
  {
    if (!CubitInterface::get_element_exists(frd->elements[i][0]))
    {
      return false;
    }
  }
  if (CubitInterface::get_node_count()!=frd->nodes.size())
  {
    return false;
  }
  if (CubitInterface::get_element_count()!=frd->elements.size())
  {
    return false;
  }
  
  return true;
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
  int current_part = 0;
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

  for (size_t i = 0; i < nparts; i++)
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
      for (size_t iii = 0; iii < nparts; iii++)
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
  int current_part = 0;
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
    for (size_t ii = 0; ii < sideset_node_ids[i].size(); ii++)
    {
      vec_frd[current_part]->elements.push_back({sideset_node_ids[i][ii],99,int(ii),0});
      vec_frd[current_part]->elements_connectivity.push_back({sideset_node_ids[i][ii]});
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
        
        me_iface->get_nodes_per_side(element_type[j],side_ids[j],num_node);
        
        std::vector<int> indices(num_node);
        me_iface->get_node_indices_per_side(element_type[j],side_ids[j],indices);

        int nodes_per_element;
        me_iface->get_nodes_per_element(element_type[j],nodes_per_element);
        std::vector<int> conn(nodes_per_element);        
        me_iface->get_connectivity(element_ids[j], conn);

        std::vector<int> tmp_connectivity; 
        
        std::string log;
        log = " element_id " + std::to_string(element_id) + " element_type " + std::to_string(element_type[j]) + " side_id " + std::to_string(side_ids[j]) + " indices.size() " + std::to_string(indices.size()) + " num_node " + std::to_string(num_node) + " \n";
        PRINT_INFO("%s", log.c_str());

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
      std::string log;
      log = "first " + std::to_string(i) + " size " + std::to_string(sideset_elements_connectivity[i][ii].size()) + " \n";
      PRINT_INFO("%s", log.c_str());

      for (size_t iii = 0; iii < sideset_elements_connectivity[i][ii].size(); iii++)
      {
        std::string log;
        log = "second " + std::to_string(iii) + " \n";
        PRINT_INFO("%s", log.c_str());
        
        it = std::find(sideset_node_ids[i].begin(), sideset_node_ids[i].end(), sideset_elements_connectivity[i][ii][iii]);
        if (it==sideset_node_ids[i].end())
        {  
          std::string log;
          log = "geht " + std::to_string(iii) + " \n";
          PRINT_INFO("%s", log.c_str());
          sideset_node_ids[i].push_back(sideset_elements_connectivity[i][ii][iii]);
        }
      }
    }
  }
 
  return true;
}