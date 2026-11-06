#ifndef CUBITSIM_DRAWINGHITTEST_HPP
#define CUBITSIM_DRAWINGHITTEST_HPP

#include "DrawingPickData.hpp"
class GView;

class DrawingHitTest
{
public:
  bool pick(GView& view, float x, float y, const DrawingPickSnapshot& snapshot, DrawingPickHit& result) const;
};

#endif
