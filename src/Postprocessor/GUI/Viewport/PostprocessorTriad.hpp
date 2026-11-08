#ifndef POSTPROCESSOR_TRIAD_HPP
#define POSTPROCESSOR_TRIAD_HPP
#include <vtkSmartPointer.h>
#include <array>

class vtkCamera;
class vtkRenderer;
class vtkRenderWindow;
class vtkPolyData;
class vtkPoints;
class vtkCellArray;
class vtkUnsignedCharArray;

// A noninteractive overlay; never participates in scene bounds, selection or
// fitting. Concrete OpenGL props avoid changing the host's VTK factories.
class PostprocessorTriad
{
public:
  explicit PostprocessorTriad(vtkRenderWindow* window);
  ~PostprocessorTriad();
  void update(vtkCamera* camera, bool visible, const std::array<double, 3>& background,
    int width, int height, double pixel_ratio);
private:
  vtkRenderWindow* window;
  vtkSmartPointer<vtkRenderer> overlay;
  vtkSmartPointer<vtkPolyData> geometry;
  vtkSmartPointer<vtkPoints> points;
  vtkSmartPointer<vtkCellArray> lines;
  vtkSmartPointer<vtkUnsignedCharArray> colors;
  std::array<double, 3> lastDirection = {};
  std::array<double, 3> lastUp = {};
  std::array<double, 3> lastBackground = {};
  int lastWidth = 0;
  int lastHeight = 0;
  double lastPixelRatio = 0;
  bool initialized = false;
};
#endif
