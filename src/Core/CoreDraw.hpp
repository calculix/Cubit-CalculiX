#ifndef COREDRAW_HPP
#define COREDRAW_HPP

#include <vector>
#include <string>

class CalculiXCoreInterface;

// class for drawing of loads and bcs

//draw polygon location pos -1 -1 0 location pos 1 -1 0 location pos 1 1 0 location pos -1 1 0 color yellow no_flush
//draw line location pos -1 0 0 location pos 1 0 0 color blue no_flush
//draw line location pos 0 -1 0 location pos 0 1 0 color blue no_flush
//draw location pos 0 0 0 color red no_flush
//graphics flush 

class CoreDraw
{

public:
  CoreDraw();
  ~CoreDraw();

  CalculiXCoreInterface *ccx_iface;
};

#endif // COREDRAW_HPP