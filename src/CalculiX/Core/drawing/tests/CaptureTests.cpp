#include "LabelTestDrawing.hpp"
#include "Core/drawing/CoreDrawScene.hpp"
#include "Core/drawing/CalculiXCaptureRequest.hpp"
#include "Framework/Drawing/DrawingCaptureRenderer.hpp"
#include "CubitColorTable.hpp"
#include <QGuiApplication>
#include <iostream>
#include <limits>

// These cases do not render a window or require a Cubit model. Successful
// native rendering and model/step parity are covered by the host checklist.
int reportImageSettingsTests();
int jobSnapshotTests();

int captureTests(int argc, char** argv)
{
  QGuiApplication application(argc, argv);
  int failures = 0;
  const auto check = [&](bool value, const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  CalculiXCaptureRequest request;
  DrawingCaptureRenderer renderer;
  DrawingFrame frame;
  frame.camera.available = true;
  DrawingCaptureCamera restored;
  check(restored.fromJson(frame.camera.toJson()) && restored.valid(), "Camera metadata round-trips independently of the viewport");
  auto corruptCamera = frame.camera.toJson();
  corruptCamera["scale"] = -1;
  check(!restored.fromJson(corruptCamera) && restored.valid(), "Invalid camera metadata preserves the previous camera");
  restored.up = {0, 0, 1};
  check(!restored.valid(), "A camera view-up parallel to its viewing direction is rejected");
  QByteArray output = "previous image";
  auto invalid = request.image;
  invalid.width = 0;
  check(!renderer.render(frame, invalid, output, error) && output == "previous image" && !error.empty(),
    "Invalid dimensions preserve the previous image");
  invalid = request.image;
  invalid.width = 8192;
  invalid.height = 8192;
  check(!renderer.render(frame, invalid, output, error) && output == "previous image",
    "The pixel budget is checked before allocating rendering resources");
  invalid = request.image;
  invalid.background[0] = std::numeric_limits<double>::quiet_NaN();
  check(!renderer.render(frame, invalid, output, error) && output == "previous image",
    "A nonfinite background cannot enter the renderer");
  check(!renderer.render(frame, request.image, output, error) && output == "previous image" && !error.empty(),
    "An empty scene cannot succeed with a blank PNG");
  DrawingGeometry geometry;
  geometry.point({0, 0, 0}, "black");
  frame.layers["model"] = {true, false};
  geometry.primitives.front().points.front()[0] = std::numeric_limits<double>::infinity();
  frame.entities.push_back({{"calculix", "model", 0, ""}, "model", geometry, false});
  check(!renderer.render(frame, request.image, output, error) && output == "previous image",
    "Invalid visible geometry is rejected before VTK rendering");
  int colorId = -1;
  double color[3] = {};
  check(CubitColorTable::string_to_color_id("grey", colorId) && CubitColorTable::color_id_to_rgb(colorId, color),
    "The capture mesh color resolves through the host Cubit color table");
  frame.entities.front().geometry.primitives.front().points.front()[0] = 0;
  frame.entities.front().geometry.primitives.front().color = "unsupported_capture_test_color";
  check(!renderer.render(frame, request.image, output, error) && output == "previous image" &&
      error.find("unsupported_capture_test_color") != std::string::npos && error.find("model 0") != std::string::npos,
    "Unknown capture colors identify the color and owning entity without replacing the previous image");
  LabelTestDrawing service;
  CoreDrawScene scene;
  scene.bind(&service);
  scene.workspaceActive = false;
  scene.state.scope = {DrawScopeKind::Step, 4};
  scene.state.labels.enabled = false;
  scene.state.pickingEnabled = true;
  service.frame.selection = {true, {"calculix", "force", 7, "step:4"}};
  service.frame.layers["existing"] = {true, false};
  frame.entities.clear();
  check(scene.capture(frame, request.image, output, error) && !service.activated &&
      service.frame.selection.active && service.frame.selection.entity.id == 7 && service.frame.layers.count("existing") == 1 &&
      !scene.workspaceActive && scene.state.scope.stepId == 4 && !scene.state.labels.enabled && scene.state.pickingEnabled,
    "Capturing an inactive scene never activates it, submits it, or changes current drawing settings");
  output = "previous image";
  service.captureSucceeds = false;
  check(!scene.capture(frame, request.image, output, error) && output == "previous image" && !error.empty(),
    "Service capture failure preserves the previous output");
  scene.bind(nullptr);
  check(!scene.capture(frame, request.image, output, error) && output == "previous image" && !error.empty(),
    "An unavailable capture service returns an error without modifying output");
  return failures + reportImageSettingsTests() + jobSnapshotTests();
}
