#include "HDF5Tool.hpp"
#include "CubitMessage.hpp"
#ifdef WIN32
 #include <windows.h>
#else
 #include <unistd.h>
#endif

HDF5Tool::HDF5Tool(std::string filename)
{
  //check if hdf5 exists, if not create a new file
  #ifdef WIN32
    if (_access(filename.c_str(), 0) != 0)
    {
      std::string log = filename + " not found. An empty HDF5 will be created.\n";
      PRINT_INFO("%s", log.c_str());

      hid_t new_file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
      if (new_file != H5I_INVALID_HID)
      {
        H5Fclose(new_file);
      }
    }
  #else
    if (access(filename.c_str(), W_OK) != 0) 
    {
      std::string log = filename + " not found. An empty HDF5 will be created.\n";
      PRINT_INFO("%s", log.c_str());

      hid_t new_file = H5Fcreate(filename.c_str(), H5F_ACC_TRUNC, H5P_DEFAULT, H5P_DEFAULT);
      if (new_file != H5I_INVALID_HID)
      {
        H5Fclose(new_file);
      }
    }
  #endif

  this->file = new H5::H5File(filename,H5F_ACC_RDWR);
}

HDF5Tool::~HDF5Tool()
{
  this->file->close();
  delete this->file;
}

bool HDF5Tool::nameExists(std::string name)
{
  return this->file->nameExists(name.c_str());
}

bool HDF5Tool::createGroup(std::string groupname)
{
  this->file->createGroup(groupname.c_str());
  return true;
}

bool HDF5Tool::renameGroup(std::string groupname,std::string new_groupname)
{
  if (!this->nameExists(groupname.c_str()))
  {
    return false;
  }

  if (this->nameExists(new_groupname.c_str()))
  {
    return false;
  }

  int result = H5Lmove(this->file->getId(), groupname.c_str(),this->file->getId(), new_groupname.c_str(), H5P_DEFAULT, H5P_DEFAULT);
  
  return true;
}

bool HDF5Tool::deleteGroup(std::string groupname)
{
  if (!this->nameExists(groupname.c_str()))
  {
    return false;
  }

  int result = H5Ldelete(this->file->getId(), groupname.c_str(), H5P_DEFAULT);
  
  return true;
}

herr_t op_func(hid_t loc_id, const char* name, const H5O_info_t* info, void *data)
{
  auto vec = static_cast<std::vector<std::string>*>(data);

  if (name[0] == '.') /* Root group, do not print '.' */
  {  //ignore
    //vec->push_back("/");
  }
  else
  {
    switch (info->type) {
        case H5O_TYPE_GROUP:
            vec->push_back(std::string(name));
            //printf("%s  (Group)\n", name);
            break;
        case H5O_TYPE_DATASET:
            //printf("%s  (Dataset)\n", name);
            break;
        case H5O_TYPE_NAMED_DATATYPE:
            //printf("%s  (Datatype)\n", name);
            break;
        //default:
            //printf("%s  (Unknown)\n", name);
    }
  }

  return 0;
}

std::vector<std::string> HDF5Tool::getGroupsFromFile()
{
  std::vector<std::string> groups;

  bool status = false;
  status = H5Ovisit(this->file->getId(), H5_INDEX_NAME, H5_ITER_NATIVE, op_func, static_cast<void*>(&groups),H5O_INFO_BASIC);

  return groups;
}



bool HDF5Tool::deleteDataset(std::string name, std::string groupname)
{
  //this->file->createGroup(groupname.c_str());
  H5::Group group(file->openGroup(groupname.c_str()));
  if (!group.exists(name.c_str()))
  {
    return false;
  }

  int result = H5Ldelete(group.getId(), name.c_str(), H5P_DEFAULT);
  
  return true;
}

bool HDF5Tool::read_dataset_int_rank_1(std::string name, std::string groupname, std::vector<int> &data)
{
  std::string log = "";

  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 1;
    if (data.size()!=0)
    {
      log = "data not empty -> please write an issue on github\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(file->openGroup(groupname.c_str()));
    if (!group.exists(name.c_str()))
    {
      //log = "dataset " + name + " in group " + groupname + " doesn't exist\n";
      //PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    H5::DataSet dataset = group.openDataSet(name);
    H5::DataSpace dataspace = dataset.getSpace();
    const int ndims = dataspace.getSimpleExtentNdims();

    if (rank!=ndims)
    {
      log = "ranks doesn't match in dataset " + name + " in group " + groupname + "\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    hsize_t dims[rank];
    dataspace.getSimpleExtentDims(dims, NULL);

    // Read the data from the dataset    
    std::vector<int> rdata(dims[0]); // buffer for data to read

    dataset.read(rdata.data(), H5::PredType::NATIVE_INT);

    for (size_t i = 0; i < dims[0]; i++)
    { 
      data.push_back(rdata[i]);
    }
    
    // Close all objects.
    dataspace.close();
    dataset.close();
    group.close();
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}


bool HDF5Tool::read_dataset_int_rank_2(std::string name, std::string groupname, std::vector<std::vector<int>> &data)
{
  std::string log = "";

  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 2;
    if (data.size()!=0)
    {
      log = "data not empty -> please write an issue on github\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(file->openGroup(groupname.c_str()));
    if (!group.exists(name.c_str()))
    {
      //log = "dataset " + name + " in group " + groupname + " doesn't exist\n";
      //PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    H5::DataSet dataset = group.openDataSet(name);
    H5::DataSpace dataspace = dataset.getSpace();
    const int ndims = dataspace.getSimpleExtentNdims();

    if (rank!=ndims)
    {
      log = "ranks doesn't match in dataset " + name + " in group " + groupname + "\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    hsize_t dims[rank];
    dataspace.getSimpleExtentDims(dims, NULL);

    // Read the data from the dataset    
    std::vector<int> rdata(dims[0]*dims[1]); // buffer for data to read

    dataset.read(rdata.data(), H5::PredType::NATIVE_INT);
    
    /* 
    int getArrayLength = rdata.size();
    std::string log = "array length " + std::to_string(getArrayLength) + " dims[0] " + std::to_string(dims[0]) + " dims[1] " + std::to_string(dims[1]) + "\n";
    PRINT_INFO("%s", log.c_str());

    for (size_t i = 0; i < rdata.size(); i++)
    {
      log = "i " + std::to_string(i) + " rdata[i] " + std::to_string(rdata[i]) + "\n";
      PRINT_INFO("%s", log.c_str());
    }
    */
    
    for (size_t i = 0; i < dims[0]; i++)
    { 
      std::vector<int> tmp;
      for (size_t ii = 0; ii < dims[1]; ii++)
      { 
        tmp.push_back(rdata[dims[1]*i+ii]);
        //log = "data.size() " + std::to_string(data.size()) + " i " + std::to_string(i) + " rdata[dims[1]*i+ii] " + std::to_string(rdata[dims[1]*i+ii]) + "\n";
        //PRINT_INFO("%s", log.c_str());
      }
      data.push_back(tmp);
    }
    
    // Close all objects.
    dataspace.close();
    dataset.close();
    group.close();
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}


bool HDF5Tool::write_dataset_int_rank_1(std::string name, std::string groupname, std::vector<int> data)
{
  // Try block to detect exceptions raised by any of the calls inside it
  try {
    //check if there is data that can be written
    if (data.size()==0)
    {
      return true;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(this->file->openGroup(groupname.c_str()));
    const int rank = 1;
    hsize_t dims[rank]; // dataset dimensions
    dims[0]              = data.size();
    H5::DataSpace *dataspace = new H5::DataSpace(rank, dims);
    
    H5::DataSet *dataset = new H5::DataSet(group.createDataSet(name.c_str(), H5::PredType::NATIVE_INT, *dataspace));

    // Write the data to the dataset using default memory space, file
    // space, and transfer properties.
    int *wdata = new int [dims[0]];
    
    for (size_t i = 0; i < data.size(); i++)
    {
      wdata[i] = data[i];
    }
    dataset->write(wdata, H5::PredType::NATIVE_INT);

    // Close all objects.
    delete dataspace;
    delete dataset;
    group.close();
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}

bool HDF5Tool::write_dataset_int_rank_2(std::string name, std::string groupname, std::vector<std::vector<int>> data)
{
  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 2;
    if (data.size()==0)
    {
      return true;
    }
    if (data[0].size()==0)
    {
      return true;
    }
    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(this->file->openGroup(groupname.c_str()));
    hsize_t dims[rank]; // dataset dimensions
    dims[0]              = data.size();
    dims[1]              = data[0].size();
    H5::DataSpace *dataspace = new H5::DataSpace(rank, dims);
    
    H5::DataSet *dataset = new H5::DataSet(group.createDataSet(name.c_str(), H5::PredType::NATIVE_INT, *dataspace));

    // Write the data to the dataset using default memory space, file
    // space, and transfer properties.
    
    int *wdata = new int[dims[0]*dims[1]]; // buffer for data to write
    
    for (size_t i = 0; i < data.size(); i++)
    {
      for (size_t ii = 0; ii < data[i].size(); ii++)
      {
        wdata[dims[1]*i+ii] = data[i][ii];
      }
    }
    dataset->write(wdata, H5::PredType::NATIVE_INT);
        
    // Close all objects.
    delete dataspace;
    delete dataset;
    delete wdata;
    group.close();

/*
    std::string log = "";
    for (size_t i = 0; i < data.size(); i++)
    {
      for (size_t ii = 0; ii < data[i].size(); ii++)
      {
        log.append(std::to_string(data[i][ii]) + "->");
      }
      log.append("\n");
    }
    PRINT_INFO("%s", log.c_str());
*/
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}

bool HDF5Tool::write_dataset_double_rank_1(std::string name, std::string groupname, std::vector<double> data)
{
  // Try block to detect exceptions raised by any of the calls inside it
  try {
    //check if there is data that can be written
    if (data.size()==0)
    {
      return true;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(this->file->openGroup(groupname.c_str()));
    const int rank = 1;
    hsize_t dims[rank]; // dataset dimensions
    dims[0]              = data.size();
    H5::DataSpace *dataspace = new H5::DataSpace(rank, dims);
    
    H5::DataSet *dataset = new H5::DataSet(group.createDataSet(name.c_str(), H5::PredType::NATIVE_DOUBLE, *dataspace));

    // Write the data to the dataset using default memory space, file
    // space, and transfer properties.
    double *wdata = new double [dims[0]];
    
    for (size_t i = 0; i < data.size(); i++)
    {
      wdata[i] = data[i];
    }
    dataset->write(wdata, H5::PredType::NATIVE_DOUBLE);

    // Close all objects.
    delete dataspace;
    delete dataset;
    group.close();
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}


bool HDF5Tool::read_dataset_double_rank_1(std::string name, std::string groupname, std::vector<double> &data)
{
  std::string log = "";

  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 1;
    if (data.size()!=0)
    {
      log = "data not empty -> please write an issue on github\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(file->openGroup(groupname.c_str()));
    if (!group.exists(name.c_str()))
    {
      //log = "dataset " + name + " in group " + groupname + " doesn't exist\n";
      //PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    H5::DataSet dataset = group.openDataSet(name);
    H5::DataSpace dataspace = dataset.getSpace();
    const int ndims = dataspace.getSimpleExtentNdims();

    if (rank!=ndims)
    {
      log = "ranks doesn't match in dataset " + name + " in group " + groupname + "\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    hsize_t dims[rank];
    dataspace.getSimpleExtentDims(dims, NULL);

    // Read the data from the dataset    
    std::vector<double> rdata(dims[0]); // buffer for data to read

    dataset.read(rdata.data(), H5::PredType::NATIVE_DOUBLE);

    for (size_t i = 0; i < dims[0]; i++)
    { 
      data.push_back(rdata[i]);
    }
    
    // Close all objects.
    dataspace.close();
    dataset.close();
    group.close();
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}


bool HDF5Tool::read_dataset_double_rank_2(std::string name, std::string groupname, std::vector<std::vector<double>> &data)
{
  std::string log = "";

  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 2;
    if (data.size()!=0)
    {
      log = "data not empty -> please write an issue on github\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(file->openGroup(groupname.c_str()));
    if (!group.exists(name.c_str()))
    {
      //log = "dataset " + name + " in group " + groupname + " doesn't exist\n";
      //PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    H5::DataSet dataset = group.openDataSet(name);
    H5::DataSpace dataspace = dataset.getSpace();
    const int ndims = dataspace.getSimpleExtentNdims();

    if (rank!=ndims)
    {
      log = "ranks doesn't match in dataset " + name + " in group " + groupname + "\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    hsize_t dims[rank];
    dataspace.getSimpleExtentDims(dims, NULL);

    // Read the data from the dataset    
    std::vector<double> rdata(dims[0]*dims[1]); // buffer for data to read

    dataset.read(rdata.data(), H5::PredType::NATIVE_DOUBLE);
    
    for (size_t i = 0; i < dims[0]; i++)
    { 
      std::vector<double> tmp;
      for (size_t ii = 0; ii < dims[1]; ii++)
      { 
        tmp.push_back(rdata[dims[1]*i+ii]);
        //log = "data.size() " + std::to_string(data.size()) + " i " + std::to_string(i) + " rdata[dims[1]*i+ii] " + std::to_string(rdata[dims[1]*i+ii]) + "\n";
        //PRINT_INFO("%s", log.c_str());
      }
      data.push_back(tmp);
    }
    
    // Close all objects.
    dataspace.close();
    dataset.close();
    group.close();
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}


bool HDF5Tool::write_dataset_double_rank_2(std::string name, std::string groupname, std::vector<std::vector<double>> data)
{
  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 2;
    if (data.size()==0)
    {
      return true;
    }
    if (data[0].size()==0)
    {
      return true;
    }
    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(this->file->openGroup(groupname.c_str()));
    hsize_t dims[rank]; // dataset dimensions
    dims[0]              = data.size();
    dims[1]              = data[0].size();
    H5::DataSpace *dataspace = new H5::DataSpace(rank, dims);
    
    H5::DataSet *dataset = new H5::DataSet(group.createDataSet(name.c_str(), H5::PredType::NATIVE_DOUBLE, *dataspace));

    // Write the data to the dataset using default memory space, file
    // space, and transfer properties.
    
    double *wdata = new double[dims[0]*dims[1]]; // buffer for data to write
    
    for (size_t i = 0; i < data.size(); i++)
    {
      for (size_t ii = 0; ii < data[i].size(); ii++)
      {
        wdata[dims[1]*i+ii] = data[i][ii];
      }
    }
    dataset->write(wdata, H5::PredType::NATIVE_DOUBLE);
        
    // Close all objects.
    delete dataspace;
    delete dataset;
    delete wdata;
    group.close();

    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}

bool HDF5Tool::read_dataset_string_rank_1(std::string name, std::string groupname, std::vector<std::string> &data)
{
  std::string log = "";

  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 1;
    if (data.size()!=0)
    {
      log = "data not empty -> please write an issue on github\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(file->openGroup(groupname.c_str()));
    if (!group.exists(name.c_str()))
    {
      //log = "dataset " + name + " in group " + groupname + " doesn't exist\n";
      //PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    H5::DataSet dataset = group.openDataSet(name);
    H5::DataSpace dataspace = dataset.getSpace();
    const int ndims = dataspace.getSimpleExtentNdims();

    if (rank!=ndims)
    {
      log = "ranks doesn't match in dataset " + name + " in group " + groupname + "\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    hsize_t dims[rank];
    dataspace.getSimpleExtentDims( dims, NULL);
    H5::StrType datatype(H5::PredType::C_S1, H5T_VARIABLE); 

    std::vector<const char *> rdata(dims[0]); // buffer for data to read

    dataset.read(rdata.data(), datatype);
        
    for (size_t i = 0; i < dims[0]; i++)
    { 
      data.push_back(rdata[i]);
    }
    
    // Close all objects.
    dataspace.close();
    dataset.close();
    group.close();
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}

bool HDF5Tool::read_dataset_string_rank_2(std::string name, std::string groupname, std::vector<std::vector<std::string>> &data)
{
  std::string log = "";

  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 2;
    if (data.size()!=0)
    {
      log = "data not empty -> please write an issue on github\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(file->openGroup(groupname.c_str()));
    if (!group.exists(name.c_str()))
    {
      //log = "dataset " + name + " in group " + groupname + " doesn't exist\n";
      //PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    H5::DataSet dataset = group.openDataSet(name);
    H5::DataSpace dataspace = dataset.getSpace();
    const int ndims = dataspace.getSimpleExtentNdims();

    if (rank!=ndims)
    {
      log = "ranks doesn't match in dataset " + name + " in group " + groupname + "\n";
      PRINT_INFO("%s", log.c_str());
      return false;
    }
    
    hsize_t dims[rank];
    dataspace.getSimpleExtentDims( dims, NULL);
    H5::StrType datatype(H5::PredType::C_S1, H5T_VARIABLE); 

    std::vector<const char *> rdata(dims[0]*dims[1]); // buffer for data to read

    dataset.read(rdata.data(), datatype);
        
    for (size_t i = 0; i < dims[0]; i++)
    { 
      std::vector<std::string> tmp;
      for (size_t ii = 0; ii < dims[1]; ii++)
      { 
        tmp.push_back(rdata[dims[1]*i+ii]);
        //log = "data.size() " + std::to_string(data.size()) + " i " + std::to_string(i) + " rdata[dims[1]*i+ii] " + rdata[dims[1]*i+ii] + "\n";
        //PRINT_INFO("%s", log.c_str());
      }
      data.push_back(tmp);
    }
    
    // Close all objects.
    dataspace.close();
    dataset.close();
    group.close();
    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}

bool HDF5Tool::write_dataset_string_rank_1(std::string name, std::string groupname, std::vector<std::string> data)
{
  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 1;
    if (data.size()==0)
    {
      return true;
    }

    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(this->file->openGroup(groupname.c_str()));
    hsize_t dims[rank]; // dataset dimensions
    dims[0]              = data.size();
    H5::DataSpace *dataspace = new H5::DataSpace(rank, dims);
    H5::StrType datatype(H5::PredType::C_S1, H5T_VARIABLE); 

    H5::DataSet *dataset = new H5::DataSet(group.createDataSet(name.c_str(), datatype, *dataspace));

    // Write the data to the dataset using default memory space, file
    // space, and transfer properties.
    
    std::vector<const char*> wdata;
    for (size_t i = 0; i < data.size(); i++)
    {
      wdata.push_back(data[i].c_str());
    }
    dataset->write(wdata.data(), datatype);
    
    // Close all objects.
    delete dataspace;
    delete dataset;
    group.close();

    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}

bool HDF5Tool::write_dataset_string_rank_2(std::string name, std::string groupname, std::vector<std::vector<std::string>> data)
{
  // Try block to detect exceptions raised by any of the calls inside it
  try {
    const int rank = 2;
    if (data.size()==0)
    {
      return true;
    }
    if (data[0].size()==0)
    {
      return true;
    }
    // Turn off the auto-printing when failure occurs so that we can
    // handle the errors appropriately
    H5::Exception::dontPrint();

    H5::Group group(this->file->openGroup(groupname.c_str()));
    hsize_t dims[rank]; // dataset dimensions
    dims[0]              = data.size();
    dims[1]              = data[0].size();
    H5::DataSpace *dataspace = new H5::DataSpace(rank, dims);
    H5::StrType datatype(H5::PredType::C_S1, H5T_VARIABLE); 

    H5::DataSet *dataset = new H5::DataSet(group.createDataSet(name.c_str(), datatype, *dataspace));

    // Write the data to the dataset using default memory space, file
    // space, and transfer properties.
    
    std::vector<const char*> wdata;
    for (size_t i = 0; i < data.size(); i++)
    {
      for (size_t ii = 0; ii < data[i].size(); ii++)
      {
        wdata.push_back(data[i][ii].c_str());
      }
    }
    dataset->write(wdata.data(), datatype);

    // Close all objects.
    delete dataspace;
    delete dataset;
    group.close();

    return true;
  } // end of try block

  // catch failure caused by the H5File operations
  catch (H5::FileIException error) {
      error.printErrorStack();
      return false;
  }

  // catch failure caused by the DataSet operations
  catch (H5::DataSetIException error) {
      error.printErrorStack();
      return false;
  }

  return true;
}