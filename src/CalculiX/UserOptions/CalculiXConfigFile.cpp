#include "CalculiXConfigFile.hpp"
#include "Framework/Utility/LoadedLibraryDirectory.hpp"

#include <QDir>
#include <fstream>
#include <iostream>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <locale>
#include <sstream>

namespace
{
int resource_library_anchor = 0;
}

CalculiXConfigFile::CalculiXConfigFile()
{
  filename = "ccx.cfg";
  const std::string directory = LoadedLibraryDirectory::from_address(&resource_library_anchor);
  componentpath = QDir::fromNativeSeparators(QString::fromStdString(directory)).toStdString() + "/";
  filepath = QDir::current().filePath(QString::fromStdString(filename)).toStdString();
}
CalculiXConfigFile::~CalculiXConfigFile()
{}

void CalculiXConfigFile::clear()
{
    std::ofstream output_file;
    output_file.open(filepath, std::ofstream::out | std::ofstream::trunc);
    output_file.close();
}

void CalculiXConfigFile::read_entry(std::string option, QString &value)
{
    std::ifstream input_file;
    input_file.open(filepath.c_str());
    if (input_file)
    {
        for( std::string line; std::getline( input_file, line ); )
        {
            if (line.find(option) != std::string::npos)
            {
                //std::cout << line.substr(option.length()+1) << "\n";
                if (line.substr(option.length(),1)=="=")
                {
                    value = QString::fromStdString(line.substr(option.length()+1));
                    if (value=="")
                    {
                        value = standard_entry(option);
                    }
                }
            }
        }
        input_file.close();
    }else{
        value = standard_entry(option);
    }
}

void CalculiXConfigFile::read_num_entry(std::string option, int &value)
{
    std::ifstream input_file;
    input_file.open(filepath.c_str());
    if (input_file)
    {
        for( std::string line; std::getline( input_file, line ); )
        {
            if (line.find(option) != std::string::npos)
            {
                if (line.substr(option.length(),1)=="=")
                {
                    value = std::stoi(line.substr(option.length()+1));
                    if (value==0)
                    {
                        value = standard_num_entry(option);
                    }
                }
            }
        }
        input_file.close();
    }else{
        value = standard_num_entry(option);
    }
}

void CalculiXConfigFile::read_bool_entry(std::string option, bool &value)
{
    std::ifstream input_file;
    QString cfgvalue;
    input_file.open(filepath.c_str());
    if (input_file)
    {
        for( std::string line; std::getline( input_file, line ); )
        {
            if (line.find(option) != std::string::npos)
            {
                //std::cout << line.substr(option.length()+1) << "\n";
                if (line.substr(option.length(),1)=="=")
                {
                    cfgvalue = QString::fromStdString(line.substr(option.length()+1));
                    if (cfgvalue=="")
                    {
                        value = standard_bool_entry(option);
                    }
                    if (cfgvalue=="true")
                    {
                        value = true;
                    }
                    if (cfgvalue=="false")
                    {
                        value = false;
                    }
                }
            }
        }
        input_file.close();
    }else{
        value = standard_bool_entry(option);
    }
}

void CalculiXConfigFile::write_entry(std::string option, QString value)
{
    std::ofstream output_file;
    output_file.open(filepath.c_str(), std::ios_base::app);
    output_file << option << "=" << value.toStdString() << "\n";
    output_file.close();
}

void CalculiXConfigFile::write_num_entry(std::string option, int value)
{
    std::ofstream output_file;
    output_file.open(filepath.c_str(), std::ios_base::app);
    output_file << option << "=" << value  << "\n";
    output_file.close();
}

void CalculiXConfigFile::write_bool_entry(std::string option, bool value)
{
    std::ofstream output_file;
    output_file.open(filepath.c_str(), std::ios_base::app);
    if (value)
    {
        output_file << option << "=true\n";
    }else{
        output_file << option << "=false\n";
    }
    output_file.close();
}

QString CalculiXConfigFile::standard_entry(std::string option)
{
    QString standard_value = "";

    if (option == "PathSolver")
    {
        #ifdef WIN32
            standard_value = QString::fromStdString(componentpath) + "Solver/ccx_dynamic.exe";
        #else
            standard_value = QString::fromStdString(componentpath) + "Solver/ccx_2.22_MT";
            //standard_value = QString::fromStdString(componentpath) + "Solver/ccx_2.19_MT";
        #endif
    }else if(option == "PathCGX")
    {
        #ifdef WIN32
            standard_value = QString::fromStdString(componentpath) + "Postprocessor/CGX/cgx_STATIC.exe";
        #else
            standard_value = QString::fromStdString(componentpath) + "Postprocessor/CGX/cgx_2.22";
        #endif
    }else if(option == "PathParaView")
    {
        #ifdef WIN32
            standard_value = QString::fromStdString(componentpath) + "Postprocessor/ParaView/bin/paraview.exe";
        #else
            standard_value = QString::fromStdString(componentpath) + "Postprocessor/ParaView/bin/paraview";
        #endif
    }else if(option == "PathIcons")
    {
        #ifdef WIN32
            standard_value = QString::fromStdString(componentpath) + "Icons/";
        #else
            standard_value = QString::fromStdString(componentpath) + "Icons/";
        #endif
    }else if(option == "PathPythonInterface")
    {
        #ifdef WIN32
            standard_value = QString::fromStdString(componentpath);
        #else
            standard_value = QString::fromStdString(componentpath);
        #endif
    }else if(option == "PathMaterialLibrary")
    {
        #ifdef WIN32
            standard_value = QString::fromStdString(componentpath) + "libmaterial.hdf5";
        #else
            standard_value = QString::fromStdString(componentpath) + "libmaterial.hdf5";
        #endif
    }else if(option == "Version")
    {
        #ifdef WIN32
            standard_value = CCX_VERSION;
        #else
            standard_value = CCX_VERSION;
        #endif
    }
    return standard_value;
}

int CalculiXConfigFile::standard_num_entry(std::string option)
{
    int standard_value = 1;

    if (option == "SolverThreads")
    {
        standard_value = 8;
    }else if (option == "ConverterThreads")
    {
        standard_value = 8;
    }
    
    return standard_value;
}

bool CalculiXConfigFile::standard_bool_entry(std::string option)
{
    bool standard_value = true;

    if (option == "mSaveLoadedResults")
    {
        standard_value = true;
    }
    
    return standard_value;
}

bool CalculiXConfigFile::read_double_entry(const std::string& option, double& value)
{
  std::ifstream input(filepath);
  bool found = false;
  for (std::string line; std::getline(input, line);)
  {
    const std::string prefix = option + "=";
    if (line.compare(0, prefix.size(), prefix) != 0)
    {
      continue;
    }
    std::istringstream parser(line.substr(prefix.size()));
    parser.imbue(std::locale::classic());
    double candidate = 0;
    if (parser >> candidate)
    {
      parser >> std::ws;
      if (parser.eof() && std::isfinite(candidate) && candidate > 0)
      {
        value = candidate;
        found = true;
      }
    }
  }
  return found;
}

void CalculiXConfigFile::write_double_entry(const std::string& option, double value)
{
  std::ofstream output(filepath, std::ios_base::app);
  output.imbue(std::locale::classic());
  output << option << "=" << std::setprecision(17) << value << "\n";
}
