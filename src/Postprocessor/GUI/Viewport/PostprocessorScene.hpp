#ifndef POSTPROCESSORSCENE_HPP
#define POSTPROCESSORSCENE_HPP

#include "Postprocessor/Core/views/ViewDefinition.hpp"
#include "Framework/Results/VtkResultsData.hpp"
#include <vtkSmartPointer.h>
#include "PostprocessorColorMap.hpp"
#include "Postprocessor/Core/representations/ResultDeformation.hpp"
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
  bool pick(double x, double y, Postprocessor::EntityKind kind,
    std::optional<Postprocessor::EntityReference>& entity, std::string& error);
  bool set_hover(const std::optional<Postprocessor::EntityReference>& entity, const PostprocessorApplicationInterface& application);
  bool clear_hover();
  const std::string& warning() const noexcept;
  void apply_appearance(const std::array<double, 3>& top, const std::array<double, 3>& bottom);

private:
  struct RepresentationActors
  {
    std::shared_ptr<const SimResults::VtkResultsData> data;
    std::string source_id;
    std::size_t increment = 0;
    Postprocessor::ResultDeformation deformation;
    std::vector<std::shared_ptr<vtkDataSet>> transformed;
    std::map<std::size_t,std::string> warnings;
    struct DatasetActor
    {
      std::size_t dataset;
      vtkSmartPointer<vtkActor> actor;
    };
    std::vector<DatasetActor> actors;
    std::vector<PostprocessorColorSurface> surfaces;
    std::unique_ptr<PostprocessorColorMap> coloring;
  };
  std::shared_ptr<RepresentationActors> build(const std::shared_ptr<const SimResults::VtkResultsData>& data,
    const Postprocessor::Representation& display, std::string& error);
  vtkSmartPointer<vtkRenderer> renderer;
  std::string viewId;
  std::optional<std::uint64_t> geometryRevision;
  std::optional<std::array<double, 6>> visibleBounds;
  std::optional<Postprocessor::CameraDefinition> appliedCamera;
  std::map<std::string, std::shared_ptr<RepresentationActors>> displays;
  std::string displayWarning;
  void update_selection(const PostprocessorApplicationInterface& application);
  void build_highlight(const Postprocessor::EntityReference& entity, const PostprocessorApplicationInterface& application,
    vtkSmartPointer<vtkActor>& actor, const std::array<double,3>& color);
  vtkSmartPointer<vtkActor> highlight;
  vtkSmartPointer<vtkActor> hoverHighlight;
  std::optional<Postprocessor::EntityReference> hoveredEntity;
  std::optional<std::uint64_t> highlightedSelection, highlightedGeometry;
};

#endif
