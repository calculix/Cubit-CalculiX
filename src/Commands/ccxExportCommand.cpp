#include "ccxExportCommand.hpp"
#include "MeshExportInterface.hpp"
#include "MaterialInterface.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include "CalculiXCoreInterface.hpp"
#include <fstream>

ccxExportCommand::ccxExportCommand()
{}

ccxExportCommand::~ccxExportCommand()
{}

std::vector<std::string> ccxExportCommand::get_syntax()
{
  // Define the syntax for the command. Note the syntax is a modified BNF
  // format. Full documentation on the command specification syntax can be
  // found in the documentation.
  std::string syntax =
      "export ccx <string:label='filename',help='<filename>'> "
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
  std::ofstream output_file;
  MeshExportInterface *iface;
  MaterialInterface *material_iface;

  // The command syntax specified that we should have a string labeled "filename" in the
  // the command. We would not have gotten to this point without having that required
  // piece of data. Retrieve it here
  std::string filename;
  data.get_string("filename", filename);

  // The syntax also indicated that we could have an optional piece of data asking if we
  // wanted to overwrite an existing file. This will return true if the overwrite option
  // was specified in the command

  //bool overwrite = data.find_keyword("overwrite");

  // TODO: check if the given file exists. If it does and 'overwrite' was not specified,
  // print an error and return from this function.

  // Get the MeshExport interface from CubitInterface
  iface = dynamic_cast<MeshExportInterface*>(CubitInterface::get_interface("MeshExport"));
  if(!iface)
  {
    PRINT_ERROR("Unable to get mesh export interface.\n");
    return false;
  }

  // Get the Material Interface from CubitInterface
  material_iface = dynamic_cast<MaterialInterface*>(CubitInterface::get_interface("Material"));
  if(!material_iface)
  {
    PRINT_ERROR("Unable to get material interface.\n");
    return false;
  }

  // define the CalculiXCoreInterface
  CalculiXCoreInterface ccx_iface;

  // Now we can use the interface as before
  iface->set_use_sequential_ids(false);

  // Open the file and write to it
  bool result =
      open_file(filename, output_file) && write_file(output_file, iface, material_iface, ccx_iface);

  // Close the file
  close_file(output_file);

  // Make sure that you release the interface after accessing it
  CubitInterface::release_interface(iface);
  CubitInterface::release_interface(material_iface);

  return result;
}

bool ccxExportCommand::open_file(const std::string& file, std::ofstream& output_file)
{
  output_file.open(file.c_str());

  return true;
}

int ccxExportCommand::get_side(int element_type,int side)
{
  int s_return;
  if ((element_type>=39) && (element_type<=43)) {
    // HEX
    if (side==1) {
      s_return=3;
    } else if (side==2) {
      s_return=4;
    } else if (side==3){
      s_return=5;
    } else if (side==4){
      s_return=6;
    } else if (side==5){
      s_return=1;
    } else if (side==6){
      s_return=2;
    }
  } else if ((element_type>=28) && (element_type<=33)) {
    // TETRA
    if (side==1) {
      s_return=2;
    } else if (side==2) {
      s_return=3;
    } else if (side==3){
      s_return=4;
    } else if (side==4){
      s_return=1;
    }
  } else if ((element_type>=11) && (element_type<=14)) {
    // TRI
    if (side==1) {
      s_return=3;
    } else if (side==2) {
      s_return=4;
    } else if (side==3){
      s_return=5;
    } else if (side==4){
      s_return=1;
    } else if (side==5){
      s_return=2;
    }
  } else if ((element_type>=15) && (element_type<=18)) {
    // TRISHELL
    if (side==1) {
      s_return=2;
    } else if (side==2) {
      s_return=1;
    } else if (side==3){
      s_return=3;
    } else if (side==4){
      s_return=4;
    } else if (side==5){
      s_return=5;
    }
  } else if ((element_type>=19) && (element_type<=22)) {
    // SHELL
    if (side==1) {
      s_return=2;
    } else if (side==2) {
      s_return=1;
    } else if (side==3){
      s_return=3;
    } else if (side==4){
      s_return=4;
    } else if (side==5){
      s_return=5;
    } else if (side==6){
      s_return=6;
    }
  } else if ((element_type>=23) && (element_type<=27)) {
    // QUAD
    if (side==1) {
      s_return=3;
    } else if (side==2) {
      s_return=4;
    } else if (side==3){
      s_return=5;
    } else if (side==4){
      s_return=6;
    } else if (side==5){
      s_return=1;
    } else if (side==6){
      s_return=2;
    }
  } else {
    s_return = side;
  }

  return s_return;
}

bool ccxExportCommand::write_file(std::ofstream& output_file, MeshExportInterface *iface, MaterialInterface *material_iface, CalculiXCoreInterface ccx_iface)
{
  bool result;
  // Initialize the exporter
  iface->initialize_export();

  // first blood ... no i mean first line
  output_file << "*HEADING \n";
  output_file << "Cubit2CalculiX with CalculiX Plugin \n";
  output_file << "** \n";

  // Write the nodes
  result = write_nodes(output_file, iface);

  // Write the elements/connectivity
  result = write_connectivity(output_file, iface, ccx_iface);

  // Write the nodesets
  result = write_nodesets(output_file, iface);

  // Write the sidesets
  result = write_sidesets(output_file, iface);

  // Write the materials and properties
  result = write_materials(output_file, iface, material_iface);
  result = write_properties(output_file, iface, material_iface);

  return result;
}

bool ccxExportCommand::close_file(std::ofstream& output_file)
{
  output_file.close();
  return true;
}



bool ccxExportCommand::write_nodes(std::ofstream& output_file,MeshExportInterface *iface)
{

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
  start += number_found;

  output_file << "** \n";
  return true;
}

bool ccxExportCommand::write_connectivity(std::ofstream& output_file,MeshExportInterface *iface, CalculiXCoreInterface ccx_iface)
{
  //output_file << " List of Elements" << std::endl;
  //output_file << " Element Type, ID, Block, Connectivity" << std::endl;


  output_file << "********************************** E L E M E N T S **************************** \n";

  /*
  // Get the list of blocks
  std::vector<BlockHandle> blocks;
  iface->get_block_list(blocks);

  // Test for get_block_size
  int num_elements = 0;
  for (size_t i = 0; i < blocks.size(); i++)
  {
    int nelems;
    bool status = iface->get_block_size(blocks[i], nelems);
    if (status)
      num_elements += nelems;
  }
  */

  // Get Block id's
  std::vector<int> blocks;
  blocks = ccx_iface.get_blocks();

  // Get a batch of elements in an initialized block
  int buf_size = 100;
  std::vector<ElementType>   element_type(buf_size);
  std::vector<ElementHandle> handles(buf_size);

  // define name variable
  std::string element_type_name;
  std::string block_name;

  // Elements in a buffer set will be of the same element type and in the same block
  for (size_t i = 0; i < blocks.size(); i++)
  {
    int num_elems;
    int start_index = 0;
    //BlockHandle block = blocks[i];
    BlockHandle block;
    iface->get_block_handle(blocks[i], block);

    if (CubitInterface::get_block_element_type(blocks[i]) != "SPHERE") // check if cubit element type from block is sphere
    {         
      // write only once per block
      int block_id = blocks[i];
      block_name = ccx_iface.get_block_name(block_id);
      element_type_name = ccx_iface.get_ccx_element_type(block_id);
      output_file << "*ELEMENT, TYPE=" << element_type_name << ", ELSET=" << block_name << "\n";

      while( (num_elems = iface->get_block_elements(start_index, buf_size, block, element_type, handles)) > 0)
      {
        // Get ids for the element handles
        std::vector<int> ids(num_elems);
        iface->get_element_ids(num_elems, handles, ids);

        // skip if element type is 0 (SPHERE), that element type doesn´t exist in CalculiX
        if (element_type[0] != 0) 
        {
          // Write out the connectivity
          for (int i = 0; i < num_elems; i++)
          {
            std::vector<int> conn(27);
            int num_nodes = iface->get_connectivity(handles[i], conn);

            //output_file << (int) element_type[i] << " " << ids[i] << " " << block_id << " ";
            output_file << ids[i] << ", ";
            for (int j = 0; j < num_nodes; j++)
            {
              // different node numbering for hex20
              if (element_type[0] == 42) {
                if (j >= 12 && j<=15) {
                  output_file << conn[j+4] << ", ";
                } else if (j >= 16 && j<=19) {
                  output_file << conn[j-4] << ", ";
                } else {
                  output_file << conn[j] << ", ";
                }
              } else {
                output_file << conn[j] << ", ";
              }

              if (j == 14) {
                output_file << "\n";
              }
            }
            output_file << std::endl;
          }
        }
        start_index += num_elems;
      }
    }
  }

  output_file << "** \n";

  return true;

}

bool ccxExportCommand::write_nodesets(std::ofstream& output_file,MeshExportInterface *iface)
{

  output_file << "********************************** N O D E S E T S ********************************** \n";

  // Get the list of nodesets
  std::vector<NodesetHandle> nodesets;
  iface->get_nodeset_list(nodesets);

  // Get a batch of nodes from the nodeset
  int buf_size = 100;

  // loop over the nodesets
  for (size_t i = 0; i < nodesets.size(); i++)
  {
    NodesetHandle nodeset = nodesets[i];
    output_file << "*NSET, NSET=" << iface->get_nodeset_name(nodeset) << " \n";

    int num_nodes;
    int start_index = 0;

    std::vector<int> ids;

    while( (num_nodes = iface->get_nodeset_node_ids(nodeset,start_index,buf_size,ids)) > 0)
    {

    //counter to know when the next line should begin
    int ic;
    ic=0;

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
    start_index += num_nodes;
    }
    output_file << std::endl;
  }

  output_file << "** \n";
  return true;
}

bool ccxExportCommand::write_sidesets(std::ofstream& output_file, MeshExportInterface *iface)
{

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
      sideset_name = "Block_" + std::to_string(i);
    }

    int num_elems;
    int start_index = 0;
    int element_count = 0;

    std::vector<ElementHandle> element_ids;
    std::vector<int> side_ids;
    std::vector<ElementType> element_type;


    // count all elements in sideset
    while( (num_elems = iface->get_sideset_sides(sideset,start_index,buf_size,element_ids,element_type,side_ids)) > 0)
    {
    start_index += num_elems;
    element_count += num_elems;
    }

    int element_id;
    std::vector<int> side_ids_all(element_count);
    std::vector<int> element_ids_all(element_count);
    std::vector<int> element_type_all(element_count);

    start_index = 0;
    while( (num_elems = iface->get_sideset_sides(sideset,start_index,buf_size,element_ids,element_type,side_ids)) > 0)
    {

    // get all elements in sideset
    for (int j = 0; j < num_elems; j++)
      {
      iface->get_element_id(element_ids[j],element_id);
      element_ids_all[start_index + j] = element_id;
      element_type_all[start_index + j] = element_type[j];
      side_ids_all[start_index + j] = side_ids[j];
      }
    start_index += num_elems;
    }

    //counter to know when the next line should begin
    int ic;
    // surface id to write
    int sw;
    std::vector<int>::iterator it;
    bool bool_first;
    bool_first=true;
    // loop for every possible side
    for (int s = 1; s < 7; s++)
    {
      // Check if side number is in vector
      it = std::find(side_ids_all.begin(), side_ids_all.end(), s);
      if (it != side_ids_all.end())
      {
        if (bool_first) {
          sw = get_side(element_type_all[0], s);
          output_file << "*ELSET, ELSET=" << sideset_name << ("_S" + std::to_string(sw)) << " \n";
          bool_first=false;
        }
        ic=0;
        for (int j = 0; j < element_count; j++)
        {
          //only write when it´s the right side
          if (s==side_ids_all[j]) {
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
    }
    // write surface card for sideset
    output_file << "*SURFACE, NAME=";
    for (int s = 1; s < 7; s++)
    {
      if (bool_first) {
        output_file << "*SURFACE, NAME=" << sideset_name << " \n";
        bool_first=false;
      }
      // Check if side number is in vector
      it = std::find(side_ids_all.begin(), side_ids_all.end(), s);
      if (it != side_ids_all.end())
        {
          // VERY IMPORTANT, Calulix hast different side numbering than cubit!!
          sw = get_side(element_type_all[0], s);

          output_file << sideset_name << ("_S" + std::to_string(sw)) << ", " << ("S" + std::to_string(sw)) << " \n";
        }
    }
    output_file << std::endl;
  }

  output_file << "** \n";
  return true;
}

bool ccxExportCommand::write_materials(std::ofstream& output_file, MeshExportInterface *iface,MaterialInterface *material_iface)
{

  // define variables
  std::string material_name;

  // define block handles
  std::vector<BlockHandle> blocks;
  BlockHandle block;

  // define material handles
  std::vector<MaterialHandle> materials;
  MaterialHandle material;
  std::vector<MaterialInterface::Property> properties;
  MaterialInterface::Property prop;
  double prop_scalar;
  MaterialVector prop_vector;
  MaterialMatrix prop_matrix;

  // Get the list of blocks and materials
  iface->get_block_list(blocks);
  iface->get_material_list(materials);

  output_file << "********************************** M A T E R I A L S **************************** \n";
  for (size_t i = 0; i < materials.size(); i++)
  {
    // variables for later output
    // are defined her so they will be erased/cleared after each different material
    std::vector<double> material_elastic(2); // index 0=modulus 1=poisson
    MaterialMatrix material_plastic;
    //
    material = materials[i];
    material_name = material_iface->get_material_name(material);
    output_file << "*MATERIAL, NAME="<< material_name << " \n";
    properties = material_iface->get_material_properties(material);
    // we have to first get all data for the material and then write to the file
    // because the order of the data that we get can be mixed
    for (size_t ii = 0; ii < properties.size(); ii++)
    {
      prop = properties[ii];
      if (material_iface->get_property_name(prop) == "MODULUS") {
        material_iface->get_material_property_value(material, prop, prop_scalar);
        material_elastic[0] = prop_scalar;
      } else if (material_iface->get_property_name(prop) == "POISSON") {
        material_iface->get_material_property_value(material, prop, prop_scalar);
        material_elastic[1] = prop_scalar;
      } else if (material_iface->get_property_name(prop) == "YIELD_STRESS_VS_STRAIN_VS_TEMPERATURE") {
        material_iface->get_material_property_value(material, prop, prop_matrix);
        material_plastic = prop_matrix;
      } else {
        output_file << "\n export for property name "<< material_iface->get_property_name(prop) << " is not supported \n";
      }
    }
    // write out the data in our wished order
    if (material_elastic[0]!=0){
    output_file << "* ELASTIC "<< " \n";
    output_file << material_elastic[0] << ", " << material_elastic[1] << " \n";
    }
    if (material_plastic.size()!=0){
    output_file << "* PLASTIC "<< " \n";
    }
    for (size_t ii = 0; ii < material_plastic.size(); ii++)
    {
      output_file << material_plastic[ii][0] << ", ";
      output_file << material_plastic[ii][1] ;
      if (material_plastic[ii][2]!=0){
        output_file << ", " << material_plastic[ii][2] << " \n";
      } else {
        output_file << " \n";
      }
    }
  }
  return true;
}


bool ccxExportCommand::write_properties(std::ofstream& output_file, MeshExportInterface *iface,MaterialInterface *material_iface)
{

  // define variables
  std::string block_name;
  std::string material_name;
  std::string section_name;
  std::string section_first_line;
  std::string section_second_line;
  
  // define block handles
  std::vector<BlockHandle> blocks;
  BlockHandle block;

  // define material handles
  std::vector<MaterialHandle> materials;
  MaterialHandle material;
  
  // Get the list of blocks and materials
  iface->get_block_list(blocks);
  iface->get_material_list(materials);

  output_file << "********************************** P R O P E R T I E S **************************** \n";
  // connect the materials to our blocks
  for (size_t i = 0; i < blocks.size(); i++)
  {

    block = blocks[i];
    block_name = iface->name_from_handle(block);
    
    //get block attributes
    std::vector<std::pair<char *,double>> block_attributes; //list of attributes on block
    iface->get_block_attributes(block, block_attributes);

    iface->get_block_material(block,material);
    material_name = material_iface->get_material_name(material);

    // Get a batch of elements in an initialized block
    std::vector<ElementType>   element_type(1);
    std::vector<ElementHandle> handles(1);

    iface->get_block_elements(0, 1, block, element_type, handles);
    // choose right section type for the element type
    if (element_type[0] == 1) {
      // BAR
      section_name = "BEAM";
    } else if (element_type[0] == 2) {
      // BAR2
      section_name = "BEAM";
    } else if (element_type[0] == 3) {
      // BAR3
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 4) {
      // BEAM
      section_name = "BEAM";
    } else if (element_type[0] == 5) {
      // BEAM2
      section_name = "BEAM";
    } else if (element_type[0] == 6) {
      // BEAM3
      section_name = "BEAM";
    } else if (element_type[0] == 7) {
      // TRUSS
      section_name = "BEAM";
    } else if (element_type[0] == 8) {
      // TRUSS2
      section_name = "BEAM";
    } else if (element_type[0] == 9) {
      // TRUSS3
      section_name = "BEAM";
    } else if (element_type[0] == 10) {
      // SPRING
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 11) {
      // TRI
      section_name = "SOLID";
    } else if (element_type[0] == 12) {
      // TRI3
      section_name = "SOLID";
    } else if (element_type[0] == 13) {
      // TRI6
      section_name = "SOLID";
    } else if (element_type[0] == 14) {
      // TRI7
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 15) {
      // TRISHELL
      section_name = "SHELL";
    } else if (element_type[0] == 16) {
      // TRISHELL3
      section_name = "SHELL";
    } else if (element_type[0] == 17) {
      // TRISHELL6
      section_name = "SHELL";
    } else if (element_type[0] == 18) {
      // TRISHELL7
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 19) {
      // SHELL
      section_name = "SHELL";
    } else if (element_type[0] == 20) {
      // SHELL4
      section_name = "SHELL";
    } else if (element_type[0] == 21) {
      // SHELL8
      section_name = "SHELL";
    } else if (element_type[0] == 22) {
      // SHELL9
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 23) {
      // QUAD
      section_name = "SOLID";
    } else if (element_type[0] == 24) {
      // QUAD4
      section_name = "SOLID";
    } else if (element_type[0] == 25) {
      // QUAD5
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 26) {
      // QUAD8
      section_name = "SOLID";
    } else if (element_type[0] == 27) {
      // QUAD9
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 28) {
      // Tetra
      section_name = "SOLID";
    } else if (element_type[0] == 29) {
      // Tetra4
      section_name = "SOLID";
    } else if (element_type[0] == 30) {
      // Tetra8
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 31) {
      // Tetra10
      section_name = "SOLID";
    } else if (element_type[0] == 32) {
      // Tetra14
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 33) {
      // Tetra15
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 39) {
      // HEX
      section_name = "SOLID";
    } else if (element_type[0] == 40) {
      // HEX8
      section_name = "SOLID";
    } else if (element_type[0] == 41) {
      // HEX9
      section_name = "NOT SUPPORTED";
    } else if (element_type[0] == 42) {
      // HEX20
      section_name = "SOLID";
    } else if (element_type[0] == 43) {
      // HEX27
      section_name = "NOT SUPPORTED";
    } else {
      // if not implemented take cubit name
      section_name = "NOT SUPPORTED";
    }
    //output_file << "BLOCKNAME "<< block_name << ", MaterialNAME "<< material_name << ", SectionNAME "<< section_name << " \n";;
    section_first_line = "*" + section_name + " SECTION, ELSET=" + block_name + ", MATERIAL=" + material_name;
    

    //the thickness for plain strain and stress is defined in block attribute 1
    //the thickness for shell defined in block attribute 1, the offset in block attribute 2
    if ((element_type[0]>=11) && (element_type[0]<=14)) {
      // TRI
      if (block_attributes.size() == 1) {
        section_second_line = std::to_string(block_attributes[0].second);  
      }else{
        section_second_line = "1";  
      }
    } else if ((element_type[0]>=15) && (element_type[0]<=18)) {
      // TRISHELL
      if (block_attributes.size() == 2) {
        section_first_line = section_first_line + ", OFFSET=" +  std::to_string(block_attributes[1].second);
        section_second_line = std::to_string(block_attributes[0].second);  
      }else{
        section_first_line = section_first_line + ", OFFSET= 0";
        section_second_line = "1";  
      }
    } else if ((element_type[0]>=19) && (element_type[0]<=22)) {
      // SHELL
      if (block_attributes.size() == 2) {
        section_first_line = section_first_line + ", OFFSET=" +  std::to_string(block_attributes[1].second);
        section_second_line = std::to_string(block_attributes[0].second);  
      }else{
        section_first_line = section_first_line + ", OFFSET= 0";
        section_second_line = "1";  
      }
    } else if ((element_type[0]>=23) && (element_type[0]<=27)) {
      // QUAD
      if (block_attributes.size() == 1) {
        section_second_line = std::to_string(block_attributes[0].second);  
      }else{
        section_second_line = "1";  
      }
    }
    
    if ((block_name!="") && (section_name!="Default-Steel")){ //catch unassigned material
    output_file << section_first_line << " \n" ;
    output_file << section_second_line << " \n" ;
    }

  }

  return true;
}
