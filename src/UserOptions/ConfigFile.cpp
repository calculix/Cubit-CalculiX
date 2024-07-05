#include "ConfigFile.hpp"

#include "Broker.hpp"
#include "ComponentInfo.hpp"
#include "Mediator.hpp"
#include "CubitMessage.hpp"
#include <QString>
#include <fstream>
#include <iostream>

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
    log = " paths:\n " + output.toStdString() + " \n";
    PRINT_INFO("%s", log.c_str());

    CubitString cs(output.toStdString());
    std::vector<CubitString> paths;
    cs.tokenize(';', paths );
    for (size_t i = 0; i < paths.size(); i++)
    {
        log = " path: " + paths[i].str() + " \n";
        PRINT_INFO("%s", log.c_str());

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
                }
            }
        }
        input_file.close();
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
                }
                //std::cout << line.substr(option.length()+1) << "\n";
            }
        }
        input_file.close();
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