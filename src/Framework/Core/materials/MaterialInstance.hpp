#pragma once
#include "MaterialData.hpp"
#include "MaterialRef.hpp"
struct MaterialInstance {
  MaterialRef reference;
  MaterialData data;
};
