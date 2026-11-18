#include "Postprocessor/Core/explosion/ExplosionRuntime.hpp"
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Postprocessor/Core/explosion/ExplosionCodec.hpp"
#include "Postprocessor/Core/selection/ResultProbe.hpp"
#include "Postprocessor/Core/representations/ResultMesh.hpp"
#include <QJsonArray>
#include <vtkCellData.h>
#include <vtkCellType.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkTypeInt64Array.h>
#include <vtkUnstructuredGrid.h>
#include <cmath>
#include <iostream>
#include <limits>
namespace
{
  int failures = 0;
  void check(bool accepted,const char* message)
  {
    if (!accepted)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  }
  std::shared_ptr<SimResults::VtkResultsData> fixture()
  {
    auto source = std::make_shared<SimResults::VtkResultsData>();
    source->job_id = 1;
    source->run_identity = "explosion-fixture";
    source->datasets = {{0,"Block_1","Blocks",1}};
    auto grid = vtkSmartPointer<vtkUnstructuredGrid>::New();
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    const double coordinates[][3] = {{0,0,0},{1,0,0},{0,1,0},{0,0,1},{-1,0,0},{0,-1,0},{0,0,-1}};
    auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
    ids->SetName("OriginalNodeId");
    auto values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    for (int i = 0; i < 7; ++i)
    {
      points->InsertNextPoint(coordinates[i]);
      ids->InsertNextValue(9007199254741001LL+i);
      values->InsertNextValue(7.25+i);
    }
    grid->SetPoints(points);
    grid->GetPointData()->AddArray(ids);
    grid->GetPointData()->AddArray(values);
    const vtkIdType a[] = {0,1,2,3}, b[] = {0,4,5,6};
    grid->InsertNextCell(VTK_TETRA,4,a);
    grid->InsertNextCell(VTK_TETRA,4,b);
    auto elements = vtkSmartPointer<vtkTypeInt64Array>::New();
    elements->SetName("OriginalElementId");
    elements->InsertNextValue(9007199254740993LL);
    elements->InsertNextValue(9007199254740995LL);
    grid->GetCellData()->AddArray(elements);
    SimResults::Increment frame;
    frame.dataset = grid;
    source->increments.push_back(frame);
    return source;
  }
  class Provider final : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = fixture();
    std::string command_prefix() const override
    {
      return "test";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = data->run_identity;
      output = {reference,"Assembly",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      return open(request,output,error);
    }
  };
}
int main()
{
  using namespace Postprocessor;
  std::string error;
  ExplosionState defaults;
  check(!defaults.enabled && defaults.meshes.empty(),"Explosion must default off");
  check(!defaults.pulse.enabled && defaults.pulse.period_seconds == 3.0 && explosion_pulse_factor(defaults.pulse) == 1,
    "Pulse defaults must preserve the static explosion");
  auto pulse = defaults.pulse;
  pulse.enabled = true;
  check(explosion_pulse_factor(pulse) == 1,"Pulse must start fully separated");
  pulse.elapsed_seconds = .75;
  check(std::abs(explosion_pulse_factor(pulse)-.5) < 1e-12,"Quarter-period pulse was not halfway separated");
  pulse.elapsed_seconds = 1.5;
  check(std::abs(explosion_pulse_factor(pulse)) < 1e-12,"Half-period pulse did not assemble");
  pulse.elapsed_seconds = 3;
  check(explosion_pulse_factor(pulse) == 1,"Pulse did not repeat smoothly");
  auto badPulse = defaults;
  badPulse.pulse.period_seconds = 0;
  check(!valid_explosion(badPulse,error),"Zero pulse period was accepted");
  badPulse = defaults;
  badPulse.pulse.minimum = .8;
  badPulse.pulse.maximum = .2;
  check(!valid_explosion(badPulse,error),"Reversed pulse range was accepted");
  std::vector<ExplosionPieceBounds> pieces(2);
  pieces[0].piece.source_id = "a";
  pieces[1].piece.source_id = "b";
  for (auto& piece : pieces)
  {
    piece.bounds = {0,1,0,2,0,3};
  }
  std::vector<ExplosionPlacement> layout;
  check(calculate_explosion_layout(pieces,ExplosionDirection::Automatic,.05,layout,error),"Automatic component layout failed");
  if (layout.size() == 2)
  {
    const double gap = .05*std::sqrt(14.0);
    check(std::abs(layout[0].offset[0]+layout[1].offset[0]) < 1e-12,"Layout changed the mean center");
    check(std::abs(layout[1].offset[0]-layout[0].offset[0]-1-gap) < 1e-12,"Automatic layout did not choose minimum-cost X separation");
  }
  auto clustered = pieces;
  clustered.push_back(pieces.back());
  clustered.back().piece.source_id = "c";
  clustered.back().bounds = {100,101,0,2,0,3};
  check(calculate_explosion_layout(clustered,ExplosionDirection::X,0,layout,error),"Clustered layout failed");
  if (layout.size() == 3)
  {
    check(std::abs(layout[0].offset[0]+.5) < 1e-12 && std::abs(layout[1].offset[0]-.5) < 1e-12 &&
      std::abs(layout[2].offset[0]) < 1e-12,"Separated clusters were moved by a nonminimal layout");
  }
  auto source = fixture();
  ExplosionReference reference;
  check(prepare_explosion_reference(source,reference,error),"Reference ownership failed");
  check(reference.blocks.size() == 1 && reference.elements.size() == 2,"Reference lost original elements");
  Representation display;
  display.input = {OutputOwner::Source,"s1",0};
  auto input = std::make_shared<EvaluatedOutput>();
  input->source_id = "s1";
  input->data = input->source = source;
  ExplosionState settings;
  settings.meshes["s1"].enabled = true;
  settings.meshes["s1"].amount = .5;
  std::shared_ptr<const EvaluatedOutput> output;
  check(explode_output(settings,display,reference,1,input,output,error),"Mesh explosion failed");
  if (!output)
  {
    return failures+1;
  }
  const auto mesh = result_mesh_parts(output->data->increments[0].dataset.get()).front().second;
  EntityReference shared{"s1",0,EntityKind::Node,9007199254741001LL,{}};
  const auto copies = entity_indices(mesh,shared,error);
  check(copies.size() == 2,"A shared node did not retain two exploded copies");
  EntityReference picked;
  if (!copies.empty())
  {
    check(entity_from_index(mesh,copies.front(),EntityKind::Node,"s1",0,picked,error),"Exploded node cannot be picked");
    check(picked.entity_id == shared.entity_id && picked.display_element.has_value(),"Pick rounded solver ID or lost display ownership");
    check(entity_indices(mesh,picked,error).size() == 1,"Owner-qualified node matched another display copy");
    const auto original = result_mesh_parts(source->increments[0].dataset.get()).front().second;
    check(entity_indices(original,picked,error).size() == 1,"Display ownership changed physical result lookup");
    check(mesh->GetPointData()->GetArray("VALUE")->GetComponent(copies.front(),0) == 7.25,"Explosion changed result values");
    double point[3];
    original->GetPoint(0,point);
    check(point[0] == 0 && point[1] == 0 && point[2] == 0,"Explosion mutated the published source geometry");
  }
  settings.enabled = true;
  settings.sources = {"s1"};
  ExplosionPlacement placement;
  placement.source_id = "s1";
  placement.offset = {10,0,0};
  settings.placements = {placement};
  settings.meshes["s1"].enabled = false;
  check(explode_output(settings,display,reference,2,input,output,error),"Scene-unit offset failed");
  if (output)
  {
    const auto moved = result_mesh_parts(output->data->increments[0].dataset.get()).front().second;
    double point[3];
    moved->GetPoint(0,point);
    check(std::abs(point[0]-5) < 1e-12,"Scene-unit offset was applied more than once");
    settings.pulse.enabled = true;
    settings.pulse.elapsed_seconds = .75;
    apply_explosion_pulse(*output,settings.pulse);
    moved->GetPoint(0,point);
    check(std::abs(point[0]-2.5) < 1e-12,"Pulse did not scale scene-unit component displacement");
    auto alias = vtkSmartPointer<vtkUnstructuredGrid>::New();
    alias->ShallowCopy(moved);
    ExplosionPointBuffers updated;
    remember_explosion_pulse_buffers(*output,updated);
    auto nextPhase = settings.pulse;
    nextPhase.elapsed_seconds = 1.5;
    apply_explosion_pulse_mesh(alias,nextPhase,updated);
    alias->GetPoint(0,point);
    check(std::abs(point[0]-2.5) < 1e-12,"An actor alias rewrote a point buffer already updated by Core");
    apply_explosion_pulse(*output,settings.pulse);
    moved->GetPoint(0,point);
    check(std::abs(point[0]-2.5) < 1e-12,"Repeated pulse evaluation accumulated drift");
    settings.pulse.target = ExplosionState::Pulse::Target::Mesh;
    apply_explosion_pulse(*output,settings.pulse);
    moved->GetPoint(0,point);
    check(std::abs(point[0]-5) < 1e-12,"Mesh-only pulse changed component displacement");
    settings.pulse.enabled = false;
    apply_explosion_pulse(*output,settings.pulse);
    moved->GetPoint(0,point);
    check(std::abs(point[0]-5) < 1e-12,"Stopping pulse did not restore the static separation");
    const auto original = result_mesh_parts(source->increments[0].dataset.get()).front().second;
    original->GetPoint(0,point);
    check(point[0] == 0 && point[1] == 0 && point[2] == 0 &&
      moved->GetPointData()->GetArray("VALUE")->GetComponent(0,0) == 7.25,
      "Pulse changed original geometry or solver values");
  }
  ExplosionState restored;
  check(explosion_from_json(explosion_to_json(settings),restored,error) && restored.enabled &&
    restored.placements.size() == 1 && restored.placements.front().offset[0] == 10,"Explosion definitions did not round-trip");
  settings.pulse.enabled = true;
  settings.pulse.period_seconds = 4;
  settings.pulse.minimum = .2;
  check(explosion_from_json(explosion_to_json(settings),restored,error) && !restored.pulse.enabled &&
    restored.pulse.elapsed_seconds == 0 && restored.pulse.period_seconds == 4 && restored.pulse.minimum == .2 &&
    restored.pulse.target == ExplosionState::Pulse::Target::Mesh,"Pulse settings did not round-trip with playback stopped");
  auto legacy = explosion_to_json(settings);
  legacy.remove("pulse");
  check(explosion_from_json(legacy,restored,error) && restored.pulse.period_seconds == 3 && !restored.pulse.enabled,
    "Legacy explosion settings did not use pulse defaults");
  auto invalid = explosion_to_json(settings);
  invalid.insert("amount",-1);
  check(!explosion_from_json(invalid,restored,error),"Negative saved amount was accepted");
  settings.amount = std::numeric_limits<double>::infinity();
  check(!valid_explosion(settings,error),"Nonfinite explosion was accepted");
  Provider provider;
  PostprocessorCore core;
  check(core.open_result(provider,SimResults::JobSourceReference{"test","",1,""},std::nullopt,error),"Cannot open explosion source");
  check(core.change_explosion("view1",{},"on",0,"","",{},"",error),"Bare explosion failed for one block");
  const auto sourceId = core.session().active_source_id;
  const auto& firstSettings = core.session().views.at("view1").explosion;
  check(!firstSettings.enabled && firstSettings.meshes.count(sourceId) == 1 && firstSettings.meshes.at(sourceId).enabled,
    "Bare single-block explosion did not choose mesh defaults");
  const auto revision = core.display_revision();
  check(!core.change_explosion("view1",{},"mesh-amount",-1,"","",{},"",error) && core.display_revision() == revision,
    "Invalid explosion published partial state");
  check(core.change_explosion("view1",{},"pulse-on",0,"","",{},"",error),"Cannot start explosion pulse");
  const auto pulseRevision = core.display_revision();
  const auto fitRequest = core.session().views.at("view1").fit_request;
  const auto cameraRevision = core.session().views.at("view1").camera_revision;
  check(core.change_explosion("view1",{},"pulse-advance",.75,"","",{},"",error) &&
    core.display_revision() == pulseRevision && core.session().views.at("view1").fit_request == fitRequest &&
    core.session().views.at("view1").camera_revision == cameraRevision &&
    std::abs(core.session().views.at("view1").explosion.pulse.elapsed_seconds-.75) < 1e-12,
    "Pulse tick rebuilt the scene, refitted the camera or lost elapsed time");
  check(!core.change_explosion("view1",{},"pulse-advance",std::numeric_limits<double>::infinity(),"","",{},"",error),
    "Nonfinite pulse elapsed time was accepted");
  const auto& display = core.session().representations.at(core.session().active_representation_id);
  std::shared_ptr<const EvaluatedOutput> beforeEdit,afterEdit;
  check(core.evaluate_output(display,beforeEdit,error),"Cannot evaluate geometry before editing pulse settings");
  const auto settingsRevision = core.session().views.at("view1").explosion.pulse.settings_revision;
  check(core.change_explosion("view1",{},"pulse-range",0,"","",{.3,1,0},"",error) &&
    core.evaluate_output(display,afterEdit,error) && beforeEdit && beforeEdit == afterEdit &&
    core.display_revision() == pulseRevision &&
    core.session().views.at("view1").explosion.pulse.settings_revision == settingsRevision+1,
    "Editing the pulse range replaced the cached geometry or missed its redraw notification");
  check(core.change_explosion("view1",{},"pulse-period",6,"","",{},"",error) &&
    std::abs(core.session().views.at("view1").explosion.pulse.elapsed_seconds-1.5) < 1e-12,
    "Editing the pulse period did not preserve the cycle phase");
  check(!core.change_explosion("view1",{},"pulse-range",0,"","",{.9,.1,0},"",error) &&
    core.session().views.at("view1").explosion.pulse.minimum == .3,
    "An invalid range changed the active pulse");
  check(core.change_explosion("view1",{},"pulse-off",0,"","",{},"",error) &&
    !core.session().views.at("view1").explosion.pulse.enabled,"Cannot stop explosion pulse");
  check(core.change_explosion("view1",{},"off",0,"","",{},"",error),"Explosion off failed");
  check(!core.explosion_active(core.session().representations.at(core.session().active_representation_id)),"Explosion off left geometry displaced");
  check(core.change_explosion("view1",{},"reset",0,"","",{},"",error) && core.session().views.at("view1").explosion.meshes.empty(),
    "Explosion reset retained stale mesh configuration");
  return failures;
}
