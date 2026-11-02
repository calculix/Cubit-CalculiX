#ifndef COREDRAWHITTEST_HPP
#define COREDRAWHITTEST_HPP

#include "CoreDrawPickData.hpp"
class GView;

class CoreDrawHitTest
{
public:
  bool pick(GView& view, float x, float y, const DrawPickSnapshot& snapshot, DrawPickHit& result) const;
};

#endif
