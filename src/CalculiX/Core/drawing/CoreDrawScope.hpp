#ifndef COREDRAWSCOPE_HPP
#define COREDRAWSCOPE_HPP

#include "CoreDrawScene.hpp"
class CalculiXCore;

struct ResolvedDrawScope
{
  std::vector<DrawEntityKey> entities;
  std::map<DrawEntityKey, DrawGeometry> geometry;
};

class CoreDrawScope
{
public:
  static bool resolveModel(CalculiXCore& core, ResolvedDrawScope& result, std::string& error);
  static bool resolveStep(CalculiXCore& core, int stepId, double size, ResolvedDrawScope& result, std::string& error);
};
#endif
