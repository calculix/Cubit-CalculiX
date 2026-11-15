#pragma once
#include "CoreFrameworkUnits.hpp"

class CUBITSIM_MODEL_EXPORT FrameworkUnitsInterface {
public:
  FrameworkUnitsInterface();
  SimUnits::CoreFrameworkUnits& core() const noexcept;
};
