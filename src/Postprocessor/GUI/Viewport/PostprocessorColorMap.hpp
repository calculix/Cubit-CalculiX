#ifndef POSTPROCESSOR_COLORMAP_HPP
#define POSTPROCESSOR_COLORMAP_HPP
#include "Postprocessor/Core/representations/Representation.hpp"
#include <vtkSmartPointer.h>
#include <optional>
class vtkActor;
class vtkDataSet;
class vtkPolyData;
class vtkRenderer;
class vtkScalarBarActor;
class vtkTextActor;

struct PostprocessorColorSurface
{
  std::size_t dataset = 0;
  vtkDataSet* input = nullptr; // Borrowed from the owning representation snapshot.
  vtkSmartPointer<vtkActor> actor;
  vtkSmartPointer<vtkPolyData> base;
  vtkSmartPointer<vtkPolyData> scalars;
  std::optional<std::array<double,2>> range;
  vtkSmartPointer<vtkActor> nodes;
  vtkSmartPointer<vtkPolyData> node_data;
};
class PostprocessorColorMap
{
public:
  explicit PostprocessorColorMap(vtkRenderer* renderer);
  ~PostprocessorColorMap();
  void update_appearance();
  void set_capture_scale(double scale);
  void apply(const Postprocessor::Representation& display, const SimResults::VtkResultsData& data,
    std::vector<PostprocessorColorSurface>& surfaces, unsigned int slot,
    const std::optional<std::array<double,2>>& rangeAcrossFrames, std::string& error);
private:
  double captureScale = 1;
  vtkRenderer* renderer;
  vtkSmartPointer<vtkScalarBarActor> legend;
  vtkSmartPointer<vtkTextActor> minimumLabel;
  vtkSmartPointer<vtkTextActor> maximumLabel;
  std::optional<Postprocessor::ResultColoring> applied;
  std::optional<std::array<double, 2>> actualRange;
  std::optional<std::array<double, 2>> allFrameRange;
  std::set<std::size_t> hiddenDatasets;
  std::string title;
  const SimResults::VtkResultsData* appliedData = nullptr;
  std::optional<std::size_t> appliedFrame;
  std::string appliedDisplayName;
  unsigned int appliedSlot = 0;
};
#endif
