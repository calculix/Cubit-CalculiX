#ifndef COREDRAW_HPP
#define COREDRAW_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

// class for drawing of loads and bcs

class CoreDraw
{

public:
  CoreDraw();
  ~CoreDraw();

  bool is_initialized = false;

  bool init(); // initialize
  std::vector<double> rotate(std::vector<double> coord, std::vector<double> direction);
  bool draw_arrow(std::vector<double> center, std::vector<double> direction, std::string color, double size); //draw an arrow from a center point, vector, color, size
  bool draw_all(); // draw all loads and bcs

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREDRAW_HPP