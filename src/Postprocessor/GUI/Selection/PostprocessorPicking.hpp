#ifndef POSTPROCESSOR_PICKING_HPP
#define POSTPROCESSOR_PICKING_HPP
class vtkDataSetSurfaceFilter;
namespace Postprocessor
{
  void configure_pick_provenance(vtkDataSetSurfaceFilter* filter);
}
#endif
