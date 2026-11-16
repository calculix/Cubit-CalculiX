#pragma once
#include "DrawingExport.hpp"
#include <string>
class MeshOutputGroup;
struct CubitColor;

class CUBITSIM_DRAWING_EXPORT DrawingCaptureColors
{
public:
  DrawingCaptureColors();
  // Alpha zero is Cubit's unset/default sentinel, not an assigned black.
  // Failure preserves output. Explicit opaque black remains valid.
  static bool encode(const CubitColor& color, std::string& output, std::string& error);
  // GUI-thread-only query of the active native Showviz simulation interface.
  // Never draws, changes colors, or substitutes a guessed fallback color.
  static bool block(MeshOutputGroup* block, std::string& output, std::string& error);
};
