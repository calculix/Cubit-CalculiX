#include "CoreResultsVtkWriter.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CoreResultsFrd.hpp"
#include "CoreResultsDat.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "ProgressTool.hpp"

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
  int nparts = 0;
  int current_part = 0;
  ProgressTool progressbar;
  std::vector<int> block_ids;
  std::vector<std::vector<int>> block_node_ids;
  std::vector<std::vector<int>> block_element_ids;
  std::vector<int> nodeset_ids;
  std::vector<std::vector<int>> nodeset_node_ids;

  // clear all data before reading and check results
  this->clear();
  this->checkResults();

  // blocks
  block_ids = ccx_iface->get_blocks();
  for (size_t i = 0; i < block_ids.size(); i++)
  {
    block_node_ids.push_back(ccx_iface->get_block_node_ids(block_ids[i]));
    block_element_ids.push_back(ccx_iface->get_block_element_ids(block_ids[i]));
  }
  nparts += block_ids.size();

  /*
  // nodesets
  nodeset_ids = CubitInterface::get_nodeset_id_list();
  for (size_t i = 0; i < nodeset_ids.size(); i++)
  {
    nodeset_node_ids.push_back(CubitInterface::get_nodeset_nodes_inclusive(nodeset_ids[i]));
  }
  nparts += nodeset_ids.size();    
  */

  progressbar.start(0,100,"Writing Results to ParaView Format - Linked Mode");
  auto t_start = std::chrono::high_resolution_clock::now();

  for (size_t i = 0; i < max_increments; i++)
  {
    filepath_vtu.clear();
    part_ids.clear();
    part_names.clear();

    ++current_increment;
    current_part = -1;
    // prepare frd and dat
    for (size_t ii = 0; ii < block_ids.size(); ii++)
    {
      ++current_part;
      CoreResultsFrd* tmp_frd;
      CoreResultsDat* tmp_dat;

      tmp_frd = new CoreResultsFrd();
      tmp_frd->init(job_id);
      tmp_dat = new CoreResultsDat();
      tmp_dat->init(job_id);

      frd = tmp_frd;
      dat = tmp_dat;

      //link nodes
      this->link_nodes(block_node_ids[ii]);
      //link elements
      this->link_elements(block_element_ids[ii]);

      //std::string log;
      //log = "nparts " + std::to_string(nparts) + " block " + std::to_string(block_ids[ii])+ " block size" + std::to_string(block_ids.size())+" ci " + std::to_string(current_increment) + " \n";
      //PRINT_INFO("%s", log.c_str());
      
      current_filepath_vtu = filepath + "." + std::to_string(current_part) + "." + this->get_increment() + ".vtu";
      
      this->write_vtu_linked();

      filepath_vtu.push_back(current_filepath_vtu);
      part_ids.push_back(current_part);
      part_names.push_back(ccx_iface->get_block_name(block_ids[ii]));

      delete tmp_frd;
      delete tmp_dat;
    }

    /*
    for (size_t ii = 0; ii < nodeset_ids.size(); ii++)
    {
      ++current_part;
      CoreResultsFrd* tmp_frd;
      CoreResultsDat* tmp_dat;

      tmp_frd = new CoreResultsFrd();
      tmp_frd->init(job_id);
      tmp_dat = new CoreResultsDat();
      tmp_dat->init(job_id);

      frd = tmp_frd;
      dat = tmp_dat;

      //link nodes
      this->link_nodes(nodeset_node_ids[ii]);
      
      current_filepath_vtu = filepath + "." + std::to_string(current_part) + "." + this->get_increment() + ".vtu";
      
      this->write_vtu_linked();

      filepath_vtu.push_back(current_filepath_vtu);
      part_ids.push_back(current_part);
      part_names.push_back(ccx_iface->get_nodeset_name(nodeset_ids[ii]));

      delete tmp_frd;
      delete tmp_dat;
    }
    */

    current_filepath_vtpc = filepath + "." + this->get_increment() + ".vtpc";
    this->write_vtpc();

    //update progress bar
    const auto t_end = std::chrono::high_resolution_clock::now();
    int duration = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //currentDataRow += frd->result_block_data[data_ids[ii]].size();
    if (duration > 500)
    {
      progressbar.percent(double(current_increment)/double(max_increments));
      progressbar.check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }
  progressbar.end();
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
  ProgressTool progressbar;
  std::string log;
  log = "writing results " + filepath + " for Job ID " + std::to_string(job_id) + " \n";
  PRINT_INFO("%s", log.c_str());
  
  // clear all data before reading and check results
  this->clear();
  this->checkResults();

  progressbar.start(0,100,"Writing Results to ParaView Format - Preparing Nodes and Elements");
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
      progressbar.percent(double(currentDataRow)/double(maxDataRows));
      progressbar.check_interrupt();
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
      progressbar.percent(double(currentDataRow)/double(maxDataRows));
      progressbar.check_interrupt();
      t_start = std::chrono::high_resolution_clock::now();
    }
  }

  progressbar.end();
  progressbar.start(0,100,"Writing Results to ParaView Format - Unlinked Mode");
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
          progressbar.percent(double(current_increment)/double(max_increments));
          progressbar.check_interrupt();
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
  
  progressbar.end();

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
  std::vector<int> ccx_type = { 1, 2, 3, 4, 5, 6,7, 8,9,10,11,12};
  std::vector<int> vtk_type = {12,13,10,25,26,24,5,22,9,23, 3,21};

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
  for (size_t i = 0; i < frd->result_blocks.size(); i++)
  {
    if (max_increments<frd->result_blocks[i][3])
    {
      max_increments = frd->result_blocks[i][3];
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

bool CoreResultsVtkWriter::link_nodes(std::vector<int> node_ids)
{
  for (size_t i = 0; i < frd_all->nodes.size(); i++)
  {
    for (size_t ii = 0; ii < node_ids.size(); ii++)
    {
      if (frd_all->nodes[i][0]==node_ids[ii])
      {
        frd->nodes.push_back(frd_all->nodes[i]);
        frd->nodes_coords.push_back(frd_all->nodes_coords[i]);
        frd->nodes[frd->nodes.size()-1][1] = frd->nodes_coords.size()-1;
      }
    }
  }

  std::vector<int> data_ids = this->get_result_blocks_data_ids_linked(); // get data ids for result blocks in current increment

  for (size_t i = 0; i < data_ids.size(); i++)
  {
    std::vector<int> node_data_ids = this->get_result_block_node_data_id_linked(data_ids[i]);

    frd->result_blocks.push_back(frd_all->result_blocks[data_ids[i]]);
    frd->total_times.push_back(frd_all->total_times[frd_all->result_blocks[data_ids[i]][4]]);
    frd->result_block_components.push_back(frd_all->result_block_components[frd_all->result_blocks[data_ids[i]][6]]);
    frd->result_block_type.push_back(frd_all->result_block_type[frd_all->result_blocks[data_ids[i]][5]]);
    
    frd->result_blocks[frd->result_blocks.size()-1][4] = frd->total_times.size()-1;
    frd->result_blocks[frd->result_blocks.size()-1][5] = frd->result_block_type.size()-1;
    frd->result_blocks[frd->result_blocks.size()-1][6] = frd->result_blocks.size()-1;
    
    std::vector<std::vector<double>> tmp_result_block_data;
    std::vector<std::vector<int>> tmp_result_block_node_data;
    frd->result_block_data.push_back(tmp_result_block_data);
    frd->result_block_node_data.push_back(tmp_result_block_node_data);

    for (size_t ii = 0; ii < node_data_ids.size(); ii++)
    {     
      for (size_t iii = 0; iii < node_ids.size(); iii++)
      {
        if (frd_all->result_block_node_data[data_ids[i]][node_data_ids[ii]][0]==node_ids[iii])
        {
          frd->result_block_data[i].push_back(frd_all->result_block_data[data_ids[i]][node_data_ids[ii]]);
          frd->result_block_node_data[i].push_back(frd_all->result_block_node_data[data_ids[i]][node_data_ids[ii]]);

          frd->result_block_node_data[i][frd->result_block_node_data[i].size()-1][1] = frd->result_block_data[i].size()-1;
        }
      }
    }
  }
  
  return true;
}

bool CoreResultsVtkWriter::link_elements(std::vector<int> element_ids)
{
  for (size_t i = 0; i < frd_all->elements.size(); i++)
  {
    for (size_t ii = 0; ii < element_ids.size(); ii++)
    {
      if (frd_all->elements[i][0]==element_ids[ii])
      {
        frd->elements.push_back(frd_all->elements[i]);
        frd->elements_connectivity.push_back(frd_all->elements_connectivity[i]);
        frd->elements[frd->elements.size()-1][2] = frd->elements_connectivity.size()-1;
      }
    }
  }

  return true;
}