#include "ConfigFile.hpp"

#include "Broker.hpp"
#include "ComponentInfo.hpp"
#include "Mediator.hpp"
#include "CubitMessage.hpp"
#include <QString>
#include <fstream>
#include <iostream>
#include <algorithm>

ConfigFile::ConfigFile()
{
    filename = "ccx.cfg";
    componentpath = "";
    filepath = "";

    // search for correct component path
    Mediator* med = Broker::instance();
    QString output;
    std::string log;

    med->get_component_paths(output);
    //log = " paths:\n " + output.toStdString() + " \n";
    //PRINT_INFO("%s", log.c_str());

    CubitString cs(output.toStdString());
    std::vector<CubitString> paths;
    cs.tokenize(';', paths );
    for (size_t i = 0; i < paths.size(); i++)
    {
        //log = " path: " + paths[i].str() + " \n";
        //PRINT_INFO("%s", log.c_str());

        componentpath = paths[i].str() + "/";
        filepath = componentpath + "calculix_comp.ccl";
        std::ifstream input_file;
        input_file.open(filepath.c_str());
        
        if (input_file)
        {
            filepath = componentpath + filename;
            break;
        }
        /*
            log = " TRUE \n";
            PRINT_INFO("%s", log.c_str());
        }else{
            log = " FALSE \n";
            PRINT_INFO("%s", log.c_str());
        }
        */
    }

    //std::vector<ComponentInfo> info;
    //med->get_component_list(info);   
    /*
    for (size_t i = 0; i < info.size(); i++)
    {
        output = info[i].get_class();
        log = " class: " + output.toStdString() + " \n";
        output = info[i].get_module();
        log.append(" module: " + output.toStdString() + " \n");
        output = info[i].get_path();
        log.append(" path: " + output.toStdString() + " \n");
        output = info[i].get_state();
        log.append(" state: " + output.toStdString() + " \n");
        PRINT_INFO("%s", log.c_str());
    }
    */
    
}

ConfigFile::~ConfigFile()
{}

void ConfigFile::clear()
{
    std::ofstream output_file;
    output_file.open(filepath, std::ofstream::out | std::ofstream::trunc);
    output_file.close();
}

void ConfigFile::read_entry(std::string option, QString &value)
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

void ConfigFile::read_num_entry(std::string option, int &value)
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

void ConfigFile::write_entry(std::string option, QString value)
{
    std::ofstream output_file;
    output_file.open(filepath.c_str(), std::ios_base::app);
    output_file << option << "=" << value.toStdString() << "\n";
    output_file.close();
}

void ConfigFile::write_num_entry(std::string option, int value)
{
    std::ofstream output_file;
    output_file.open(filepath.c_str(), std::ios_base::app);
    output_file << option << "=" << value  << "\n";
    output_file.close();
}


QString ConfigFile::standard_entry(std::string option)
{
    QString standard_value = "";

    if (option == "PathSolver")
    {
        #ifdef WIN32
            standard_value = QString::fromStdString(componentpath) + "Solver/ccx_dynamic.exe";
        #else
            //standard_value = QString::fromStdString(componentpath) + "Solver/ccx_2.21_MT";
            standard_value = QString::fromStdString(componentpath) + "Solver/ccx_2.21";
        #endif
    }else if(option == "PathCGX")
    {
        #ifdef WIN32
            standard_value = QString::fromStdString(componentpath) + "Postprocessor/CGX/cgx_STATIC.exe";
        #else
            standard_value = QString::fromStdString(componentpath) + "Postprocessor/CGX/cgx_2.21";
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
    }

    return standard_value;
}

int ConfigFile::standard_num_entry(std::string option)
{
    int standard_value = 1;

    if (option == "SolverThreads")
    {
        standard_value = 8;
    }
    return standard_value;
}