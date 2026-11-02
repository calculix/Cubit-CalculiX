#ifndef COREDRAWHOVER_HPP
#define COREDRAWHOVER_HPP

#include "CoreDrawGeometry.hpp"

class CoreDrawHover
{
public:
  void show(const DrawGeometry& geometry, const std::string& color);
  void clear() noexcept;
  void detach() noexcept;
private:
  bool visible = false;
};

#endif
