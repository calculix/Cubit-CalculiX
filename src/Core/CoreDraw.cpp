#include "CoreDraw.hpp"
#include "CubitInterface.hpp"
#include <cmath>
#include "CalculiXCoreInterface.hpp"
#include "CubitMessage.hpp"

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
        //commands[i].append("\n");
        //PRINT_INFO("%s", commands[i].c_str());
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
                commands.push_back("draw polygon location pos " + std::to_string(draw_coord[0]) + " " + std::to_string(draw_coord[1]) + " " + std::to_string(draw_coord[2]) + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]) + " location pos " + std::to_string(polygon_coords[0][0]) + " " + std::to_string(polygon_coords[0][1]) + " " + std::to_string(polygon_coords[0][2]) + " color " + color + " no_flush");
            }else{
                commands.push_back("draw polygon location pos " + std::to_string(draw_coord[0]) + " " + std::to_string(draw_coord[1]) + " " + std::to_string(draw_coord[2]) + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]) + " location pos " + std::to_string(polygon_coords[i+1][0]) + " " + std::to_string(polygon_coords[i+1][1]) + " " + std::to_string(polygon_coords[i+1][2]) + " color " + color + " no_flush");
            }
        }
        cmd = "draw polygon";
        for (size_t i = 0; i < npolygon; i++)
        {
            cmd = cmd + " location pos " + std::to_string(polygon_coords[i][0]) + " " + std::to_string(polygon_coords[i][1]) + " " + std::to_string(polygon_coords[i][2]);
        }

        cmd = cmd + " color " + color + " no_flush";
        commands.push_back(cmd);
        // draw line
        commands.push_back("draw line location pos " + std::to_string(draw_coord[0]) + " " + std::to_string(draw_coord[1]) + " " + std::to_string(draw_coord[2]) + " location pos " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " color " + color + " no_flush");
    }else if (dof==11)
    {
        commands.push_back("draw location " +  std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " color " + color + " no_flush");
    }
    commands.push_back("graphics flush");
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        CubitInterface::silent_cmd(commands[i].c_str());
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
    std::vector<int> node_ids;
    node_ids = ccx_iface->loadstrajectory_get_node_ids(id);
    std::vector<std::vector<double>> hit_coordinates;
    hit_coordinates = ccx_iface->loadstrajectory_get_hit_coordinates(id);
    std::vector<std::vector<std::vector<int>>> face_ids;
    face_ids = ccx_iface->loadstrajectory_get_draw_face_ids(id);
    std::vector<std::vector<double>> times;
    times = ccx_iface->loadstrajectory_get_times(id);
    bool switch_color = true;

    for (size_t i = 0; i < hit_coordinates.size(); i++)
    {
        if (hit_coordinates[i].size()>0)
        {
            std::array<double,3> coord = CubitInterface::get_nodal_coordinates(node_ids[i]);
            if (switch_color)
            {
                ccx_iface->silent_cmd("draw location " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " color red");
                ccx_iface->silent_cmd("draw line location " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " location " + std::to_string(hit_coordinates[i][0]) + " " + std::to_string(hit_coordinates[i][1]) + " " + std::to_string(hit_coordinates[i][2]) +  " color red");
                //switch_color = false;
            }else{
                ccx_iface->silent_cmd("draw location " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " color blue");
                ccx_iface->silent_cmd("draw line location " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " location " + std::to_string(hit_coordinates[i][0]) + " " + std::to_string(hit_coordinates[i][1]) + " " + std::to_string(hit_coordinates[i][2]) +  " color blue");
                //switch_color = true;
            }
        }
    }
    
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
                cmd.append("color " + get_color(ii) + " add");
                
                ccx_iface->silent_cmd(cmd);
                //ccx_iface->cmd(cmd);
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
        std::string label = "locate location " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " ";
        label.append("\" t_begin " + ccx_iface->to_string_scientific(times[0][0]) + "\"");
        ccx_iface->silent_cmd(label);

        coord = CubitInterface::get_nodal_coordinates(node_ids[node_ids.size()-1]);
        label = "locate location " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " ";
        label.append("\" t_end " + ccx_iface->to_string_scientific(times[times.size()-1][1]) + "\"");
        ccx_iface->silent_cmd(label);
    }

    return true;
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

bool CoreDraw::draw_all(double size)
{
    this->draw_bcs(size);
    this->draw_loads(size);
    this->draw_orientations(size);

    std::string log = "Loads and BCs drawn with size " + std::to_string(size) +"\n";
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