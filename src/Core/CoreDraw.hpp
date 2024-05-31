#ifndef COREDRAW_HPP
#define COREDRAW_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

// why it's not in the gui section, because we control it over the commands
// class for drawing of loads and bcs

class CoreDraw
{

public:
  CoreDraw();
  ~CoreDraw();

  bool is_initialized = false;

  bool init(); // initialize
  std::vector<double> rotate(std::vector<double> coord, std::vector<double> vec_a, std::vector<double> vec_b); // rotates coordinate from vector a to b
  std::vector<double> rotate_about_axis(std::vector<double> coord, std::vector<double> axis, double angle); // rotates coordinate about an axis with given angle
  std::vector<double> unit_vector(std::vector<double> input);
  double magnitude(std::vector<double> input); //magnitude for a vector with starting point {0,0,0}
  std::vector<double> cross_product(std::vector<double> vec_a, std::vector<double> vec_b);
  std::vector<std::vector<double>> cross_product_matrix(std::vector<double> vec);
  double inner_product(std::vector<double> vec_a, std::vector<double> vec_b);
  std::vector<double> mult_vector_scalar(std::vector<double> vec, double scalar);
  std::vector<std::vector<double>> mult_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b); //multiplicate 3x3 matrix with 3x3
  std::vector<std::vector<double>> mult_matrix_scalar(std::vector<std::vector<double>> mat_a, double scalar); //multiplicate 3x3 matrix with scalar
  std::vector<std::vector<double>> add_matrix(std::vector<std::vector<double>> mat_a, std::vector<std::vector<double>> mat_b); //addition of 3x3 matrix
  std::vector<double> mult_matrix_vector(std::vector<std::vector<double>> mat, std::vector<double> vec); //multiplicate 3x3 matrix with 3x1 vector
  
  bool draw_arrow(std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size); //draw an arrow from a center point, vector, color, size
  bool draw_arrow_flat(std::vector<double> start_point, std::vector<double> direction, bool from_start_point, std::string color, double size); //draw an arrow from a center point, vector, color, size
  bool draw_dof(std::vector<double> coord, int dof, std::string color, double size); //draw dof (1,2,3,4,5,6,11) symbol at coord, color, size
  
  bool draw_load_force(int id, double size = 1); // draw load force with id
  bool draw_load_pressure(int id, double size = 1); // draw load pressure with id
  bool draw_load_heatflux(int id, double size = 1); // draw load heatflux with id
  bool draw_load_gravity(int id, double size = 1); // draw load gravity with id
  bool draw_load_centrifugal(int id, double size = 1); // draw centrifugal load with id
  bool draw_bc_displacement(int id, double size = 1); // draw bc displacement
  bool draw_bc_temperature(int id, double size = 1); // draw bc temperature
  bool draw_orientation(int id, double size = 1); // draw orientation for all linked section

  bool draw_loads(double size = 1); // draw all loads
  bool draw_bcs(double size = 1); // draw all bcs
  bool draw_orientations(double size = 1); // draw all orientations for all linked section
  bool draw_all(double size = 1); // draw all

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREDRAW_HPP