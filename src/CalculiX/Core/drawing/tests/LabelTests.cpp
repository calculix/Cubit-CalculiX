#include "LabelTestDrawing.hpp"
#include "Core/drawing/CalculiXDrawLabels.hpp"
#include "Core/drawing/CoreDrawScene.hpp"
#include <iostream>
#include <limits>

namespace
{
int failures = 0;
void check(bool condition, const char* message)
{
  if (!condition)
  {
    std::cerr << message << '\n';
    ++failures;
  }
}
}

int captureTests(int argc, char** argv);

int main(int argc, char** argv)
{
  std::string error;
  DrawLabelSettings settings;
  DrawLabelSettings restored;
  settings.showValues = true;
  check(DrawLabelSettings::from_json(settings.to_json(), restored, error) && restored.to_json() == settings.to_json(),
    "Label settings round-trip without persisting drawing geometry");
  auto malformed = settings.to_json();
  malformed["enabled"] = "true";
  check(!DrawLabelSettings::from_json(malformed, restored, error) && restored.to_json() == settings.to_json(),
    "Malformed label settings preserve the previous output");
  DrawLabelData data;
  data.entity = {DrawFeature::Displacement, 3};
  data.typeName = "Displacement BC";
  data.name = "Support";
  data.values = {{"UX", "0"}, {"UY", "free"}, {"UZ", "FIXED"}};
  data.anchors = {{{1, 2, 3}}};
  const auto label = CalculiXDrawLabels::text(data, settings);
  check(label.find("UX=0") != std::string::npos && label.find("UY=free") != std::string::npos && label.find("UZ=FIXED") != std::string::npos,
    "Label text distinguishes free, zero and FIXED boundary components");
  data.amplitude = "2";
  settings.showAmplitude = true;
  check(CalculiXDrawLabels::text(data, settings).find("Amplitude 2 (reference)") != std::string::npos,
    "Amplitude is identified as a reference rather than evaluated output");
  data.secondaryAmplitude = "3";
  check(CalculiXDrawLabels::text(data, settings).find("Secondary amplitude 3 (reference)") != std::string::npos,
    "Secondary amplitudes are also references");
  settings.showAmplitude = false;
  check(CalculiXDrawLabels::text(data, settings).find("amplitude") == std::string::npos &&
    CalculiXDrawLabels::text(data, settings).find("Amplitude") == std::string::npos,
    "Amplitude visibility applies to both references independently of values");
  settings.showAmplitude = true;
  data.stepId = 2;
  data.values.clear();
  CalculiXDrawLabels::resolved_value(data, "FX", "1");
  CalculiXDrawLabels::resolved_value(data, "FX", "2");
  check(CalculiXDrawLabels::text(data, settings).find("Step 2") != std::string::npos && data.values.front().second == "varies by target",
    "Nonuniform step values are not mislabeled with one global value");
  DrawingGeometry base;
  check(base.point({1, 2, 3}, "green"), "Create a symbol fixture");
  auto decorated = base;
  check(CalculiXDrawLabels::append(data, settings, 1, decorated, error) && decorated.primitives.size() == 3 && base.primitives.size() == 1,
    "Labels and leaders are added without mutating cached base symbols");
  auto invalid = data;
  invalid.anchors[0][0] = std::numeric_limits<double>::quiet_NaN();
  const auto count = decorated.primitives.size();
  check(!CalculiXDrawLabels::append(invalid, settings, 1, decorated, error) && decorated.primitives.size() == count,
    "Invalid anchors cannot partially append label geometry");
  LabelTestDrawing renderer;
  CoreDrawScene scene;
  scene.bind(&renderer);
  scene.workspaceActive = true;
  scene.state.groups[DrawGroup::BoundaryConditions].enabled = true;
  scene.state.groups[DrawGroup::BoundaryConditions].allInScope = true;
  scene.state.groups[DrawGroup::BoundaryConditions].sourceScope = {DrawScopeKind::Step, 2};
  scene.state.labels = settings;
  scene.replace(data.entity, base, &data);
  check(scene.synchronize(error) && renderer.frame.entities.size() == 1 && renderer.frame.entities[0].geometry.primitives.size() == 3 &&
    renderer.frame.entities[0].key.id == 3 && renderer.frame.entities[0].key.context == "step:2", "Labels share the symbol's owner and step identity");
  check(scene.synchronize(error) && renderer.frame.entities[0].geometry.primitives.size() == 3,
    "Repeated refresh does not duplicate labels");
  scene.state.labels.enabled = false;
  check(scene.synchronize(error) && renderer.frame.entities[0].geometry.primitives.size() == 1,
    "Turning labels off preserves symbol geometry");
  scene.state.labels.enabled = true;
  scene.state.labels.selectedOnly = true;
  check(scene.synchronize(error) && renderer.frame.entities[0].geometry.primitives.size() == 1,
    "Selected-only mode hides labels for unselected symbols");
  scene.setSelection({true, data.entity, -1});
  check(scene.synchronize(error) && renderer.frame.entities[0].geometry.primitives.size() == 3,
    "Selected-only mode shows labels for the selected owner");
  scene.state.labels.boundaryConditions = false;
  check(scene.synchronize(error) && renderer.frame.entities[0].geometry.primitives.size() == 1,
    "BC label visibility is independent of symbols and load labels");
  scene.clear(DrawGroup::BoundaryConditions);
  check(scene.synchronize(error) && renderer.frame.entities.empty(), "Removing an entity removes cached labels");
  failures += captureTests(argc, argv);
  std::cout << failures << " drawing label/capture failures\n";
  return failures == 0 ? 0 : 1;
}
