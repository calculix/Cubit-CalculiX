#include "AnimationCaptureTests.hpp"
#include "Postprocessor/Core/capture/AnimationCapturePlan.hpp"
#include "Postprocessor/Core/views/CameraOperations.hpp"
#include <QTemporaryDir>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkNew.h>
#include <vtkCellType.h>
#include <cmath>
#include <iostream>
int animation_capture_cases()
{
  using namespace Postprocessor;
  int failures = 0;
  const auto check = [&](bool value,const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  QTemporaryDir directory;
  auto mesh = std::shared_ptr<vtkUnstructuredGrid>(vtkUnstructuredGrid::New(),[](auto* value)
  {
    value->Delete();
  });
  vtkNew<vtkPoints> points;
  points->InsertNextPoint(0,0,0);
  mesh->SetPoints(points);
  const vtkIdType vertex = 0;
  mesh->InsertNextCell(VTK_VERTEX,1,&vertex);
  auto data = std::make_shared<SimResults::VtkResultsData>();
  data->increments.resize(3);
  data->increments[0].dataset = mesh;
  data->increments[2].dataset = mesh; // Skip unavailable frame 2.
  CaptureSnapshot snapshot;
  snapshot.target_id = "view1";
  snapshot.sources["source"] = data;
  CameraDefinition base;
  base.position = {0,-10,0};
  base.view_up = {0,0,1};
  snapshot.session.views.at("view1").camera = base;
  Representation display;
  display.id = "display";
  display.view_id = "view1";
  display.input = {OutputOwner::Source,"source",0};
  display.animation.rotation[2] = {true,180};
  snapshot.session.representations[display.id] = display;
  snapshot.session.active_representation_id = display.id;
  AnimationCaptureDefinition request;
  request.output.format = Framework::Media::AnimationFormat::Png;
  request.output.path = directory.filePath("sequence");
  request.output.fps = 60;
  request.result_fps = 10;
  request.passes = 2;
  AnimationCapturePlan plan;
  std::string error;
  check(plan.prepare(snapshot,"source",request,error) && plan.frames == std::vector<std::size_t>({0,2}) &&
    plan.output.frames == 24,"Animation timeline did not skip unavailable frames or resample FPS");
  CaptureSnapshot sample;
  check(plan.sample(3,sample,error) && sample.session.representations.at(display.id).increment_index == 0,
    "Smooth camera sample changed the result frame too early");
  if (sample.session.views.at("view1").camera)
  {
    const auto& camera = *sample.session.views.at("view1").camera;
    check(std::abs(camera.position[0]-std::sqrt(50.0)) < 1e-10,"Camera was not interpolated between result frames");
  }
  check(plan.sample(6,sample,error) && sample.session.representations.at(display.id).increment_index == 2,
    "Animation sample selected an unavailable raw frame");
  check(plan.sample(12,sample,error) && sample.session.representations.at(display.id).increment_index == 0 &&
    std::abs(sample.session.views.at("view1").camera->position[1]-10) < 1e-10,
    "Camera phase reset at the exported pass boundary");
  check(CameraOperations::equal(*snapshot.session.views.at("view1").camera,base) &&
    snapshot.session.representations.at(display.id).increment_index == 0,"Animation planning mutated live definitions");
  check(!plan.sample(plan.output.frames,sample,error),"Animation accepted an out-of-range sample");
  request.image.background = CaptureBackground::Transparent;
  request.output.format = Framework::Media::AnimationFormat::Gif;
  check(!valid_animation_capture(request,error),"GIF accepted unsupported transparent output");
  request.image.background = CaptureBackground::Current;
  request.first = 3;
  request.last = 1;
  check(!valid_animation_capture(request,error),"Animation accepted reversed frame bounds");
  return failures;
}
