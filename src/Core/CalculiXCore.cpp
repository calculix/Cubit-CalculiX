#include "CalculiXCore.hpp"

CalculiXCore::CalculiXCore()
{}

CalculiXCore::~CalculiXCore()
{}

std::string CalculiXCore::test()
{

  std::string s_return;
  s_return = std::to_string(testint++);
  return s_return;
}

