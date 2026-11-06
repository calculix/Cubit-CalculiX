#ifndef CUBITSIM_DRAWINGHOVER_HPP
#define CUBITSIM_DRAWINGHOVER_HPP

#include "DrawingGeometry.hpp"

class DrawingHover
{
public:
  void show(const DrawingGeometry& geometry, const std::string& color);
  void clear() noexcept;
  void detach() noexcept;
private:
  bool attached = false;
  bool visible = false;
};

#endif
