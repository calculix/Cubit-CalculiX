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

    //log = "vec_a = " + std::to_string(vec_a[0]) + " " + std::to_string(vec_a[1]) + " " + std::to_string(vec_a[2]) + " " +"\n";
    //log.append("vec_b = " + std::to_string(vec_b[0]) + " " + std::to_string(vec_b[1]) + " " + std::to_string(vec_b[2]) + " " +"\n");
    //log.append("vec_k = " + std::to_string(vec_k[0]) + " " + std::to_string(vec_k[1]) + " " + std::to_string(vec_k[2]) + " " +"\n");
    
    //sinus cosinus
    sinus = magnitude(cross_product(vec_a, vec_b));
    cosinus = inner_product(vec_a, vec_b);

    //log.append("sinus = " + std::to_string(sinus) +"\n");
    //log.append("cosinus = " + std::to_string(cosinus) +"\n");

    //cross product matrix
    std::vector<std::vector<double>> mat_k = cross_product_matrix(vec_k);
    std::vector<std::vector<double>> mat_k2 = mult_matrix(mat_k,mat_k);
    /*
    log.append("mat_k = \n");
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {
            log.append(std::to_string(mat_k[i][ii]) + " ");
        }
        log.append("\n");
    }
    log.append("mat_k2 = \n");
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {
            log.append(std::to_string(mat_k2[i][ii]) + " ");
        }
        log.append("\n");
    }
    */
    // rotation matrix

    mat_r = add_matrix(mat_r, mult_matrix_scalar(mat_k,sinus));
    mat_r = add_matrix(mat_r, mult_matrix_scalar(mat_k2, 1 - cosinus));

    tmp_coord = mult_matrix_vector(mat_r,coord);

    /*
    log.append("mat_r = \n");
    for (size_t i = 0; i < 3; i++)
    {
        for (size_t ii = 0; ii < 3; ii++)
        {
            log.append(std::to_string(mat_r[i][ii]) + " ");
        }
        log.append("\n");
    }

    log.append("coord = " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " " +"\n");
    log.append("tmp_coord = " + std::to_string(tmp_coord[0]) + " " + std::to_string(tmp_coord[1]) + " " + std::to_string(tmp_coord[2]) + " " +"\n");
    log.append("*********************************************\n");
    PRINT_INFO("%s", log.c_str());
    */

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
    }else if (dof==11)
    {
        /* code */
    }
    // draw line
    commands.push_back("draw line location pos " + std::to_string(draw_coord[0]) + " " + std::to_string(draw_coord[1]) + " " + std::to_string(draw_coord[2]) + " location pos " + std::to_string(coord[0]) + " " + std::to_string(coord[1]) + " " + std::to_string(coord[2]) + " color " + color + " no_flush");
    commands.push_back("graphics flush"); 
    
    
    for (size_t i = 0; i < commands.size(); i++)
    {
        CubitInterface::silent_cmd(commands[i].c_str());
    }

    return true;
}

bool draw_load_force(int id)
{
    return true;
}

bool draw_load_pressure(int id)
{
    return true;
}

bool draw_load_heatflux(int id)
{
    return true;
}

bool draw_load_gravity(int id)
{
    return true;
}

bool draw_load_centrifugal(int id)
{
    return true;
}

bool draw_bc_displacement(int id)
{
    return true;
}

bool draw_bc_temperature(int id)
{
    return true;
}

bool CoreDraw::draw_loads()
{
    std::vector<std::vector<std::string>> tmp_load_ids;
    
    tmp_load_ids = ccx_iface->get_loadsforces_tree_data();    
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_force(std::stoi(tmp_load_ids[i][1]));
    }

    tmp_load_ids = ccx_iface->get_loadspressures_tree_data();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_pressure(std::stoi(tmp_load_ids[i][1]));
    }

    tmp_load_ids = ccx_iface->get_loadsheatfluxes_tree_data();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_heatflux(std::stoi(tmp_load_ids[i][1]));
    }

    tmp_load_ids = ccx_iface->get_loadsgravity_tree_data();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_gravity(std::stoi(tmp_load_ids[i][1]));
    }

    tmp_load_ids = ccx_iface->get_loadscentrifugal_tree_data();
    for (size_t i = 0; i < tmp_load_ids.size(); i++)
    {
        draw_load_centrifugal(std::stoi(tmp_load_ids[i][1]));
    }
    return true;
}

bool CoreDraw::draw_bcs()
{
    std::vector<std::vector<std::string>> tmp_bc_ids;
    
    tmp_bc_ids = ccx_iface->get_bcsdisplacements_tree_data();    
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        draw_bc_displacement(std::stoi(tmp_bc_ids[i][1]));
    }

    tmp_bc_ids = ccx_iface->get_bcstemperatures_tree_data();    
    for (size_t i = 0; i < tmp_bc_ids.size(); i++)
    {
        draw_bc_temperature(std::stoi(tmp_bc_ids[i][1]));
    }

    return true;
}

bool CoreDraw::draw_all()
{
    std::vector<double> start_point = {0,0,0};
    std::vector<double> direction = {1,1,1};
    std::vector<double> coord = {0,0,0};
    std::string color = "red";
    double size = 1;

    //this->draw_arrow(start_point, direction, false, color, size);

    start_point = {0,0,0};
    direction = {0.1,1,0.3};
    color = "blue";
    size = 1;
    //this->draw_arrow(start_point, direction, true, color, size);

    start_point = {0,0,0};
    direction = {-1,-1,0};
    color = "green";
    size = 1;
    //this->draw_arrow(start_point, direction, false, color, size);

    coord = {1,0,0};
    color = "blue";
    size = 1;
    this->draw_dof(coord, 1, color, size);
    coord = {1,0,0};
    color = "yellow";
    size = 2;
    this->draw_dof(coord, 2, color, size);
    coord = {1,0,0};
    color = "red";
    size = 3;
    //this->draw_dof(coord, 3, color, size);
    coord = {1,0,0};
    color = "blue";
    size = 1;
    this->draw_dof(coord, 4, color, size);
    coord = {1,0,0};
    color = "yellow";
    size = 2;
    this->draw_dof(coord, 5, color, size);
    coord = {1,0,0};
    color = "red";
    size = 3;
    this->draw_dof(coord, 6, color, size);


    return true;
}