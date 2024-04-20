#include "CoreDraw.hpp"
#include "CubitInterface.hpp"
#include <cmath>
#include "CalculiXCoreInterface.hpp"

CoreDraw::CoreDraw()
{}

CoreDraw::~CoreDraw()
{}


bool CoreDraw::init()
{
  if (is_initialized)
  {
    return false; // already initialized
  }else{
    CalculiXCoreInterface *ccx_iface = new CalculiXCoreInterface();
    is_initialized = true;  
    return true;
  }
}

std::vector<double> CoreDraw::rotate(std::vector<double> coord, std::vector<double> direction)
{
    std::vector<double> tmp_coord = {0,0,0};
    double pi = 3.14159265359;

    double alpha = 0;
    double beta = 0;
    double gamma = 0;

    if (direction[1]==0)
    {   
        if (direction[2]!=0)
        {
            alpha = copysign(pi/2,direction[2]);
        }
    }else{
        alpha = atan(direction[2]/direction[1]);
    }
    if (direction[0]==0)
    {
        if (direction[2]!=0)
        {
            beta = copysign(pi/2,direction[2]);
        }
    }else{
        beta = atan(direction[2]/direction[0]);
    }
    if (direction[0]==0)
    {
        if (direction[1]!=0)
        {
            gamma = copysign(pi/2,direction[1]);
        }
    }else{
        gamma = atan(direction[1]/direction[0]);
    }
    // rotation matrices
    std::vector<std::vector<double>> rx;
    rx.push_back({cos(alpha),-sin(alpha),0});
    rx.push_back({sin(alpha),cos(alpha),0});
    rx.push_back({0,0,1});
    std::vector<std::vector<double>> ry;
    ry.push_back({cos(beta),0,sin(beta)});
    ry.push_back({0,1,0});
    ry.push_back({-sin(beta),0,cos(beta)});
    std::vector<std::vector<double>> rz;
    rz.push_back({1,0,0});
    rz.push_back({0,cos(gamma),-sin(gamma)});
    rz.push_back({0,sin(gamma),cos(gamma)});

    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {
            tmp_coord[i] += coord[ii]*rx[i][ii];
        }
    }
    coord = tmp_coord;
    
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {
            tmp_coord[i] += coord[ii]*ry[i][ii];
        }
    }
    coord = tmp_coord;

    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {
            tmp_coord[i] += coord[ii]*rz[i][ii];
        }
    }

    return tmp_coord;
}

bool CoreDraw::draw_arrow(std::vector<double> center, std::vector<double> direction, std::string color, double size)
{
    std::vector<std::string> commands;
    std::string cmd;
    int npolygon = 10;
    double radius = 0.1;
    double pi = 3.14159265359;
    std::vector<std::vector<double>> polygon_coords;
    std::vector<double> arrow_end_coord(3);

    arrow_end_coord[0] = center[0] - direction[0];
    arrow_end_coord[1] = center[1] - direction[1];
    arrow_end_coord[2] = center[2] - direction[2];

    for (size_t i = 0; i < npolygon; i++)
    {
        std::vector<double> coord(3);
        coord[0]=-0.3;
        coord[1]=radius*cos(2*pi/npolygon*i);
        coord[2]=radius*sin(2*pi/npolygon*i);
        //coord = rotate(coord,direction);
        polygon_coords.push_back(coord);
    }

    for (size_t i = 0; i < npolygon; i++)
    {
        if (i==npolygon-1)
        {
            commands.push_back("draw polygon location pos " + std::to_string(center[0]) + " " + std::to_string(center[1]) + " " + std::to_string(center[2]) + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]) + " location pos " + std::to_string(polygon_coords[0][0]) + " " + std::to_string(polygon_coords[0][1]) + " " + std::to_string(polygon_coords[0][2]) + " color " + color + " no_flush");
        }else{
            commands.push_back("draw polygon location pos " + std::to_string(center[0]) + " " + std::to_string(center[1]) + " " + std::to_string(center[2]) + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]) + " location pos " + std::to_string(polygon_coords[i+1][0]) + " " + std::to_string(polygon_coords[i+1][1]) + " " + std::to_string(polygon_coords[i+1][2]) + " color " + color + " no_flush");
        }
    }

    cmd = "draw polygon";

    for (size_t i = 0; i < npolygon; i++)
    {
        cmd = cmd + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]);
    }

    cmd = cmd + " color " + color + " no_flush";
    commands.push_back(cmd);

    commands.push_back("draw line location pos " + std::to_string(arrow_end_coord[0]) + " " + std::to_string(arrow_end_coord[1]) + " " + std::to_string(arrow_end_coord[2]) + " location pos " + std::to_string(center[0]) + " " + std::to_string(center[1]) + " " + std::to_string(center[2]) + " color " + color + " no_flush");
    commands.push_back("graphics flush"); 
    
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        CubitInterface::silent_cmd(commands[i].c_str());
    }

    return true;
}

bool CoreDraw::draw_all()
{
    std::vector<double> center = {0,0,0};
    std::vector<double> direction = {0,1,0};
    std::string color = "red";
    double size = 1;

    this->draw_arrow(center, direction, color, size);
    return true;
}