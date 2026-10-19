#include "CoreDraw.hpp"
#include "CubitInterface.hpp"
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"
#include "GifAnimation.hpp"
#include "CubitColorTable.hpp"
#include "CubitVector.hpp"
#include "GCubitModelSet.hpp"
#include "GGenericModel.hpp"
#include "GView.hpp"
#include "GWindowFactory.hpp"

#include <algorithm>
#include <limits>
#include <map>
#include <sstream>
#include <cmath>
#include <set>

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QImage>
#include <QStringList>
#include <QTemporaryDir>
#include <QSaveFile>

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
    ccx_iface = new CalculiXCoreInterface();
    is_initialized = true;  
    return true;
  }
}

std::vector<double> CoreDraw::rotate(std::vector<double> coord, std::vector<double> vec_a, std::vector<double> vec_b)
{
    // https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
    
    // if vector is parallel and the same direction return coord    
    if ((vec_a[0]==vec_b[0])&&(vec_a[1]==vec_b[1])&&(vec_a[2]==vec_b[2]))
    {
        return coord;
    }

    // if vector is parallel and the opposite direction return inverse coord
    if ((vec_a[0]==-vec_b[0])&&(vec_a[1]==-vec_b[1])&&(vec_a[2]==-vec_b[2]))
    {
        coord[0] = coord[0] * (-1);
        coord[1] = coord[1] * (-1);
        coord[2] = coord[2] * (-1);
        return coord;
    }
    
    std::string log;
    std::vector<double> tmp_coord = {0,0,0};
    std::vector<double> vec_k = {0,0,0};
    std::vector<std::vector<double>> mat_r;
    mat_r.push_back({1,0,0});
    mat_r.push_back({0,1,0});
    mat_r.push_back({0,0,1});

    double sinus = 0;
    double cosinus = 0;

    //first compute the unit vectors
    vec_a = unit_vector(vec_a);
    vec_b = unit_vector(vec_b);
    
    //define rotation axis
    vec_k = cross_product(vec_a, vec_b);
    vec_k = unit_vector(vec_k);
    
    //sinus cosinus
    sinus = magnitude(cross_product(vec_a, vec_b));
    cosinus = inner_product(vec_a, vec_b);

    //cross product matrix
    std::vector<std::vector<double>> mat_k = cross_product_matrix(vec_k);
    std::vector<std::vector<double>> mat_k2 = mult_matrix(mat_k,mat_k);

    // rotation matrix

    mat_r = add_matrix(mat_r, mult_matrix_scalar(mat_k,sinus));
    mat_r = add_matrix(mat_r, mult_matrix_scalar(mat_k2, 1 - cosinus));

    tmp_coord = mult_matrix_vector(mat_r,coord);

    return tmp_coord;
}

std::vector<double> CoreDraw::rotate_about_axis(std::vector<double> coord, std::vector<double> axis, double angle)
{
    // https://de.wikipedia.org/wiki/Drehmatrix
    
    std::vector<double> n_coord = this->unit_vector(coord);
    std::vector<double> n_axis = this->unit_vector(axis);
    std::vector<double> tmp_coord = {0,0,0};
    std::vector<std::vector<double>> mat_r;
    mat_r.push_back({0,0,0});
    mat_r.push_back({0,0,0});
    mat_r.push_back({0,0,0});
    double pi = 3.14159265359;
    double sinus = sin(angle*pi/180);
    double cosinus = cos(angle*pi/180);

    // if vector is parallel and the same direction with coord return coord    
    if ((n_coord[0]==n_axis[0])&&(n_coord[1]==n_axis[1])&&(n_coord[2]==n_axis[2]))
    {
        return coord;
    }

    // if vector is parallel and the opposite direction with coord return inverse coord
    if ((n_coord[0]==-n_axis[0])&&(n_coord[1]==-n_axis[1])&&(n_coord[2]==-n_axis[2]))
    {
        coord[0] = coord[0] * (-1);
        coord[1] = coord[1] * (-1);
        coord[2] = coord[2] * (-1);
        return coord;
    }
    
    // rotation matrix
    mat_r[0][0] = n_axis[0]*n_axis[0]*(1-cosinus)+cosinus;
    mat_r[0][1] = n_axis[0]*n_axis[1]*(1-cosinus)-n_axis[2]*sinus;
    mat_r[0][2] = n_axis[0]*n_axis[2]*(1-cosinus)+n_axis[1]*sinus;
    mat_r[1][0] = n_axis[0]*n_axis[1]*(1-cosinus)+n_axis[2]*sinus;
    mat_r[1][1] = n_axis[1]*n_axis[1]*(1-cosinus)+cosinus;
    mat_r[1][2] = n_axis[1]*n_axis[2]*(1-cosinus)-n_axis[0]*sinus;
    mat_r[2][0] = n_axis[0]*n_axis[2]*(1-cosinus)-n_axis[1]*sinus;
    mat_r[2][1] = n_axis[1]*n_axis[2]*(1-cosinus)+n_axis[0]*sinus;
    mat_r[2][2] = n_axis[2]*n_axis[2]*(1-cosinus)+cosinus;

    tmp_coord = mult_matrix_vector(mat_r,coord);

    return tmp_coord;
}

std::vector<double> CoreDraw::unit_vector(std::vector<double> input)
{
    std::vector<double> output(3);
    double vec_magnitude = magnitude(input);

    output[0] = input[0]/vec_magnitude;
    output[1] = input[1]/vec_magnitude;
    output[2] = input[2]/vec_magnitude;

    return output;
}


double CoreDraw::magnitude(std::vector<double> input)
{
    double magnitude = sqrt(pow(input[0],2) + pow(input[1],2) + pow(input[2],2));

    return magnitude;
}

std::vector<double> CoreDraw::cross_product(std::vector<double> vec_a, std::vector<double> vec_b)
{
    std::vector<double> output(3);

    output[0] = vec_a[1]*vec_b[2]-vec_a[2]*vec_b[1];
    output[1] = vec_a[2]*vec_b[0]-vec_a[0]*vec_b[2];
    output[2] = vec_a[0]*vec_b[1]-vec_a[1]*vec_b[0];

    return output;
}

std::vector<std::vector<double>> CoreDraw::cross_product_matrix(std::vector<double> vec)
{
    std::vector<std::vector<double>> output;

    output.push_back({0,-vec[2],vec[1]});
    output.push_back({vec[2],0,-vec[0]});
    output.push_back({-vec[1],vec[0],0});

    return output;
}

double CoreDraw::inner_product(std::vector<double> vec_a, std::vector<double> vec_b)
{
    double output = 0;

    for (size_t i = 0; i < 3; i++)
    {
        output += vec_a[i]*vec_b[i];
    }

    return output;
}

std::vector<double> CoreDraw::mult_vector_scalar(std::vector<double> vec, double scalar)
{
    std::vector<double> output(3);

    for (size_t i = 0; i < 3; i++)
    {
        output[i] = vec[i]*scalar;
    }

    return output;
}

std::vector<std::vector<double>> CoreDraw::mult_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b)
{
    std::vector<std::vector<double>> output;

    output.push_back({0,0,0});
    output.push_back({0,0,0});
    output.push_back({0,0,0});

    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {   
            for (size_t iii = 0; iii < 3; iii++)
            {
                output[i][ii]+=mat_a[i][iii]*mat_b[iii][ii];
            }
        }
    }
    return output;    
}


std::vector<std::vector<double>> CoreDraw::mult_matrix_scalar(std::vector<std::vector<double>> mat_a, double scalar)
{
    std::vector<std::vector<double>> output;

    output.push_back({0,0,0});
    output.push_back({0,0,0});
    output.push_back({0,0,0});

    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {  
            output[i][ii] = mat_a[i][ii]*scalar;
        }
    }
    return output;    
}

std::vector<std::vector<double>> CoreDraw::add_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b)
{
    std::vector<std::vector<double>> output;

    output.push_back({0,0,0});
    output.push_back({0,0,0});
    output.push_back({0,0,0});

    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {   
            output[i][ii] =mat_a[i][ii] + mat_b[i][ii];
        }
    }
    return output;    
}

std::vector<double> CoreDraw::mult_matrix_vector(std::vector<std::vector<double>> mat, std::vector<double> vec)
{
    std::vector<double> output(3);

    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {   
            output[i]+=mat[i][ii]*vec[ii];
        }
    }
    return output;
}

bool CoreDraw::draw_arrow(std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size)
{
    //skip if zero vector
    if (direction[0]==0 && direction[1]==0 && direction[2]==0)
    {
        return true;
    }

    //delete sign of zero
    for (size_t i = 0; i < direction.size(); i++)
    {
        if (direction[i]==0)
        {
            direction[i]=0;
        }
    }
    
    int npolygon = 10;
    double radius = 0.1;
    double pi = 3.14159265359;
    std::vector<std::vector<double>> polygon_coords;
    std::vector<double> arrow_start_coord(3);
    std::vector<double> arrow_end_coord(3);
    std::vector<double> arrow_direction = unit_vector(direction); //compute unit vector for direction

    arrow_start_coord[0] = start_point[0];
    arrow_start_coord[1] = start_point[1];
    arrow_start_coord[2] = start_point[2];

    arrow_end_coord[0] = start_point[0] + arrow_direction[0]*size;
    arrow_end_coord[1] = start_point[1] + arrow_direction[1]*size;
    arrow_end_coord[2] = start_point[2] + arrow_direction[2]*size;

    //arrow tip and rotation
    for (size_t i = 0; i < npolygon; i++)
    {
        std::vector<double> coord(3);
        coord[0]= 0.3*size;
        coord[1]= radius*cos(2*pi/npolygon*i)*size;
        coord[2]= radius*sin(2*pi/npolygon*i)*size;
        coord = rotate(coord,{1.,0.,0.},arrow_direction);
        polygon_coords.push_back(coord);
    }
    //translate arrow head to starting point
    for (size_t i = 0; i < npolygon; i++)
    {
        polygon_coords[i][0] += arrow_start_coord[0];
        polygon_coords[i][1] += arrow_start_coord[1];
        polygon_coords[i][2] += arrow_start_coord[2];
    }

    // translate arrow if it should not point on starting point
    // we translate away the full arrow length in arrow direction
    if (from_start_point)
    {
        arrow_start_coord[0] = start_point[0] - arrow_direction[0]*size;
        arrow_start_coord[1] = start_point[1] - arrow_direction[1]*size;
        arrow_start_coord[2] = start_point[2] - arrow_direction[2]*size;

        arrow_end_coord[0] = start_point[0];
        arrow_end_coord[1] = start_point[1];
        arrow_end_coord[2] = start_point[2];

        for (size_t i = 0; i < npolygon; i++)
        {
            polygon_coords[i][0] -= arrow_end_coord[0] - arrow_start_coord[0];
            polygon_coords[i][1] -= arrow_end_coord[1] - arrow_start_coord[1];
            polygon_coords[i][2] -= arrow_end_coord[2] - arrow_start_coord[2];
        }
    }
    
    // Draw arrow head.
    for (size_t i = 0; i < polygon_coords.size(); i++)
    {
        const size_t next = (i + 1) % polygon_coords.size();
        if (!graphics_polygon({arrow_start_coord, polygon_coords[i], polygon_coords[next]}, color))
        {
            return false;
        }
    }

    // Draw arrow-head base and shaft.
    if (!graphics_polygon(polygon_coords, color))
    {
        return false;
    }
    if (!graphics_line(arrow_end_coord, arrow_start_coord, color))
    {
        return false;
    }
    if (!graphics_flush())
    {
        return false;
    }

    return true;
}

bool CoreDraw::draw_arrow_flat(std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size)
{
    //skip if zero vector
    if (direction[0]==0 && direction[1]==0 && direction[2]==0)
    {
        return true;
    }
    
    std::vector<std::string> commands;
    std::string cmd;
    int npolygon = 10;
    double radius = 0.1;
    double pi = 3.14159265359;
    std::vector<std::vector<double>> polygon_coords;
    std::vector<double> arrow_start_coord(3);
    std::vector<double> arrow_end_coord(3);
    std::vector<double> arrow_direction = unit_vector(direction); //compute unit vector for direction

    arrow_start_coord[0] = start_point[0];
    arrow_start_coord[1] = start_point[1];
    arrow_start_coord[2] = start_point[2];

    arrow_end_coord[0] = start_point[0] + arrow_direction[0]*size;
    arrow_end_coord[1] = start_point[1] + arrow_direction[1]*size;
    arrow_end_coord[2] = start_point[2] + arrow_direction[2]*size;

    //arrow tip and rotation
    for (size_t i = 0; i < npolygon; i++)
    {
        std::vector<double> coord(3);
        coord[0]= 0.1*size;
        coord[1]= radius*cos(2*pi/npolygon*i)*size*1.1;
        coord[2]= radius*sin(2*pi/npolygon*i)*size*1.1;
        coord = rotate(coord,{1.,0.,0.},arrow_direction);
        polygon_coords.push_back(coord);
    }
    //translate arrow head to starting point
    for (size_t i = 0; i < npolygon; i++)
    {
        polygon_coords[i][0] += arrow_start_coord[0];
        polygon_coords[i][1] += arrow_start_coord[1];
        polygon_coords[i][2] += arrow_start_coord[2];
    }

    // translate arrow if it should not point on starting point
    // we translate away the full arrow length in arrow direction
    if (from_start_point)
    {
        arrow_start_coord[0] = start_point[0] - arrow_direction[0]*size;
        arrow_start_coord[1] = start_point[1] - arrow_direction[1]*size;
        arrow_start_coord[2] = start_point[2] - arrow_direction[2]*size;

        arrow_end_coord[0] = start_point[0];
        arrow_end_coord[1] = start_point[1];
        arrow_end_coord[2] = start_point[2];

        for (size_t i = 0; i < npolygon; i++)
        {
            polygon_coords[i][0] -= arrow_end_coord[0] - arrow_start_coord[0];
            polygon_coords[i][1] -= arrow_end_coord[1] - arrow_start_coord[1];
            polygon_coords[i][2] -= arrow_end_coord[2] - arrow_start_coord[2];
        }
    }
    
    // draw arrow head
    for (size_t i = 0; i < npolygon; i++)
    {
        if (i==npolygon-1)
        {
            commands.push_back("draw polygon location pos " + ccx_iface->to_string_scientific(arrow_start_coord[0]) + " " + ccx_iface->to_string_scientific(arrow_start_coord[1]) + " " + ccx_iface->to_string_scientific(arrow_start_coord[2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[0][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[0][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[0][2]) + " color " + color + " no_flush");
        }else{
            commands.push_back("draw polygon location pos " + ccx_iface->to_string_scientific(arrow_start_coord[0]) + " " + ccx_iface->to_string_scientific(arrow_start_coord[1]) + " " + ccx_iface->to_string_scientific(arrow_start_coord[2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i+1][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i+1][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i+1][2]) + " color " + color + " no_flush");
        }
    }
    cmd = "draw polygon";
    for (size_t i = 0; i < npolygon; i++)
    {
        cmd = cmd + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][2]);
    }

    cmd = cmd + " color " + color + " no_flush";
    commands.push_back(cmd);

    // draw arrow line
    commands.push_back("draw line location pos " + ccx_iface->to_string_scientific(arrow_end_coord[0]) + " " + ccx_iface->to_string_scientific(arrow_end_coord[1]) + " " + ccx_iface->to_string_scientific(arrow_end_coord[2]) + " location pos " + ccx_iface->to_string_scientific(arrow_start_coord[0]) + " " + ccx_iface->to_string_scientific(arrow_start_coord[1]) + " " + ccx_iface->to_string_scientific(arrow_start_coord[2]) + " color " + color + " no_flush");
    commands.push_back("graphics flush"); 
    
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        ccx_iface->silent_cmd(commands[i]);
    }

    return true;
}

bool CoreDraw::draw_dof(std::vector<double> coord, int dof, std::string color, double size)
{
    std::vector<std::string> commands;
    std::string cmd;
    int npolygon = 4;
    double radius = 0.1;
    double pi = 3.14159265359;
    std::vector<std::vector<double>> polygon_coords;
    std::vector<double> draw_coord(3);
    draw_coord = coord;

    if (dof>3)
    {
        npolygon = 10;
    }
    
    if (dof==1)
    {
        //arrow tip
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= -0.2*size;
            tmp_coord[1]= radius*cos(2*pi/npolygon*i)*size;
            tmp_coord[2]= radius*sin(2*pi/npolygon*i)*size;
            polygon_coords.push_back(tmp_coord);
        }
        //translate arrow head to starting point
        for (size_t i = 0; i < npolygon; i++)
        {
            draw_coord[0] = coord[0] + -0.05*size;
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else if (dof==2)
    {
        //arrow tip
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= radius*sin(2*pi/npolygon*i)*size;
            tmp_coord[1]= -0.2*size;
            tmp_coord[2]= radius*cos(2*pi/npolygon*i)*size;
            polygon_coords.push_back(tmp_coord);
        }
        //translate arrow head to starting point
        for (size_t i = 0; i < npolygon; i++)
        {
            draw_coord[1] = coord[1] + -0.05*size;
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else if (dof==3)
    {
        //arrow tip
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= radius*cos(2*pi/npolygon*i)*size;
            tmp_coord[1]= radius*sin(2*pi/npolygon*i)*size;
            tmp_coord[2]= -0.2*size;
            polygon_coords.push_back(tmp_coord);
        }
        //translate arrow head to starting point
        for (size_t i = 0; i < npolygon; i++)
        {
            draw_coord[2] = coord[2] + -0.05*size;
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else if (dof==4)
    {
        //arrow tip
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= -0.1*size;
            tmp_coord[1]= radius*cos(2*pi/npolygon*i)*size;
            tmp_coord[2]= radius*sin(2*pi/npolygon*i)*size;
            polygon_coords.push_back(tmp_coord);
        }
        //translate arrow head to starting point
        for (size_t i = 0; i < npolygon; i++)
        {
            draw_coord[0] = coord[0] + -0.25*size;
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else if (dof==5)
    {
        //arrow tip
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= radius*sin(2*pi/npolygon*i)*size;
            tmp_coord[1]= -0.1*size;
            tmp_coord[2]= radius*cos(2*pi/npolygon*i)*size;
            polygon_coords.push_back(tmp_coord);
        }
        //translate arrow head to starting point
        for (size_t i = 0; i < npolygon; i++)
        {
            draw_coord[1] = coord[1] + -0.25*size;
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else if (dof==6)
    {
        //arrow tip
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= radius*cos(2*pi/npolygon*i)*size;
            tmp_coord[1]= radius*sin(2*pi/npolygon*i)*size;
            tmp_coord[2]= -0.1*size;
            polygon_coords.push_back(tmp_coord);
        }
        //translate arrow head to starting point
        for (size_t i = 0; i < npolygon; i++)
        {
            draw_coord[2] = coord[2] + -0.25*size;
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else if (dof==11)
    {
        /* code */
    }else{
        std::string log = "dof " + std::to_string(dof) + " not supported";
        PRINT_INFO("%s", log.c_str());        
    }
        
    if (dof < 7)
    {    
        // draw dof 1,2,3,4,5,6
        for (size_t i = 0; i < npolygon; i++)
        {
            if (i==npolygon-1)
            {
                commands.push_back("draw polygon location pos " + ccx_iface->to_string_scientific(draw_coord[0]) + " " + ccx_iface->to_string_scientific(draw_coord[1]) + " " + ccx_iface->to_string_scientific(draw_coord[2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[0][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[0][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[0][2]) + " color " + color + " no_flush");
            }else{
                commands.push_back("draw polygon location pos " + ccx_iface->to_string_scientific(draw_coord[0]) + " " + ccx_iface->to_string_scientific(draw_coord[1]) + " " + ccx_iface->to_string_scientific(draw_coord[2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i+1][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i+1][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i+1][2]) + " color " + color + " no_flush");
            }
        }
        cmd = "draw polygon";
        for (size_t i = 0; i < npolygon; i++)
        {
            cmd = cmd + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][2]);
        }

        cmd = cmd + " color " + color + " no_flush";
        commands.push_back(cmd);
        // draw line
        commands.push_back("draw line location pos " + ccx_iface->to_string_scientific(draw_coord[0]) + " " + ccx_iface->to_string_scientific(draw_coord[1]) + " " + ccx_iface->to_string_scientific(draw_coord[2]) + " location pos " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " color " + color + " no_flush");
    }else if (dof==11)
    {
        commands.push_back("draw location " +  ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " color " + color + " no_flush");
    }
    commands.push_back("graphics flush");
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        ccx_iface->silent_cmd(commands[i]);
    }

    return true;
}

bool CoreDraw::draw_equation_node(std::vector<double> coord, int dof, std::string color, double size)
{
    std::vector<std::string> commands;
    std::string cmd;
    int npolygon = 10;
    double radius = 0.1;
    double pi = 3.14159265359;
    std::vector<std::vector<double>> polygon_coords;
    std::vector<double> draw_coord(3);
    draw_coord = coord;

    if (dof>3)
    {
        npolygon = 10;
    }

    if (color=="")
    {
        if (dof==1)
        {
            color = "red";
        }else if (dof==2)
        {
            color = "green";
        }else if (dof==3)
        {
            color = "blue";
        }else{
            color = "yellow";
        }
    }
    
    
    if (dof==1)
    {
        //polygon coords around global coordinate center
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= 0;
            tmp_coord[1]= radius*cos(2*pi/npolygon*i)*size;
            tmp_coord[2]= radius*sin(2*pi/npolygon*i)*size;
            polygon_coords.push_back(tmp_coord);
        }
        //translate polygon to nodal coords
        for (size_t i = 0; i < npolygon; i++)
        {
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else if (dof==2)
    {
        //polygon coords around global coordinate center
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= radius*sin(2*pi/npolygon*i)*size;
            tmp_coord[1]= 0;
            tmp_coord[2]= radius*cos(2*pi/npolygon*i)*size;
            polygon_coords.push_back(tmp_coord);
        }
        //translate polygon to nodal coords
        for (size_t i = 0; i < npolygon; i++)
        {
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else if (dof==3)
    {
        //polygon coords around global coordinate center
        for (size_t i = 0; i < npolygon; i++)
        {
            std::vector<double> tmp_coord(3);
            tmp_coord[0]= radius*cos(2*pi/npolygon*i)*size;
            tmp_coord[1]= radius*sin(2*pi/npolygon*i)*size;
            tmp_coord[2]= 0;
            polygon_coords.push_back(tmp_coord);
        }
        //translate polygon to nodal coords
        for (size_t i = 0; i < npolygon; i++)
        {
            polygon_coords[i][0] += draw_coord[0];
            polygon_coords[i][1] += draw_coord[1];
            polygon_coords[i][2] += draw_coord[2];
        }
    }else{
        std::string log = "dof " + std::to_string(dof) + " not supported";
        PRINT_INFO("%s", log.c_str());        
    }
        
    if (dof < 4)
    {    
        // draw dof 1,2,3
        for (size_t i = 0; i < npolygon; i++)
        {
            if (i==npolygon-1)
            {
                commands.push_back("draw polygon location pos " + ccx_iface->to_string_scientific(draw_coord[0]) + " " + ccx_iface->to_string_scientific(draw_coord[1]) + " " + ccx_iface->to_string_scientific(draw_coord[2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[0][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[0][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[0][2]) + " color " + color + " no_flush");
            }else{
                commands.push_back("draw polygon location pos " + ccx_iface->to_string_scientific(draw_coord[0]) + " " + ccx_iface->to_string_scientific(draw_coord[1]) + " " + ccx_iface->to_string_scientific(draw_coord[2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i][2]) + " location pos " + ccx_iface->to_string_scientific(polygon_coords[i+1][0]) + " " + ccx_iface->to_string_scientific(polygon_coords[i+1][1]) + " " + ccx_iface->to_string_scientific(polygon_coords[i+1][2]) + " color " + color + " no_flush");
            }
        }
    }

    commands.push_back("graphics flush");
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        ccx_iface->silent_cmd(commands[i]);
    }

    return true;
}  

bool CoreDraw::draw_label(std::vector<double> coord, std::string color, double size)
{
    //locate location 0 0 0 "test"
    return true;
}

bool CoreDraw::draw_load_force(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_force(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_arrow({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {-draw_data[i][3],-draw_data[i][4],-draw_data[i][5]}, false, "green", size);
        i+=1;
        draw_arrow_flat({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {-draw_data[i][3],-draw_data[i][4],-draw_data[i][5]}, false, "seagreen", size);
    }

    return true;
}

bool CoreDraw::draw_load_pressure(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_pressure(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_arrow({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "lightskyblue", size);
    }

    return true;
}

bool CoreDraw::draw_load_heatflux(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_heatflux(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_arrow({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "purple", size);
    }

    return true;
}

bool CoreDraw::draw_load_gravity(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_gravity(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_arrow({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "brown", size);
    }

    return true;
}

bool CoreDraw::draw_load_centrifugal(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_centrifugal(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_arrow({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "khaki", size);
    }

    return true;
}

bool CoreDraw::draw_load_trajectory(int id, double size)
{
    if (ccx_iface->loadstrajectory_get_load_type(id)=="HEATFLUX")
    {
        std::vector<int> node_ids;
        node_ids = ccx_iface->loadstrajectory_heatflux_get_node_ids(id);
        std::vector<std::vector<double>> hit_coordinates;
        hit_coordinates = ccx_iface->loadstrajectory_heatflux_get_hit_coordinates(id);
        std::vector<std::vector<std::vector<int>>> face_ids;
        face_ids = ccx_iface->loadstrajectory_heatflux_get_draw_face_ids(id);
        std::vector<std::vector<double>> times;
        times = ccx_iface->loadstrajectory_heatflux_get_times(id);
        bool switch_color = true;

        // draw faces
        for (size_t i = 0; i < face_ids.size(); i++) //loop over nodes
        {
            for (size_t ii = 0; ii < face_ids[i].size(); ii++) //loop over radius
            {
                if (face_ids[i][ii].size()!=0)
                {
                    std::string cmd = "draw face ";
                    for (size_t iii = 0; iii < face_ids[i][ii].size(); iii++)
                    {
                        cmd.append(std::to_string(face_ids[i][ii][iii]) + " ");
                    }
                    cmd.append("color " + get_color(int(ii)) + " add");
                    
                    ccx_iface->silent_cmd(cmd);
                    //ccx_iface->cmd(cmd);
                }
            }
        }

        for (size_t i = 0; i < hit_coordinates.size(); i++)
        {
            if (hit_coordinates[i].size()>0)
            {
                std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[i]);
                if (switch_color)
                {
                    ccx_iface->silent_cmd("draw location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " color red");
                    ccx_iface->silent_cmd("draw line location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " location " + ccx_iface->to_string_scientific(hit_coordinates[i][0]) + " " + ccx_iface->to_string_scientific(hit_coordinates[i][1]) + " " + ccx_iface->to_string_scientific(hit_coordinates[i][2]) +  " color red");
                    //switch_color = false;
                }else{
                    ccx_iface->silent_cmd("draw location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " color blue");
                    ccx_iface->silent_cmd("draw line location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " location " + ccx_iface->to_string_scientific(hit_coordinates[i][0]) + " " + ccx_iface->to_string_scientific(hit_coordinates[i][1]) + " " + ccx_iface->to_string_scientific(hit_coordinates[i][2]) +  " color blue");
                    //switch_color = true;
                }
            }
        }
    
        /*
        for (size_t i = 0; i < times.size(); i++)
        {
            if (times[i].size()>0)
            {
                std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[i]);
                std::string label = "locate location " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " ";
                label.append("\" t_begin " + ccx_iface->to_string_scientific(times[i][0]) + ",t_end " + ccx_iface->to_string_scientific(times[i][1]) + "\"");
                ccx_iface->cmd(label);
            }
        }
        */
        if (times.size()>1)
        {
            std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[0]);
            std::string label = "locate location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " ";
            label.append("\" t_begin " + ccx_iface->to_string_scientific(times[0][0]) + "\"");
            ccx_iface->silent_cmd(label);

            coord = CubitInterface::get_nodal_coordinates(node_ids[node_ids.size()-1]);
            label = "locate location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " ";
            label.append("\" t_end " + ccx_iface->to_string_scientific(times[times.size()-1][1]) + "\"");
            ccx_iface->silent_cmd(label);
        }

        return true;
    }

    if (ccx_iface->loadstrajectory_get_load_type(id)=="BODYHEATFLUXSPHERE")
    {
        std::vector<int> node_ids;
        node_ids = ccx_iface->loadstrajectory_bodyheatfluxsphere_get_node_ids(id);
        std::vector<std::vector<double>> hit_coordinates;
        hit_coordinates = ccx_iface->loadstrajectory_bodyheatfluxsphere_get_hit_coordinates(id);
        std::vector<std::vector<std::vector<int>>> element_ids;
        element_ids = ccx_iface->loadstrajectory_bodyheatfluxsphere_get_draw_element_ids(id);
        std::vector<std::vector<double>> times;
        times = ccx_iface->loadstrajectory_bodyheatfluxsphere_get_times(id);
        bool switch_color = true;

                // draw elements
        for (size_t i = 0; i < element_ids.size(); i++) //loop over nodes
        {
            for (size_t ii = 0; ii < element_ids[i].size(); ii++) //loop over radius
            {
                if (element_ids[i][ii].size()!=0)
                {
                    std::string cmd = "draw element ";
                    for (size_t iii = 0; iii < element_ids[i][ii].size(); iii++)
                    {
                        cmd.append(std::to_string(element_ids[i][ii][iii]) + " ");
                    }
                    cmd.append("color " + get_color(int(ii)) + " add");
                    
                    ccx_iface->silent_cmd(cmd);
                    //ccx_iface->cmd(cmd);
                }
            }
        }

        for (size_t i = 0; i < hit_coordinates.size(); i++)
        {
            if (hit_coordinates[i].size()>0)
            {
                std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[i]);
                if (switch_color)
                {
                    ccx_iface->silent_cmd("draw location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " color red");
                    ccx_iface->silent_cmd("draw line location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " location " + ccx_iface->to_string_scientific(hit_coordinates[i][0]) + " " + ccx_iface->to_string_scientific(hit_coordinates[i][1]) + " " + ccx_iface->to_string_scientific(hit_coordinates[i][2]) +  " color red");
                    //switch_color = false;
                }else{
                    ccx_iface->silent_cmd("draw location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " color blue");
                    ccx_iface->silent_cmd("draw line location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " location " + ccx_iface->to_string_scientific(hit_coordinates[i][0]) + " " + ccx_iface->to_string_scientific(hit_coordinates[i][1]) + " " + ccx_iface->to_string_scientific(hit_coordinates[i][2]) +  " color blue");
                    //switch_color = true;
                }
            }
        }
        
        /*
        for (size_t i = 0; i < times.size(); i++)
        {
            if (times[i].size()>0)
            {
                std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[i]);
                std::string label = "locate location " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " ";
                label.append("\" t_begin " + ccx_iface->to_string_scientific(times[i][0]) + ",t_end " + ccx_iface->to_string_scientific(times[i][1]) + "\"");
                ccx_iface->cmd(label);
            }
        }
        */
        if (times.size()>1)
        {
            std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[0]);
            std::string label = "locate location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " ";
            label.append("\" t_begin " + ccx_iface->to_string_scientific(times[0][0]) + "\"");
            ccx_iface->silent_cmd(label);

            coord = CubitInterface::get_nodal_coordinates(node_ids[node_ids.size()-1]);
            label = "locate location " + ccx_iface->to_string_scientific(coord[0]) + " " + ccx_iface->to_string_scientific(coord[1]) + " " + ccx_iface->to_string_scientific(coord[2]) + " ";
            label.append("\" t_end " + ccx_iface->to_string_scientific(times[times.size()-1][1]) + "\"");
            ccx_iface->silent_cmd(label);
        }

        return true;
    }

    return false;
}

bool CoreDraw::draw_load_film(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_film(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_arrow({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "indianred", size);
    }

    return true;
}

bool CoreDraw::draw_load_radiation(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_radiation(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_arrow({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {draw_data[i][3],draw_data[i][4],draw_data[i][5]}, false, "tomato", size);
    }

    return true;
}

bool CoreDraw::draw_load_surface_traction(int id, double size)
{
    //std::string log = "Surface Traction ID " + std::to_string(id) + "  drawn with size " + std::to_string(size) +"\n";
    //PRINT_INFO("%s", log.c_str());
    
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_surface_traction(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_arrow({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, {-draw_data[i][3],-draw_data[i][4],-draw_data[i][5]}, true, "darkgreen", size);
    }

    return true;
}


bool CoreDraw::draw_load_bodyheatflux(int id, double size)
{
    std::vector<std::vector<int>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_load_bodyheatflux(id);
    std::vector<std::string> commands;
 
    
    for (size_t i = 0; i < draw_data[0].size(); i++)
    {
        commands.push_back("draw block " + std::to_string(draw_data[0][i]) + " color purple add");
    }
    
    for (size_t i = 0; i < draw_data[1].size(); i++)
    {

        commands.push_back("draw element " + std::to_string(draw_data[1][i]) + " color purple add");
    }
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        ccx_iface->silent_cmd(commands[i]);
    }

    return true;
}

bool CoreDraw::draw_bc_displacement(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_bc_displacement(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_dof({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, draw_data[i][3], "coral", size);
    }

    return true;
}

bool CoreDraw::draw_bc_temperature(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_bc_temperature(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_dof({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, draw_data[i][3], "red", size);
    }

    return true;
}

bool CoreDraw::draw_orientation(int id, double size)
{
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_orientation(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        // center point coordinates
        std::vector<double> center;
        center = draw_data[i+1];
        // axis
        std::vector<double> x_hat;
        std::vector<double> y_hat;
        std::vector<double> z_hat;

        //check if rectangular or cylindrical system
        if (draw_data[i][0]==1.0)
        {
            x_hat = draw_data[i+2];
            z_hat = this->cross_product(x_hat,draw_data[i+3]);
            y_hat = this->cross_product(x_hat,z_hat);
            
            //rotate about local axis
            if (draw_data[i][1]==1.0)
            {
                y_hat = this->rotate_about_axis(y_hat,x_hat,draw_data[i][2]);
                z_hat = this->rotate_about_axis(z_hat,x_hat,draw_data[i][2]);
            }else if (draw_data[i][1]==2.0)
            {
                x_hat = this->rotate_about_axis(x_hat,y_hat,-draw_data[i][2]);
                z_hat = this->rotate_about_axis(z_hat,y_hat,-draw_data[i][2]);
            }else if (draw_data[i][1]==3.0)
            {
                x_hat = this->rotate_about_axis(x_hat,z_hat,draw_data[i][2]);
                y_hat = this->rotate_about_axis(y_hat,z_hat,-draw_data[i][2]);
            }
            //inverse signum
            for (size_t ii = 0; ii < 3; ii++)
            {
                x_hat[ii] = -x_hat[ii];
                y_hat[i] = -y_hat[i];
                z_hat[ii] = -z_hat[ii];
            }
            
            draw_arrow(center, x_hat, true, "green", size);
            draw_arrow(center, y_hat, true, "yellow", size);
            draw_arrow(center, z_hat, true, "red", size);   
        }else if (draw_data[i][0]==2.0)
        {
            for (size_t ii = 0; ii < 3; ii++)
            {
                z_hat.push_back(draw_data[i+2][ii]-draw_data[i+3][ii]);
            }
            draw_arrow_flat(center, z_hat, true, "green", size);   
        }
        i+=3;
    }

    return true;
}

bool CoreDraw::draw_equation(int id, double size)
{
    //std::string log = "Equation ID " + std::to_string(id) + "  drawn with size " + std::to_string(size) +"\n";
    //PRINT_INFO("%s", log.c_str());
    
    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_equation(id);
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        draw_equation_node({draw_data[i][0],draw_data[i][1],draw_data[i][2]}, draw_data[i][3], "", size);
    }

    // draw lines
    std::vector<std::string> commands;
    
    if (draw_data.size()>1)
    {
        for (size_t i = 1; i < draw_data.size(); i++)
        {
            commands.push_back("draw line location pos " + ccx_iface->to_string_scientific(draw_data[0][0]) + " " + ccx_iface->to_string_scientific(draw_data[0][1]) + " " + ccx_iface->to_string_scientific(draw_data[0][2]) + " location pos " + ccx_iface->to_string_scientific(draw_data[i][0]) + " " + ccx_iface->to_string_scientific(draw_data[i][1]) + " " + ccx_iface->to_string_scientific(draw_data[i][2]) + " color yellow no_flush");
        }
        commands.push_back("graphics flush");
    }

    for (size_t i = 0; i < commands.size(); i++)
    {
        ccx_iface->silent_cmd(commands[i]);
    }
    
    return true;
}

bool CoreDraw::draw_equationgroup(int id, double size)
{
    //std::string log = "Equation ID " + std::to_string(id) + "  drawn with size " + std::to_string(size) +"\n";
    //PRINT_INFO("%s", log.c_str());
    
    ccx_iface->set_block_core_update(true);
    ccx_iface->set_block_gui_update(true);

    std::vector<std::vector<double>> draw_data;
    draw_data = ccx_iface->get_draw_data_for_equationgroup(id);
    std::vector<std::string> commands;
    
    for (size_t i = 0; i < draw_data.size(); i++)
    {
        commands.push_back("draw location pos " + ccx_iface->to_string_scientific(draw_data[i][0]) + " " + ccx_iface->to_string_scientific(draw_data[i][1]) + " " + ccx_iface->to_string_scientific(draw_data[i][2]) + " color yellow no_flush");
    }

    // draw lines
    if (draw_data.size()>1)
    {
        for (size_t i = 0; i < draw_data.size(); i = i + 2)
        {
            commands.push_back("draw line location pos " + ccx_iface->to_string_scientific(draw_data[i][0]) + " " + ccx_iface->to_string_scientific(draw_data[i][1]) + " " + ccx_iface->to_string_scientific(draw_data[i][2]) + " location pos " + ccx_iface->to_string_scientific(draw_data[i+1][0]) + " " + ccx_iface->to_string_scientific(draw_data[i+1][1]) + " " + ccx_iface->to_string_scientific(draw_data[i+1][2]) + " color yellow no_flush");
        }
        commands.push_back("graphics flush");
    }
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        ccx_iface->silent_cmd(commands[i]);
    }

    ccx_iface->set_block_core_update(false);
    ccx_iface->set_block_gui_update(false);
    
    return true;
}

bool CoreDraw::draw_loads(double size)
{
    std::vector<int> tmp_load_ids;
    std::vector<int> tmp_bc_ids;
    
    tmp_load_ids = ccx_iface->get_loadsforces_ids();    
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_force(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadspressures_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_pressure(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadsheatfluxes_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_heatflux(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadsgravity_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_gravity(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadscentrifugal_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_centrifugal(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadstrajectory_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_trajectory(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadsfilm_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_film(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadsradiation_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_radiation(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadssurfacetraction_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_surface_traction(tmp_load_ids[i], size);
    }

    tmp_load_ids = ccx_iface->get_loadsbodyheatflux_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_bodyheatflux(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_bcs(double size)
{
    std::vector<int> tmp_bc_ids;
    
    tmp_bc_ids = ccx_iface->get_bcsdisplacements_ids();    
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        draw_bc_displacement(tmp_bc_ids[i], size);
    }

    tmp_bc_ids = ccx_iface->get_bcstemperatures_ids();    
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        draw_bc_temperature(tmp_bc_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_orientations(double size)
{
    std::vector<int> tmp_ids;
    
    tmp_ids = ccx_iface->get_orientations_ids();    
    for (size_t i = 0; i < tmp_ids.size(); i++)
    {
        draw_orientation(tmp_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_equations(double size)
{
    std::vector<int> tmp_ids;
    
    tmp_ids = ccx_iface->get_equation_ids();    
    for (size_t i = 0; i < tmp_ids.size(); i++)
    {
        draw_equation(tmp_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_equationgroups(double size)
{
    std::vector<int> tmp_ids;
    
    tmp_ids = ccx_iface->get_equationgroup_ids();    
    for (size_t i = 0; i < tmp_ids.size(); i++)
    {
        draw_equationgroup(tmp_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_all(double size)
{
    this->draw_bcs(size);
    this->draw_loads(size);
    this->draw_orientations(size);
    this->draw_equations(size);
    this->draw_equationgroups(size);

    std::string log = "Loads, BCs, Orientations and Equations drawn with size " + ccx_iface->to_string_scientific(size) +"\n";
    PRINT_INFO("%s", log.c_str());

    return true;
}

bool CoreDraw::draw_load_forces(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadsforces_ids();    
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_force(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_pressures(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadspressures_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_pressure(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_heatfluxes(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadsheatfluxes_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_heatflux(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_gravities(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadsgravity_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_gravity(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_centrifugals(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadscentrifugal_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_centrifugal(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_trajectories(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadstrajectory_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_trajectory(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_films(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadsfilm_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_film(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_radiations(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadsradiation_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_radiation(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_surface_tractions(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadssurfacetraction_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_surface_traction(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_load_bodyheatfluxes(double size)
{
    std::vector<int> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadsbodyheatflux_ids();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_bodyheatflux(tmp_load_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_bc_displacements(double size)
{
    std::vector<int> tmp_bc_ids;
    
    tmp_bc_ids = ccx_iface->get_bcsdisplacements_ids();    
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        draw_bc_displacement(tmp_bc_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_bc_temperatures(double size)
{
    std::vector<int> tmp_bc_ids;
    
    tmp_bc_ids = ccx_iface->get_bcstemperatures_ids();    
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        draw_bc_temperature(tmp_bc_ids[i], size);
    }

    return true;
}

bool CoreDraw::draw_topology(int topology_id,int iteration_id,double threshold)
{
  if (!is_initialized || !ccx_iface ||
      iteration_id < -1 ||
      !std::isfinite(threshold) ||
      threshold < 0.0 || threshold > 1.0)
  {
    PRINT_ERROR("Invalid topology drawing request.\n");
    return false;
  }

  const auto rows = ccx_iface->get_topology_iteration_element_data(topology_id, iteration_id);

  if (rows.empty())
  {
    return false;
  }

  std::map<std::string, std::vector<int>> keep;
  std::map<std::string, std::vector<int>> remove;
  size_t kept_count = 0;
  size_t removed_count = 0;

  // Resolve every element before changing the display.
  for (const auto& row : rows)
  {
    if (row.size() != 4 ||
        !std::isfinite(row[0]) ||
        row[0] < 1.0 ||
        row[0] > static_cast<double>(
        (std::numeric_limits<int>::max)()) ||
        std::floor(row[0]) != row[0] ||
        !std::isfinite(row[1]) ||
        row[1] < 0.0 || row[1] > 1.0 ||
        (row[3] != 0.0 && row[3] != 1.0))
    {
      PRINT_ERROR("Invalid topology preview data.\n");
      return false;
    }

    const int global_id = static_cast<int>(row[0]);
    const std::string type = CubitInterface::get_element_type(global_id);

    std::string entity = type;

    if (entity != "hex" && entity != "tet" && entity != "wedge")
    {
      entity = ccx_iface->get_cubit_element_type_entity(type);
    }

    const int local_id = CubitInterface::get_element_type_id(global_id);

    if ((entity != "hex" && entity != "tet" && entity != "wedge") ||
        local_id <= 0)
    {
      PRINT_ERROR("Unable to resolve topology element %d.\n", global_id);
      return false;
    }

    const bool preserved = row[3] == 1.0;
    const bool removal_candidate = !preserved && row[1] < threshold;

    if (removal_candidate)
    {
      remove[entity].push_back(local_id);
      ++removed_count;
    }
    else
    {
      keep[entity].push_back(local_id);
      ++kept_count;
    }
  }

  bool first_draw = true;

  const auto draw_groups = [&](const std::map<std::string, std::vector<int>>& groups, bool edges_only, const std::string& color)
  {
    constexpr size_t batch_size = 200;

    for (const auto& group : groups)
    {
      std::string entity = group.first;
      std::vector<int> ids = group.second;

      if (edges_only)
      {
        std::set<int> edge_ids;

        for (int element_id : group.second)
        {
          const auto edges = CubitInterface::get_sub_elements(group.first, element_id, 1);
          edge_ids.insert(edges.begin(), edges.end());
        }

        if (edge_ids.empty())
        {
          continue;
        }

        entity = "edge";
        ids.assign(edge_ids.begin(), edge_ids.end());
      }

      for (size_t start = 0; start < ids.size(); start += batch_size)
      {
        std::ostringstream command;
        command << "draw " << entity;

        const size_t end = std::min(ids.size(), start + batch_size);

        for (size_t index = start; index < end; ++index)
        {
          command << " " << ids[index];
        }

        command << " color " << color;

        if (!first_draw)
        {
          command << " add";
        }

        if (!ccx_iface->silent_cmd(command.str()))
        {
          PRINT_ERROR("Failed to draw topology preview.\n");
          return false;
        }

        first_draw = false;
      }
    }

    return true;
  };

  if (!draw_groups(keep, false, "green") ||
      !draw_groups(remove, true, "red"))
  {
    return false;
  }

  if (first_draw)
  {
    if (!ccx_iface->silent_cmd("graphics clear"))
    {
      PRINT_ERROR("Unable to clear the topology preview.\n");
      return false;
    }
  }

  if (!ccx_iface->silent_cmd("graphics flush"))
  {
    return false;
  }

  if (!ccx_iface->is_silent_cmd())
  {
  PRINT_INFO(
      "Topology %d preview: threshold %.6g; "
      "%zu kept elements, %zu removal candidates shown as red edges.\n",
      topology_id, threshold, kept_count, removed_count);
  }

  return true;
}

bool CoreDraw::animate_topology(int topology_id, double threshold,int fps,std::string save_filepath, bool overwrite)
{
  if (!is_initialized || !ccx_iface)
  {
    PRINT_ERROR("Drawing core is not initialized.\n");
    return false;
  }

  if (!std::isfinite(threshold) ||
      threshold < 0.0 || threshold > 1.0 ||
      fps < 1 || fps > 100)
  {
    PRINT_ERROR("Animation requires threshold 0..1 and fps 1..100.\n");
    return false;
  }

  const std::string run_state = ccx_iface->get_topology_run_state(topology_id);

  if (run_state.empty())
  {
    return false;
  }

  if (run_state == "running")
  {
    PRINT_ERROR("Cannot animate a running topology case.\n");
    return false;
  }

  const auto rows = ccx_iface->get_topology_iteration_data(topology_id);

  std::set<int> iterations;

  for (const auto& row : rows)
  {
    // Completed job with an evaluated objective and volume.
    if (row.size() < 6 ||
        row[2] != "2" ||
        row[3].empty() ||
        row[4].empty())
    {
      continue;
    }

    int iteration_id = -1;
    std::istringstream stream(row[0]);

    if (!(stream >> iteration_id))
    {
      PRINT_ERROR("Invalid topology iteration ID.\n");
      return false;
    }

    stream >> std::ws;

    if (!stream.eof() || iteration_id < 1)
    {
      PRINT_ERROR("Invalid topology iteration ID.\n");
      return false;
    }

    iterations.insert(iteration_id);
  }

  if (iterations.empty())
  {
    PRINT_ERROR("Topology %d has no completed iterations to animate.\n", topology_id);
    return false;
  }

  if (save_filepath.empty())
  {
    save_filepath = "topo_" + std::to_string(topology_id) + "_animation.gif";
  }

  const QString output = QDir::current().absoluteFilePath(QString::fromStdString(save_filepath));

  const QFileInfo output_info(output);

  if (output_info.suffix().compare("gif", Qt::CaseInsensitive) != 0)
  {
    PRINT_ERROR("Animation save path must end with .gif.\n");
    return false;
  }

  if (!output_info.dir().exists())
  {
    PRINT_ERROR("Animation output directory does not exist.\n");
    return false;
  }

  if (output_info.exists() && !overwrite)
    {
    PRINT_ERROR(
        "Animation output file already exists; use overwrite.\n");
    return false;
    }

  // One temporary directory for this export, not for solver jobs.
  QTemporaryDir temporary;

  if (!temporary.isValid())
  {
    PRINT_ERROR("Unable to create animation temporary directory.\n");
    return false;
  }

  const QDir frame_directory(temporary.path());

  // Frame paths are inserted into quoted Cubit commands.
  if (temporary.path().contains('\'') ||
      temporary.path().contains('\n') ||
      temporary.path().contains('\r'))
  {
    PRINT_ERROR("Unsupported animation temporary path.\n");
    return false;
  }

  QSize frame_size;
  int frame_index = 0;
  QStringList frame_paths;

  for (const int iteration_id : iterations)
  {
    PRINT_INFO(
        "Topology %d animation: capturing iteration %d.\n",
        topology_id, iteration_id);

    const std::string preview_command = QString(
      "ccx result project topology %1 iteration %2 threshold %3 preview")
      .arg(topology_id).arg(iteration_id)
      .arg(QString::number(threshold, 'f', 6)).toStdString();
    if (!ccx_iface->silent_cmd(preview_command))
    {
      return false;
    }

    const QString frame_path = frame_directory.filePath(
        QString("frame_%1.png").arg(
            frame_index, 6, 10, QChar('0')));

    const std::string command = "hardcopy '" + QDir::fromNativeSeparators(frame_path).toStdString() + "' png";

    if (!ccx_iface->silent_cmd(command))
    {
      PRINT_ERROR("Unable to capture topology animation frame.\n");
      return false;
    }

    const QImage frame(frame_path);

    if (frame.isNull())
    {
      PRINT_ERROR("Topology frame was not captured. An active graphics window is required.\n");
      return false;
    }

    if (frame_index == 0)
    {
      frame_size = frame.size();
    }
    else if (frame.size() != frame_size)
    {
      PRINT_ERROR("Animation frame dimensions changed.\n");
      return false;
    }

    frame_paths.push_back(frame_path);
    ++frame_index;
  }

  const QString temporary_gif =
      frame_directory.filePath("animation.gif");

  PRINT_INFO("Topology %d animation: encoding %d frames.\n", topology_id, frame_index);

  if (!GifAnimation::write(frame_paths, temporary_gif, fps))
  {
    return false;
  }

  if (QFileInfo(temporary_gif).size() == 0)
  {
    PRINT_ERROR("GIF encoder produced an empty file.\n");
    return false;
  }

  // Publish only the successfully encoded GIF.
  if (overwrite)
  {
    QFile source(temporary_gif);
    QSaveFile destination(output);

    // Preserve the previous GIF if writing the replacement fails.
    destination.setDirectWriteFallback(false);

    if (!source.open(QIODevice::ReadOnly) ||
        !destination.open(QIODevice::WriteOnly))
    {
        PRINT_ERROR("Unable to open animation files for replacement.\n");
        return false;
    }

    while (!source.atEnd())
    {
        const QByteArray chunk = source.read(1024 * 1024);

        if (source.error() != QFileDevice::NoError ||
            chunk.isEmpty())
        {
        PRINT_ERROR("Unable to read encoded animation.\n");
        return false;
        }

        if (destination.write(chunk) != chunk.size())
        {
        PRINT_ERROR("Unable to write replacement animation.\n");
        return false;
        }
    }

    if (!destination.commit())
    {
        PRINT_ERROR("Unable to replace animation output.\n");
        return false;
    }
  }
  else
  {
        if (!QFile::copy(temporary_gif, output))
        {
            PRINT_ERROR("Unable to save topology animation.\n");
            return false;
        }
    }

  PRINT_INFO("Topology %d animation saved to %s.\n",topology_id, output.toUtf8().constData());

  return true;
}

std::string CoreDraw::get_color(int color_id)
{
    std::vector<std::string> colors;
    colors.push_back("red");    
    colors.push_back("orange");
    colors.push_back("yellow");
    colors.push_back("salmon");
    colors.push_back("coral");
    colors.push_back("pink");
    colors.push_back("purple");
    colors.push_back("paleturquoise");
    colors.push_back("lightsalmon");
    colors.push_back("springgreen");
    colors.push_back("slateblue");
    colors.push_back("sienna");
    colors.push_back("seagreen");
    colors.push_back("deepskyblue");
    colors.push_back("lightblue");
    colors.push_back("lightgreen");
    colors.push_back("khaki");
    colors.push_back("tomato");
    colors.push_back("lightskyblue");
    colors.push_back("turquoise");
    colors.push_back("greenyellow");
    colors.push_back("powderblue");
    colors.push_back("mediumturquoise");
    colors.push_back("skyblue");
    colors.push_back("lightcyan");
    colors.push_back("dodgerblue");
    colors.push_back("aquamarine");
    colors.push_back("lightgoldenrodyellow");
    colors.push_back("darkgreen");
    colors.push_back("lightcoral");
    colors.push_back("green");
    colors.push_back("magenta");
    colors.push_back("cyan");
    colors.push_back("blue");
    colors.push_back("white");
    colors.push_back("brown");
    colors.push_back("gold");
    
    if ((color_id>0)&&(color_id<38))
    {
        return colors[color_id];
    }
    return colors[0];
}

namespace
{
  constexpr const char* ccx_graphics_set_name = "ccx_draw";

  GCubitModelSet* ccx_graphics_model_set()
  {
    auto* factory = GWindowFactory::get_window_factory();
    if (factory == nullptr)
    {
      return nullptr;
    }

    for (const auto& view : factory->views())
    {
      if (view == nullptr)
      {
        continue;
      }

      auto* model_set = dynamic_cast<GCubitModelSet*>(view->get_model_set());
      if (model_set != nullptr)
      {
        return model_set;
      }
    }

    return nullptr;
  }

  GGenericModel* ccx_graphics_model()
  {
    auto* model_set = ccx_graphics_model_set();
    if (model_set == nullptr)
    {
      PRINT_ERROR("No Cubit graphics model set is available.\n");
      return nullptr;
    }

    auto* drawing_set = model_set->get_drawing_set(CubitString(ccx_graphics_set_name));
    if (drawing_set == nullptr)
    {
      // Retain the set across display; graphics_clear explicitly clears CCX data.
      drawing_set = model_set->create_drawing_set(CubitString(ccx_graphics_set_name), false);
    }

    if (drawing_set == nullptr || drawing_set->mGenericModel == nullptr)
    {
      PRINT_ERROR("Unable to obtain the CCX graphics drawing set.\n");
      return nullptr;
    }

    return drawing_set->mGenericModel;
  }

  bool ccx_graphics_color(const std::string& name, CubitColor& color)
  {
    int color_id = -1;
    if (!CubitColorTable::string_to_color_id(name.c_str(), color_id))
    {
      PRINT_ERROR("Unknown Cubit color: %s\n", name.c_str());
      return false;
    }

    return CubitColorTable::color_id_to_rgb(color_id, color);
  }

  bool ccx_graphics_valid_point(const std::vector<double>& coord)
  {
    return coord.size() == 3 &&
           std::isfinite(coord[0]) &&
           std::isfinite(coord[1]) &&
           std::isfinite(coord[2]);
  }
}

bool CoreDraw::graphics_point(const std::vector<double>& coord, const std::string& color)
{
  CubitColor draw_color;
  if (!ccx_graphics_valid_point(coord) || !ccx_graphics_color(color, draw_color))
  {
    return false;
  }

  auto* model = ccx_graphics_model();
  if (model == nullptr)
  {
    return false;
  }

  const CubitVector point(coord[0], coord[1], coord[2]);
  model->add_points(1, &point, draw_color);
  return true;
}

bool CoreDraw::graphics_line(const std::vector<double>& start, const std::vector<double>& end, const std::string& color)
{
  CubitColor draw_color;
  if (!ccx_graphics_valid_point(start) || !ccx_graphics_valid_point(end) || !ccx_graphics_color(color, draw_color))
  {
    return false;
  }

  auto* model = ccx_graphics_model();
  if (model == nullptr)
  {
    return false;
  }

  const CubitVector start_point(start[0], start[1], start[2]);
  const CubitVector end_point(end[0], end[1], end[2]);
  model->add_line(start_point, end_point, draw_color);
  return true;
}

bool CoreDraw::graphics_polygon(const std::vector<std::vector<double>>& coords, const std::string& color)
{
  CubitColor draw_color;
  if (coords.size() < 3 || coords.size() > static_cast<size_t>(std::numeric_limits<int>::max()) || !ccx_graphics_color(color, draw_color))
  {
    return false;
  }

  std::vector<CubitVector> points;
  points.reserve(coords.size());
  for (const auto& coord : coords)
  {
    if (!ccx_graphics_valid_point(coord))
    {
      return false;
    }

    points.emplace_back(coord[0], coord[1], coord[2]);
  }

  auto* model = ccx_graphics_model();
  if (model == nullptr)
  {
    return false;
  }

  model->add_polygon(static_cast<int>(points.size()), points.data(), draw_color, SVUtil::SHADE);
  return true;
}

bool CoreDraw::graphics_label(const std::vector<double>& coord, const std::string& text, const std::string& color)
{
  CubitColor draw_color;
  if (!ccx_graphics_valid_point(coord) || !ccx_graphics_color(color, draw_color))
  {
    return false;
  }

  auto* model = ccx_graphics_model();
  if (model == nullptr)
  {
    return false;
  }

  const CubitVector point(coord[0], coord[1], coord[2]);
  model->add_label(text.c_str(), point, draw_color);
  return true;
}

bool CoreDraw::graphics_flush()
{
  auto* model_set = ccx_graphics_model_set();
  if (model_set == nullptr)
  {
    return false;
  }

  model_set->update_model_parts();
  model_set->render();
  return true;
}

bool CoreDraw::graphics_clear()
{
  auto* model_set = ccx_graphics_model_set();
  if (model_set == nullptr)
  {
    return false;
  }

  auto* drawing_set = model_set->get_drawing_set(CubitString(ccx_graphics_set_name));
  if (drawing_set != nullptr)
  {
    drawing_set->clear();
  }

  model_set->update_model_parts();
  model_set->render();
  return true;
}
