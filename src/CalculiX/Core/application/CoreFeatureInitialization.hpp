#ifndef COREFEATUREINITIALIZATION_HPP
#define COREFEATUREINITIALIZATION_HPP

namespace CoreFeatureInitialization
{
template<typename Feature>
void initialize(Feature*& feature)
{
  if (!feature)
  {
    feature = new Feature;
  }

  feature->init();
}
}

#endif
