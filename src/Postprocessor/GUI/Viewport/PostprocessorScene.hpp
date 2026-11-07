#ifndef POSTPROCESSORSCENE_HPP
#define POSTPROCESSORSCENE_HPP

#include "Postprocessor/Core/views/ViewDefinition.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include <vtkSmartPointer.h>
#include <map>
#include <memory>
#include <vector>

class vtkActor;
class vtkRenderer;
class PostprocessorApplicationInterface;

class PostprocessorScene
{
public:
  PostprocessorScene(vtkRenderer* renderer, const std::string& view_id);
  ~PostprocessorScene();
  bool synchronize(const PostprocessorApplicationInterface& application, std::string& error);
  void apply_camera(const Postprocessor::CameraDefinition& camera);
  bool prepare_camera_fit(const std::array<double, 6>& bounds, Postprocessor::CameraDefinition& camera, std::string& error);
  void clear();

private:
  struct RepresentationActors
  {
    std::shared_ptr<const SimResults::VtkResultsData> data;
    std::size_t increment = 0;
    std::vector<vtkSmartPointer<vtkActor>> actors;
  };
  std::shared_ptr<RepresentationActors> build(const std::shared_ptr<const SimResults::VtkResultsData>& data,
    std::size_t increment, std::string& error);
  vtkSmartPointer<vtkRenderer> renderer;
  std::string viewId;
  std::map<std::string, std::shared_ptr<RepresentationActors>> displays;
};

#endif
