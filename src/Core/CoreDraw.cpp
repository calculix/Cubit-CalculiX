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

std::vector<double> CoreDraw::rotate(std::vector<double> coord, std::vector<double> vec_a, std::vector<double> vec_b)
{
    // https://en.wikipedia.org/wiki/Rodrigues%27_rotation_formula
    std::vector<double> tmp_coord = {0,0,0};
    std::vector<std::vector<double>> r_mat;
    r_mat.push_back({1,0,0});
    r_mat.push_back({0,1,0});
    r_mat.push_back({0,0,1});

    double sinus = 0;
    double cosinus = 0;

    //first compute the unit vectors
    vec_a = unit_vector(vec_a);
    vec_b = unit_vector(vec_b);

    //sinus cosinus
    sinus = magnitude(cross_product(vec_b, vec_a));
    cosinus = inner_product(vec_a, vec_b);

    //cross product matrix
    std::vector<std::vector<double>> k_mat = cross_product_matrix(vec_b,vec_a);
    std::vector<std::vector<double>> k2_mat = mult_matrix(k_mat,k_mat);

    // rotation matrix
    r_mat = add_matrix(r_mat, mult_matrix_scalar(k_mat,sinus));
    r_mat = add_matrix(r_mat, mult_matrix_scalar(k2_mat, 1 - cosinus));

    tmp_coord = mult_matrix_vector(r_mat,coord);

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

std::vector<std::vector<double>> CoreDraw::cross_product_matrix(std::vector<double> vec_a, std::vector<double> vec_b)
{
    std::vector<std::vector<double>> output;

    output.push_back({0,-vec_a[2],vec_a[1]});
    output.push_back({vec_a[2],0,-vec_a[0]});
    output.push_back({-vec_a[1],vec_a[0],0});

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
bool CoreDraw::draw_arrow(std::vector<double> start_point, std::vector<double> direction, bool inverse_head, std::string color, double size)
{
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

    for (size_t i = 0; i < npolygon; i++)
    {
        std::vector<double> coord(3);
        coord[0]= radius*cos(2*pi/npolygon*i);
        coord[1]= radius*sin(2*pi/npolygon*i);
        coord[2]= 0.3;
        coord = rotate(coord,{0.,0.,1.},arrow_direction);
        polygon_coords.push_back(coord);
    }

    // draw arrow head
    for (size_t i = 0; i < npolygon; i++)
    {
        if (i==npolygon-1)
        {
            commands.push_back("draw polygon location pos " + std::to_string(arrow_start_coord[0]) + " " + std::to_string(arrow_start_coord[1]) + " " + std::to_string(arrow_start_coord[2]) + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]) + " location pos " + std::to_string(polygon_coords[0][0]) + " " + std::to_string(polygon_coords[0][1]) + " " + std::to_string(polygon_coords[0][2]) + " color " + color + " no_flush");
        }else{
            commands.push_back("draw polygon location pos " + std::to_string(arrow_start_coord[0]) + " " + std::to_string(arrow_start_coord[1]) + " " + std::to_string(arrow_start_coord[2]) + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]) + " location pos " + std::to_string(polygon_coords[i+1][0]) + " " + std::to_string(polygon_coords[i+1][1]) + " " + std::to_string(polygon_coords[i+1][2]) + " color " + color + " no_flush");
        }
    }
    cmd = "draw polygon";
    for (size_t i = 0; i < npolygon; i++)
    {
        cmd = cmd + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]);
    }

    cmd = cmd + " color " + color + " no_flush";
    commands.push_back(cmd);

    // draw arrow line
    commands.push_back("draw line location pos " + std::to_string(arrow_end_coord[0]) + " " + std::to_string(arrow_end_coord[1]) + " " + std::to_string(arrow_end_coord[2]) + " location pos " + std::to_string(arrow_start_coord[0]) + " " + std::to_string(arrow_start_coord[1]) + " " + std::to_string(arrow_start_coord[2]) + " color " + color + " no_flush");
    commands.push_back("graphics flush"); 
    
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        CubitInterface::silent_cmd(commands[i].c_str());
    }

    return true;
}

bool CoreDraw::draw_all()
{
    std::vector<double> start_point = {0,0,0};
    std::vector<double> direction = {1,1,1};
    std::string color = "red";
    double size = 1;

    this->draw_arrow(start_point, direction, false, color, size);


    start_point = {1,1,1};
    direction = {0.1,1,0.3};
    color = "blue";
    size = 2;
    //this->draw_arrow(start_point, direction, true, color, size);
    return true;
}