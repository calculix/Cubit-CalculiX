#ifndef COREDRAWGRAPHICS_HPP
#define COREDRAWGRAPHICS_HPP

#include "CoreDrawGeometry.hpp"
#include "CoreDrawTypes.hpp"
#include <map>

class CoreDrawGraphics
{
public:
  bool synchronize(const std::map<DrawGroup, DrawGeometry>& geometry, const CalculiXDrawState& state, bool workspaceActive, std::string& error);
  void detach() noexcept;
private:
  struct NativeVisibility
  {
    bool geometry = true;
    bool mesh = true;
    bool simulation = true;
  };
  std::map<const void*, NativeVisibility> nativeVisibility;
};
#endif
