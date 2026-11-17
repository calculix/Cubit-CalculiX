#include <type_traits>
#include "Postprocessor/Core/application/PostprocessorCore.hpp"
#include "Postprocessor/Core/filters/FilterCodec.hpp"
#include "Postprocessor/Core/persistence/PostprocessorSessionCodec.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include <QApplication>
#include <QJsonObject>
#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/Filters/Clip/PostprocessorClipControls.hpp"
#include "Postprocessor/GUI/Filters/Section/PostprocessorSectionControls.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include <vtkOpenGLRenderer.h>
#include <vtkActor.h>
#include <vtkActorCollection.h>
#include <vtkProperty.h>
#include <QSlider>
#include <QTimer>
#include <QKeyEvent>
#include <sstream>
#include <iomanip>
#include <vtkUnstructuredGrid.h>
#include <vtkPoints.h>
#include <vtkPointData.h>
#include <vtkCellData.h>
#include <vtkDoubleArray.h>
#include <vtkTypeInt64Array.h>
#include <vtkPolyData.h>
#include <vtkSmartPointer.h>
#include <vtkVariant.h>
#include <vtkCellType.h>
#include <iostream>
#include <cmath>
namespace
{
  int failures = 0;
  void check(bool value,const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  }
  std::shared_ptr<SimResults::VtkResultsData> fixture(bool quadratic)
  {
    auto data = std::make_shared<SimResults::VtkResultsData>();
    data->job_id = 1;
    data->run_identity = "filter-fixture";
    auto mesh = std::shared_ptr<vtkUnstructuredGrid>(vtkUnstructuredGrid::New(),[](auto* value)
    {
      value->Delete();
    });
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    const double coordinates[][3] = {{0,0,0},{1,0,0},{1,1,0},{0,1,0},{0,0,1},{1,0,1},{1,1,1},{0,1,1},
      {.5,0,0},{1,.5,0},{.5,1,0},{0,.5,0},{.5,0,1},{1,.5,1},{.5,1,1},{0,.5,1},{0,0,.5},{1,0,.5},{1,1,.5},{0,1,.5}};
    const int count = [&]() -> int {
      if (quadratic) {
        return 20;
      } else {
        return 8;
      }
    }();
    auto values = vtkSmartPointer<vtkDoubleArray>::New();
    values->SetName("VALUE");
    auto ids = vtkSmartPointer<vtkTypeInt64Array>::New();
    ids->SetName("OriginalNodeId");
    for (int i = 0; i < count; ++i)
    {
      points->InsertNextPoint(coordinates[i]);
      values->InsertNextValue(coordinates[i][0]);
      ids->InsertNextValue(9007199254740993LL+i);
    }
    mesh->SetPoints(points);
    vtkIdType vertices[20];
    for (int i = 0; i < count; ++i)
    {
      vertices[i] = i;
    }
    mesh->InsertNextCell([&]() -> std::common_type_t<decltype(VTK_QUADRATIC_HEXAHEDRON), decltype(VTK_HEXAHEDRON)> {
      if (quadratic) {
        return VTK_QUADRATIC_HEXAHEDRON;
      } else {
        return VTK_HEXAHEDRON;
      }
    }(),count,vertices);
    mesh->GetPointData()->AddArray(values);
    mesh->GetPointData()->AddArray(ids);
    auto elements = vtkSmartPointer<vtkTypeInt64Array>::New();
    elements->SetName("OriginalElementId");
    elements->InsertNextValue(9007199254741093LL);
    mesh->GetCellData()->AddArray(elements);
    SimResults::Increment frame;
    frame.dataset = mesh;
    frame.step = frame.increment = 1;
    frame.time = .1;
    SimResults::Field field;
    field.name = field.array_name = "VALUE";
    field.source = "VTK";
    field.components = {"X"};
    frame.fields.push_back(field);
    data->increments.push_back(frame);
    data->datasets.push_back({0,"Block_1","Blocks",1});
    return data;
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = fixture(false);
    std::string command_prefix() const override
    {
      return "test";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      auto resolved = std::get<SimResults::JobSourceReference>(request);
      resolved.run_identity = data->run_identity;
      output = {resolved,"Mesh",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& output,std::string& error) override
    {
      return open(request,output,error);
    }
  };
  class Registry : public ResultSourceRegistryInterface
  {
  public:
    mutable Provider provider;
    bool resolve_result_provider(const std::string&,ResolvedResultSourceProvider& result,std::string&) const override
    {
      result.application_id = "test";
      result.provider = &provider;
      return true;
    }
  };
  template<class Controls> void slider_cases(Postprocessor::FilterKind kind,const std::string& label)
  {
    Registry registry;
    auto app = std::make_shared<PostprocessorApplicationInterface>(registry);
    std::string error;
    check(app->open_job("test",1,std::nullopt,error),"Cannot open slider fixture");
    Postprocessor::FilterDefinition filter;
    filter.name = "Slider";
    filter.kind = kind;
    filter.plane.origin = {.5,0,0};
    filter.reference_visible = true;
    check(app->create_filter(filter,"Mesh",error),"Cannot create slider fixture filter");
    const auto id = app->session().filters.begin()->first;
    if (kind == Postprocessor::FilterKind::Clip)
    {
      auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
      PostprocessorScene scene(renderer,app->session().active_view_id);
      check(scene.synchronize(*app,error),"Cannot build removed-reference scene");
      auto* actors = renderer->GetActors();
      actors->InitTraversal();
      bool referenceFound = false;
      while (auto* actor = actors->GetNextActor())
      {
        if (std::abs(actor->GetProperty()->GetOpacity()-Postprocessor::FilterDefaults::removed_opacity) < 1e-12)
        {
          referenceFound = true;
          check(!actor->GetPickable() && !actor->GetUseBounds(),"Removed-side reference intercepts picks or affects fitting");
        }
      }
      check(referenceFound,"Removed-side reference actor was not rendered");
    }
    int journals = 0,previews = 0;
    const auto dispatch = [&](const QString& command,bool journal)
    {
      if (journal)
      {
        ++journals;
      }
      else
      {
        ++previews;
      }
      std::istringstream stream(command.toStdString());
      std::string post,verb,operation,name,origin,normal;
      Postprocessor::PlaneDefinition plane;
      stream >> post >> verb >> operation >> std::quoted(name) >> origin >> plane.origin[0] >> plane.origin[1] >> plane.origin[2] >>
        normal >> plane.normal[0] >> plane.normal[1] >> plane.normal[2];
      check(stream && operation == "plane" && origin == "origin" && normal == "normal",
        "Slider emitted an invalid plane command");
      check(app->change_filter(name,"plane","",plane,"",error),"Slider plane command failed");
    };
    Controls controls(app,[&](const QString& command)
    {
      dispatch(command,true);
    },nullptr,[&](const QString& command)
    {
      dispatch(command,false);
    });
    controls.refresh();
    auto* slider = controls.template findChild<QSlider*>(QString::fromStdString("Postprocessor."+label+".Position"));
    auto* timer = controls.template findChild<QTimer*>(QString::fromStdString("Postprocessor."+label+".MotionPreviewTimer"));
    check(slider && timer && slider->isEnabled(),"Plane position slider is unavailable");
    if (!slider || !timer)
    {
      return;
    }
    check(journals == 0 && previews == 0,"Refreshing slider emitted commands");
    slider->setSliderDown(true);
    slider->setValue(750);
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
    check(previews == 1 && journals == 0,"Plane drag preview was journaled");
    slider->setSliderDown(false);
    check(journals == 1 && std::abs(app->session().filters.at(id).plane.origin[0]-.75) < 1e-12,
      "Plane release did not journal once or move along the normal");
    slider->setValue(250);
    check(journals == 2 && std::abs(app->session().filters.at(id).plane.origin[0]-.25) < 1e-12,
      "Keyboard plane movement was not journaled");
    slider->setSliderDown(true);
    slider->setValue(500);
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
    QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
    QApplication::sendEvent(slider,&escape);
    slider->setSliderDown(false);
    check(journals == 2 && std::abs(app->session().filters.at(id).plane.origin[0]-.25) < 1e-12,
      "Escape did not silently restore the plane");
  }
}
int main(int argc,char** argv)
{
  QApplication application(argc,argv);
  using namespace Postprocessor;
  std::string error;
  PlaneDefinition plane;
  plane.normal = {1e300,0,0};
  check(normalize_plane(plane,error) && plane.normal[0] == 1,"Normal normalization overflowed");
  plane.normal = {0,0,0};
  check(!normalize_plane(plane,error),"Zero plane normal was accepted");
  for (bool quadratic : {false,true})
  {
    const auto source = fixture(quadratic);
    FilterDefinition filter;
    filter.id = "test";
    filter.name = "Cutaway";
    filter.input = {OutputOwner::Source,"s",0};
    filter.plane.origin = {.5,0,0};
    Representation display;
    display.id = "r";
    EvaluatedOutput clipped;
    check(evaluate_filter(filter,display,source,clipped,error),"Cannot clip linear/quadratic hexahedron");
    if (!clipped.parts.empty())
    {
      const auto& part = clipped.parts.front();
      check(part.removed && part.removed->GetNumberOfCells() > 0,"Clip lost removed-side reference geometry");
      if (part.removed && part.removed->GetNumberOfCells() > 0)
      {
        double bounds[6];
        part.removed->GetBounds(bounds);
        check(bounds[0] >= -1e-8 && bounds[1] <= .5+1e-8,"Removed-side reference contains retained-side geometry");
      }
      check(part.mesh->GetNumberOfCells() > 0 && part.nodes->GetNumberOfPoints() > 0,"Clip lost retained geometry/nodes");
      check(!part.mesh->GetPointData()->HasArray("OriginalNodeId"),"Generated points retained interpolated solver node IDs");
      auto* elements = part.mesh->GetCellData()->GetArray("OriginalElementId");
      check(elements && elements->GetVariantValue(0).ToLongLong() == 9007199254741093LL,"Clip lost exact 64-bit element ID");
      auto* nodes = part.nodes->GetPointData()->GetArray("OriginalNodeId");
      check(nodes && nodes->GetVariantValue(0).ToLongLong() > 9007199254740992LL,"Retained node marker lost exact solver ID");
    }
    auto outside = filter;
    outside.plane.origin = {-1,0,0};
    EvaluatedOutput whole;
    check(evaluate_filter(outside,display,source,whole,error) && !whole.parts.empty() &&
      whole.parts.front().mesh->GetNumberOfCells() > 0 && whole.parts.front().removed &&
      whole.parts.front().removed->GetNumberOfCells() == 0,"Clip dropped wholly retained elements or invented removed geometry");
    filter.enabled = false;
    EvaluatedOutput bypass;
    check(evaluate_filter(filter,display,source,bypass,error) && !bypass.parts.empty() &&
      bypass.parts.front().mesh->GetNumberOfCells() == 1 && !bypass.parts.front().removed &&
      bypass.parts.front().nodes->GetNumberOfPoints() == vtkDataSet::SafeDownCast(source->increments.front().dataset.get())->GetNumberOfPoints(),
      "Disabled clip did not preserve the full input mesh and nodes");
    filter.enabled = true;
    filter.kind = FilterKind::Section;
    EvaluatedOutput section;
    check(evaluate_filter(filter,display,source,section,error),"Cannot section linear/quadratic hexahedron");
    if (!section.parts.empty())
    {
      auto* mesh = section.parts.front().mesh.get();
      check(mesh->GetNumberOfCells() > 0,"Section unexpectedly empty");
      for (vtkIdType i = 0; i < mesh->GetNumberOfPoints(); ++i)
      {
        double point[3];
        mesh->GetPoint(i,point);
        check(std::abs(point[0]-.5) < 1e-8,"Section generated a point off the plane");
      }
    }
    filter.plane.origin = {2,0,0};
    check(evaluate_filter(filter,display,source,section,error) && !section.parts.empty() && section.parts.front().mesh->GetNumberOfCells() == 0,
      "Empty section did not succeed as an empty output");
    auto* original = vtkDataSet::SafeDownCast(source->increments.front().dataset.get());
    check(original->GetNumberOfCells() == 1 && original->GetPointData()->HasArray("OriginalNodeId"),"Filter modified source mesh/IDs");
  }
  Provider provider;
  PostprocessorCore core;
  check(core.open_result(provider,SimResults::JobSourceReference{"test","",1,""},std::nullopt,error),"Cannot open filter source fixture");
  const auto source = core.session().active_source_id;
  FilterDefinition requested;
  requested.name = "Cutaway";
  requested.input = {OutputOwner::Source,source,0};
  requested.plane.origin = {.5,0,0};
  check(core.create_filter(requested,"view1",error),"Cannot create named clip");
  if (failures != 0 || core.session().representations.count(core.session().active_representation_id) == 0)
  {
    if (failures == 0) {
      return 1;
    } else {
      return failures;
    }
  }
  const auto display = core.session().representations.at(core.session().active_representation_id);
  std::shared_ptr<const EvaluatedOutput> first, second;
  check(core.evaluate_output(display,first,error) && core.evaluate_output(display,second,error) && first == second,"Filter cache missed unchanged context");
  PostprocessorProjectState saved, decoded;
  check(core.capture_project(saved,error) && project_from_json(project_to_json(saved,"."),".",decoded,error) &&
      decoded.session.filters.size() == 1,"Project did not preserve filter definitions/feature representations");
  std::array<double,2> motionRange;
  check(core.filter_motion_range("Cutaway","view1",motionRange,error) && std::abs(motionRange[0]+.5) < 1e-12 &&
    std::abs(motionRange[1]-.5) < 1e-12,"Plane slider range did not use complete source bounds");
  auto oblique = requested.plane;
  oblique.origin = {.5,.5,0};
  oblique.normal = {1,1,0};
  check(core.change_filter("Cutaway","plane","view1",oblique,"",error) &&
    core.filter_motion_range("Cutaway","view1",motionRange,error) &&
    std::abs(motionRange[0]+std::sqrt(.5)) < 1e-12 && std::abs(motionRange[1]-std::sqrt(.5)) < 1e-12,
    "Oblique slider range did not project all bounding-box corners");
  check(core.change_filter("Cutaway","plane","view1",requested.plane,"",error),"Cannot restore slider test plane");
  check(core.change_filter("Cutaway","disable","view1",{},"",error),"Cannot disable clip");
  std::shared_ptr<const EvaluatedOutput> bypass;
  check(core.evaluate_output(core.session().representations.at(display.id),bypass,error) && bypass &&
    bypass->parts.front().mesh->GetNumberOfCells() == 1 && !bypass->parts.front().removed,
    "Disabling clip did not bypass it");
  check(core.change_filter("Cutaway","reference","view1",{},"off",error),"Cannot turn off removed-side reference");
  PostprocessorProjectState flagSaved,flagDecoded;
  check(core.capture_project(flagSaved,error) && project_from_json(project_to_json(flagSaved,"."),".",flagDecoded,error) &&
    !flagDecoded.session.filters.begin()->second.enabled && !flagDecoded.session.filters.begin()->second.reference_visible,
    "Filter enabled/reference flags did not persist");
  check(core.change_filter("Cutaway","enable","view1",{},"",error),"Cannot re-enable clip");
  auto legacyFlags = filters_to_json(core.session().filters);
  auto legacyObject = legacyFlags[0].toObject();
  legacyObject.remove("enabled");
  legacyObject.remove("reference_visible");
  legacyFlags[0] = legacyObject;
  std::map<std::string,FilterDefinition> legacyDecoded;
  check(filters_from_json(legacyFlags,legacyDecoded,error) && legacyDecoded.begin()->second.enabled &&
    !legacyDecoded.begin()->second.reference_visible,"Legacy filter defaults were not restored");
  PostprocessorResultsTree tree([](const QString&)
  {
  },[](const QString&,bool)
  {
  });
  tree.refresh(core.session(),[&](const std::string& id)
  {
    return core.result_data(id);
  },[&](const std::string& id,std::size_t frame)
  {
    return core.frame_has_mesh(id,frame);
  });
  const auto* root = tree.topLevelItem(0);
  check(root && root->childCount() > 0 && root->child(root->childCount()-1)->text(0) == "Filters" &&
    root->child(root->childCount()-1)->childCount() == 1 &&
    root->child(root->childCount()-1)->child(0)->checkState(0) == Qt::Checked,
    "Source Filters group is missing, not last, or lacks output-visibility checkbox");
  const auto revision = core.display_revision();
  plane.normal = {0,0,0};
  check(!core.change_filter("Cutaway","plane","view1",plane,"",error) && core.display_revision() == revision,"Invalid plane mutated committed session");
  check(core.change_filter("Cutaway","hide","view1",{},"",error) && !core.session().representations.at(display.id).visible,"Filter visibility not view-local");
  check(core.change_filter("Cutaway","show","view1",{},"",error),"Cannot restore filter visibility");
  FilterDefinition sectionDefinition = requested;
  sectionDefinition.name = "Section only";
  sectionDefinition.kind = FilterKind::Section;
  check(core.create_filter(sectionDefinition,"view1",error),"Cannot create exclusive section output");
  const auto sectionDisplay = core.session().active_representation_id;
  check(core.session().representations.at(sectionDisplay).visible && !core.session().representations.at(display.id).visible,
    "Creating a section left a clip visible over the cut");
  for (const auto& item : core.session().representations)
  {
    if (item.second.view_id == "view1" && item.second.input.owner == OutputOwner::Source && item.second.input.object_id == source)
    {
      check(!item.second.visible,"Creating a section left its full source visible");
    }
  }
  std::shared_ptr<const EvaluatedOutput> sectionOutput;
  check(core.evaluate_output(core.session().representations.at(sectionDisplay),sectionOutput,error) && sectionOutput &&
    !sectionOutput->parts.empty(),"Section output is unavailable");
  if (sectionOutput && !sectionOutput->parts.empty())
  {
    double bounds[6];
    sectionOutput->parts.front().mesh->GetBounds(bounds);
    check(std::abs(bounds[0]-.5) < 1e-12 && std::abs(bounds[1]-.5) < 1e-12,
      "Section output contains volume geometry outside the cut plane");
  }
  check(core.set_category_visible(source,"view1",DatasetCategory::Blocks,false,error),"Cannot hide filtered blocks");
  check(core.session().representations.at(sectionDisplay).hidden_datasets.count(0) == 1,
    "Block visibility did not target the active section");
  tree.refresh(core.session(),[&](const std::string& id)
  {
    return core.result_data(id);
  },[&](const std::string& id,std::size_t frame)
  {
    return core.frame_has_mesh(id,frame);
  });
  bool foundBlocks = false;
  auto* currentRoot = tree.topLevelItem(0);
  for (int i = 0; currentRoot && i < currentRoot->childCount(); ++i)
  {
    auto* item = currentRoot->child(i);
    if (item->text(0).startsWith("Blocks"))
    {
      foundBlocks = true;
      check(item->checkState(0) == Qt::Unchecked,"Tree read block checks from the inactive source instead of the active section");
    }
  }
  check(foundBlocks,"Cannot find filtered block category in tree");
  check(core.change_filter("Cutaway","show","view1",{},"",error) &&
    core.session().representations.at(display.id).visible && !core.session().representations.at(sectionDisplay).visible,
    "Showing a clip did not hide the other output of the same source");
  check(core.change_filter("Cutaway","disable","view1",{},"",error),"Cannot prepare visible bypass test");
  tree.refresh(core.session(),[&](const std::string& id)
  {
    return core.result_data(id);
  },[&](const std::string& id,std::size_t frame)
  {
    return core.frame_has_mesh(id,frame);
  });
  QString checkboxAction;
  tree.filter_action = [&](const QString& name,const QString& action,const QString&)
  {
    checkboxAction = action;
    check(core.change_filter(name.toStdString(),action.toStdString(),"view1",{},"",error),"Tree filter visibility command failed");
  };
  currentRoot = tree.topLevelItem(0);
  auto* filterGroup = currentRoot->child(currentRoot->childCount()-1);
  for (int i = 0; i < filterGroup->childCount(); ++i)
  {
    auto* item = filterGroup->child(i);
    if (item->text(0) == "Cutaway")
    {
      check(item->checkState(0) == Qt::Checked,"Visible bypass output incorrectly appeared unchecked");
      item->setCheckState(0,Qt::Unchecked);
    }
  }
  QApplication::processEvents();
  check(checkboxAction == "hide" && !core.session().representations.at(display.id).visible,
    "Unchecking a filter did not hide its output");
  const auto clipDefinition = core.session().filters.find(display.input.object_id);
  check(clipDefinition != core.session().filters.end() && !clipDefinition->second.enabled,
    "Tree visibility checkbox changed the bypass setting");
  const auto originalDisplay = core.session().active_representation_id;
  check(core.session().representations.at(originalDisplay).input.owner == OutputOwner::Source &&
    core.session().representations.at(originalDisplay).visible,
    "Unchecking the visible filter did not restore the original source");
  const auto originalHidden = core.session().representations.at(originalDisplay).hidden_datasets;
  check(core.change_filter("Cutaway","select","view1",{},"",error),"Cannot select a hidden filter");
  const auto refreshChecks = [&]()
  {
    tree.refresh(core.session(),[&](const std::string& id)
    {
      return core.result_data(id);
    },[&](const std::string& id,std::size_t frame)
    {
      return core.frame_has_mesh(id,frame);
    });
  };
  refreshChecks();
  check(tree.topLevelItem(0)->checkState(0) == Qt::Checked,
    "Selecting a hidden filter unchecked a visible source");
  check(core.set_category_visible(source,"view1",DatasetCategory::Blocks,false,error) &&
    core.session().representations.at(originalDisplay).hidden_datasets.count(0) == 1 &&
    core.session().representations.at(display.id).hidden_datasets.count(0) == 0,
    "Block visibility targeted a selected hidden filter instead of the visible source");
  refreshChecks();
  for (int i = 0; i < tree.topLevelItem(0)->childCount(); ++i)
  {
    const auto* item = tree.topLevelItem(0)->child(i);
    if (item->text(0).startsWith("Blocks"))
    {
      check(item->checkState(0) == Qt::Unchecked,"Block checks do not match visible source settings");
    }
  }
  check(core.set_source_visible(source,"view1",false,error),"Cannot hide the source master");
  refreshChecks();
  check(tree.topLevelItem(0)->checkState(0) == Qt::Unchecked,"Hidden source master remained checked");
  check(core.change_filter("Cutaway","hide","view1",{},"",error) &&
    !core.session().representations.at(originalDisplay).visible,
    "Hiding an already-hidden filter revived a hidden source");
  check(core.set_source_visible(source,"view1",true,error) && core.session().representations.at(display.id).visible &&
    !core.session().representations.at(originalDisplay).visible,
    "Source master did not restore the selected filter exclusively");
  check(core.change_filter("Section only","hide","view1",{},"",error) && core.session().representations.at(display.id).visible &&
    core.session().active_representation_id == display.id,
    "Hiding an inactive filter changed the current output");
  check(core.set_source_visible(source,"view1",false,error),"Cannot hide a source with a visible filter");
  for (const auto& item : core.session().representations)
  {
    if (item.second.view_id == "view1" && core.output_source(item.second.input) == source)
    {
      check(!item.second.visible,"Source master left a filter output visible");
    }
  }
  check(core.set_source_visible(source,"view1",true,error) &&
    core.change_filter("Cutaway","hide","view1",{},"",error) &&
    core.session().active_representation_id == originalDisplay &&
    core.session().representations.at(originalDisplay).visible &&
    core.session().representations.at(originalDisplay).hidden_datasets.count(0) == 1,
    "Filter-hide fallback lost source identity, master visibility or block choices");
  check(core.set_category_visible(source,"view1",DatasetCategory::Blocks,true,error) &&
    core.session().representations.at(originalDisplay).hidden_datasets == originalHidden,
    "Cannot restore original block selections after visibility changes");
  refreshChecks();
  check(tree.topLevelItem(0)->checkState(0) == Qt::Checked,"Filter-hide fallback unchecked the source master");
  check(core.close_source(source,error) && core.session().filters.empty(),"Source closure left dependent filters");
  check(core.reset(error) && core.session().filters.empty(),"Reset did not clear filters");
  std::map<std::string,FilterDefinition> legacy;
  check(filters_from_json(QJsonValue(QJsonValue::Undefined),legacy,error) && legacy.empty(),"Legacy project filter defaults failed");
  slider_cases<PostprocessorClipControls>(FilterKind::Clip,"Clip");
  slider_cases<PostprocessorSectionControls>(FilterKind::Section,"Section");
  return failures;
}
