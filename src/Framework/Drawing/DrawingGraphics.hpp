#ifndef CUBITSIM_DRAWINGGRAPHICS_HPP
#define CUBITSIM_DRAWINGGRAPHICS_HPP

#include "DrawingTypes.hpp"
#include <map>
#include <set>

class DrawingGraphics
{
public:
  bool synchronize(const std::string& workspace, const std::map<std::string, DrawingGeometry>& geometry, const DrawingFrame& state, std::string& error);
  void detach() noexcept;
private:
  struct NativeVisibility
  {
    bool geometry = true;
    bool mesh = true;
    bool simulation = true;
  };
  std::map<const void*, NativeVisibility> nativeVisibility;
  std::set<std::string> ownedSets;
};
#endif
