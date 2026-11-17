#pragma once
#include "PostprocessorColorMap.hpp"
#include <vtkOpenGLActor.h>
class PostprocessorDepthActor : public vtkOpenGLActor
{
public:
  static PostprocessorDepthActor* New();
  vtkTypeMacro(PostprocessorDepthActor,vtkOpenGLActor);
  // 0: normal color/depth; 1: depth only; 2: no drawing. CPU picking remains available.
  void set_mode(int value);
  void Render(vtkRenderer* renderer,vtkMapper* mapper) override;
protected:
  PostprocessorDepthActor() = default;
  ~PostprocessorDepthActor() override = default;
private:
  int mode = 0;
};
void apply_dataset_appearance(PostprocessorColorSurface& surface,const Postprocessor::DatasetAppearance& appearance,
  const SimResults::Dataset* metadata,bool visible);
