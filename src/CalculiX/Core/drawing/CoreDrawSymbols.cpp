#include "CoreDrawSymbols.hpp"
#include <cmath>

std::vector<double> CoreDrawSymbols::rotate(std::vector<double> coord, std::vector<double> vec_a, std::vector<double> vec_b)
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

std::vector<double> CoreDrawSymbols::rotate_about_axis(std::vector<double> coord, std::vector<double> axis, double angle)
{
    // https://de.wikipedia.org/wiki/Drehmatrix

    std::vector<double> n_coord = unit_vector(coord);
    std::vector<double> n_axis = unit_vector(axis);
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

std::vector<double> CoreDrawSymbols::unit_vector(std::vector<double> input)
{
  if (input.size() != 3)
  {
    return {0, 0, 0};
  }
  const double length = magnitude(input);
  if (!std::isfinite(length) || length <= 1e-15)
  {
    return {0, 0, 0};
  }
  return {input[0] / length, input[1] / length, input[2] / length};
}

double CoreDrawSymbols::magnitude(std::vector<double> input)
{
    double magnitude = sqrt(pow(input[0],2) + pow(input[1],2) + pow(input[2],2));

    return magnitude;
}

std::vector<double> CoreDrawSymbols::cross_product(std::vector<double> vec_a, std::vector<double> vec_b)
{
    std::vector<double> output(3);

    output[0] = vec_a[1]*vec_b[2]-vec_a[2]*vec_b[1];
    output[1] = vec_a[2]*vec_b[0]-vec_a[0]*vec_b[2];
    output[2] = vec_a[0]*vec_b[1]-vec_a[1]*vec_b[0];

    return output;
}

std::vector<std::vector<double>> CoreDrawSymbols::cross_product_matrix(std::vector<double> vec)
{
    std::vector<std::vector<double>> output;

    output.push_back({0,-vec[2],vec[1]});
    output.push_back({vec[2],0,-vec[0]});
    output.push_back({-vec[1],vec[0],0});

    return output;
}

double CoreDrawSymbols::inner_product(std::vector<double> vec_a, std::vector<double> vec_b)
{
    double output = 0;

    for (size_t i = 0; i < 3; i++)
    {
        output += vec_a[i]*vec_b[i];
    }

    return output;
}

std::vector<double> CoreDrawSymbols::mult_vector_scalar(std::vector<double> vec, double scalar)
{
    std::vector<double> output(3);

    for (size_t i = 0; i < 3; i++)
    {
        output[i] = vec[i]*scalar;
    }

    return output;
}

std::vector<std::vector<double>> CoreDrawSymbols::mult_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b)
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

std::vector<std::vector<double>> CoreDrawSymbols::mult_matrix_scalar(std::vector<std::vector<double>> mat_a, double scalar)
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

std::vector<std::vector<double>> CoreDrawSymbols::add_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b)
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

std::vector<double> CoreDrawSymbols::mult_matrix_vector(std::vector<std::vector<double>> mat, std::vector<double> vec)
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

bool CoreDrawSymbols::arrow(DrawGeometry& geometry, std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size)
{
  if (start_point.size() != 3 || !std::isfinite(size) || size <= 0)
  {
    return false;
  }
  if (direction.size() != 3)
  {
    return false;
  }

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
        if (!geometry.polygon({arrow_start_coord, polygon_coords[i], polygon_coords[next]}, color))
        {
            return false;
        }
    }

    // Draw arrow-head base and shaft.
    if (!geometry.polygon(polygon_coords, color))
    {
        return false;
    }
    if (!geometry.line(arrow_end_coord, arrow_start_coord, color))
    {
        return false;
    }


    return true;
}

bool CoreDrawSymbols::flatArrow(DrawGeometry& geometry, std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size)
{
  if (start_point.size() != 3 || !std::isfinite(size) || size <= 0)
  {
    return false;
  }
  if (direction.size() != 3)
  {
    return false;
  }


    //skip if zero vector
    if (direction[0]==0 && direction[1]==0 && direction[2]==0)
    {
        return true;
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

    for (std::size_t index = 0; index < polygon_coords.size(); ++index)
    {
      const auto next = (index + 1) % polygon_coords.size();
      if (!geometry.polygon({arrow_start_coord, polygon_coords[index], polygon_coords[next]}, color))
      {
        return false;
      }
    }
    return geometry.polygon(polygon_coords, color) && geometry.line(arrow_end_coord, arrow_start_coord, color);
}

bool CoreDrawSymbols::displacement(DrawGeometry& geometry, std::vector<double> coord, int dof, std::string color, double size)
{
  if (coord.size() != 3 || !std::isfinite(size) || size <= 0)
  {
    return false;
  }


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
    }

    if (dof == 11)
    {
      return geometry.point(coord, color);
    }
    if (polygon_coords.empty())
    {
      return false;
    }
    for (std::size_t index = 0; index < polygon_coords.size(); ++index)
    {
      const auto next = (index + 1) % polygon_coords.size();
      if (!geometry.polygon({draw_coord, polygon_coords[index], polygon_coords[next]}, color))
      {
        return false;
      }
    }
    return geometry.polygon(polygon_coords, color) && geometry.line(draw_coord, coord, color);
}

bool CoreDrawSymbols::equationNode(DrawGeometry& geometry, std::vector<double> coord, int dof, std::string color, double size)
{
  if (coord.size() != 3 || !std::isfinite(size) || size <= 0)
  {
    return false;
  }


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
    }

    if (dof == 11)
    {
      return geometry.point(coord, color);
    }
    if (polygon_coords.empty())
    {
      return false;
    }
    for (std::size_t index = 0; index < polygon_coords.size(); ++index)
    {
      const auto next = (index + 1) % polygon_coords.size();
      if (!geometry.polygon({draw_coord, polygon_coords[index], polygon_coords[next]}, color))
      {
        return false;
      }
    }
    return true;
}
