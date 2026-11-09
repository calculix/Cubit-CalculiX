#include "Postprocessor/GUI/Navigation/PostprocessorDeformationControls.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorScene.hpp"
#include "Framework/Interfaces/ResultSourceProviderInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QLineEdit>
#include <QTimer>
#include <QLabel>
#include <QFocusEvent>
#include <QKeyEvent>
#include <QHideEvent>
#include <vtkActorCollection.h>
#include <vtkCellType.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkOpenGLRenderer.h>
#include <vtkPartitionedDataSet.h>
#include <vtkPartitionedDataSetCollection.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkSmartPointer.h>
#include <vtkUnstructuredGrid.h>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

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
  SimResults::Increment frame(double ux, bool invalid = false)
  {
    auto* mesh = vtkUnstructuredGrid::New();
    auto points = vtkSmartPointer<vtkPoints>::New();
    points->SetDataTypeToDouble();
    points->InsertNextPoint(0,0,0);
    points->InsertNextPoint(1,0,0);
    points->InsertNextPoint(0,1,0);
    points->InsertNextPoint(0,0,1);
    mesh->SetPoints(points);
    mesh->Allocate(1);
    vtkIdType ids[] = {0,1,2,3};
    mesh->InsertNextCell(VTK_TETRA,4,ids);
    auto vectors = vtkSmartPointer<vtkDoubleArray>::New();
    vectors->SetName("u");
    vectors->SetNumberOfComponents(4);
    vectors->SetNumberOfTuples(4);
    auto mask = vtkSmartPointer<vtkDoubleArray>::New();
    mask->SetName("valid_u");
    mask->SetNumberOfTuples(4);
    for (vtkIdType i = 0; i < 4; ++i)
    {
      const double tuple[] = {999,3,ux,2}; // Magnitude, Z, X, Y.
      vectors->SetTuple(i,tuple);
      mask->SetValue(i,1);
    }
    if (invalid)
    {
      mask->SetValue(1,0);
      vectors->SetComponent(2,2,std::numeric_limits<double>::quiet_NaN());
      vectors->SetComponent(3,2,std::numeric_limits<double>::max());
    }
    mesh->GetPointData()->AddArray(vectors);
    mesh->GetPointData()->AddArray(mask);
    SimResults::Field field;
    field.name = "DISP";
    field.array_name = "u";
    field.validity_array = "valid_u";
    field.components = {"Magnitude","Z","X","Y"};
    field.vector_components = std::array<std::size_t,3>{2,3,1};
    field.role = SimResults::FieldRole::Displacement;
    SimResults::Increment result;
    result.fields.push_back(field);
    result.dataset = std::shared_ptr<vtkDataObject>(mesh,[](vtkDataObject* object)
    {
      object->Delete();
    });
    return result;
  }
  class Provider : public ResultSourceProviderInterface
  {
  public:
    std::shared_ptr<SimResults::VtkResultsData> data = std::make_shared<SimResults::VtkResultsData>();
    Provider()
    {
      data->job_id = 1;
      data->run_identity = "deformation-test";
      data->increments = {frame(1),frame(2),frame(0)};
      data->increments[2].fields.clear();
    }
    std::string command_prefix() const override
    {
      return "fake";
    }
    bool open(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& result,std::string& error) override
    {
      auto reference = std::get<SimResults::JobSourceReference>(request);
      reference.run_identity = data->run_identity;
      result = {reference,"Mesh",data};
      error.clear();
      return true;
    }
    bool refresh(const SimResults::ResultSourceReference& request,SimResults::PublishedResultSource& result,std::string& error) override
    {
      return open(request,result,error);
    }
  };
  class Registry : public ResultSourceRegistryInterface
  {
  public:
    mutable Provider provider;
    bool resolve_result_provider(const std::string& prefix,ResolvedResultSourceProvider& result,std::string& error) const override
    {
      if (prefix != "fake")
      {
        error = "Unknown provider";
        return false;
      }
      result.application_id = "test";
      result.provider = &provider;
      return true;
    }
  };

  void automatic_checks()
  {
    using namespace Postprocessor;
    const auto nodal = frame(1);
    const auto integration = frame(4);
    auto collection = vtkSmartPointer<vtkPartitionedDataSetCollection>::New();
    auto first = vtkSmartPointer<vtkPartitionedDataSet>::New();
    first->SetPartition(0,nodal.dataset.get());
    collection->SetPartitionedDataSet(0,first);
    auto second = vtkSmartPointer<vtkPartitionedDataSet>::New();
    second->SetPartition(0,integration.dataset.get());
    collection->SetPartitionedDataSet(1,second);
    SimResults::Increment mixed;
    collection->Register(nullptr);
    mixed.dataset = std::shared_ptr<vtkDataObject>(collection.GetPointer(),[](vtkDataObject* value)
    {
      value->Delete();
    });
    mixed.fields = {nodal.fields[0],integration.fields[0]};
    mixed.fields[1].name = "IntegrationPointDisplacement";
    mixed.fields[1].dataset_id = 1;
    mixed.fields[1].association = SimResults::Association::IntegrationPoint;
    ResultDeformation settings;
    settings.enabled = true;
    std::string warning, error;
    check(settings.field_mode == DeformationFieldMode::Automatic && !settings.field,"Automatic is not the Core default");
    check(deformation_available(mixed,settings,error),"Mixed displacement did not resolve");
    for (std::size_t dataset = 0; dataset < 2; ++dataset)
    {
      auto* mesh = vtkDataSet::SafeDownCast(dataset == 0 ? nodal.dataset.get() : integration.dataset.get());
      std::shared_ptr<vtkDataSet> moved;
      check(deform_mesh(mesh,dataset,mixed,settings,moved,warning,error) && moved && warning.empty(),"Automatic mixed displacement failed");
      if (moved)
      {
        double p[3];
        moved->GetPoint(0,p);
        check(p[0] == (dataset == 0 ? 1 : 4) && p[1] == 2 && p[2] == 3,"Wrong per-dataset displacement used");
      }
    }
    mixed.fields[0].name = "SolverSpecificMotion";
    check(resolve_deformation_field(mixed,0,settings,warning) == &mixed.fields[0],"Automatic selection depends on DISP spelling");
    auto duplicate = mixed.fields[0];
    duplicate.name = "Other displacement";
    duplicate.array_name = "other_u";
    mixed.fields.push_back(duplicate);
    std::shared_ptr<vtkDataSet> moved;
    check(deform_mesh(vtkDataSet::SafeDownCast(nodal.dataset.get()),0,mixed,settings,moved,warning,error) &&
      !moved && warning.find("ambiguous") != std::string::npos,"Ambiguous automatic selection guessed a field");
    check(deformation_available(mixed,settings,error),"One ambiguous dataset prevented another unique dataset from deforming");
    mixed.fields.pop_back();
    mixed.fields[0].role = SimResults::FieldRole::Unspecified;
    mixed.fields[0].name = "DISP";
    check(!resolve_deformation_field(mixed,0,settings,warning) && !warning.empty(),"Untagged DISP name was treated as semantic metadata");
    auto next = mixed;
    next.fields.clear();
    check(deform_mesh(vtkDataSet::SafeDownCast(nodal.dataset.get()),0,next,settings,moved,warning,error) &&
      !moved && !warning.empty(),"Missing-frame automatic selection retained stale geometry");
    check(!deformation_available(next,settings,error),"Automatic enabled with no displacement datasets");
    settings.field_mode = DeformationFieldMode::Explicit;
    settings.field = FieldSelection{0,"u",SimResults::Association::Node,{},true,field_group_key(mixed.fields[0])};
    check(deform_mesh(vtkDataSet::SafeDownCast(nodal.dataset.get()),0,mixed,settings,moved,warning,error) &&
      moved && warning.empty(),"Explicit override could not use an untagged external vector");
  }
}

int main(int argc,char** argv)
{
  QApplication qt(argc,argv);
  using namespace Postprocessor;
  automatic_checks();
  std::string error, warning;
  auto invalid = frame(1,true);
  ResultDeformation definition;
  definition.field_mode = DeformationFieldMode::Explicit;
  definition.field = FieldSelection{{},"u",SimResults::Association::Node,{},true,field_group_key(invalid.fields[0])};
  definition.enabled = true;
  definition.scale = 2;
  std::shared_ptr<vtkDataSet> output;
  auto* input = vtkDataSet::SafeDownCast(invalid.dataset.get());
  check(deform_mesh(input,0,invalid,definition,output,warning,error),"Masked deformation failed");
  check(output && warning.find("3 invalid") != std::string::npos,"Invalid samples were not reported");
  if (output)
  {
    double p[3];
    output->GetPoint(0,p);
    check(p[0] == 2 && p[1] == 4 && p[2] == 6,"Declared vector columns were not used");
    for (vtkIdType i = 1; i < 4; ++i)
    {
      double original[3];
      input->GetPoint(i,original);
      output->GetPoint(i,p);
      check(p[0] == original[0] && p[1] == original[1] && p[2] == original[2],"Invalid sample moved");
    }
    input->GetPoint(0,p);
    check(p[0] == 0 && p[1] == 0 && p[2] == 0,"Published source coordinates changed");
    check(output->GetPointData()->GetArray("u") == input->GetPointData()->GetArray("u"),"Immutable result arrays were duplicated");
  }
  definition.field->dataset_id = 7;
  check(deform_mesh(input,0,invalid,definition,output,warning,error) && !output && warning.empty(),"Dataset scope was ignored");

  Registry registry;
  auto app = std::make_shared<PostprocessorApplicationInterface>(registry);
  check(app->open_job("fake",1,std::nullopt,error),"Cannot open deformation fixture");
  auto automatic = app->deformation("Mesh",error);
  check(automatic && automatic->field_mode == DeformationFieldMode::Automatic && !automatic->enabled,"New source did not default to disabled automatic deformation");
  if (automatic)
  {
    automatic->enabled = true;
    check(app->set_deformation("Mesh",*automatic,error),"Cannot enable automatic without explicit field");
    automatic->enabled = false;
    check(app->set_deformation("Mesh",*automatic,error),"Cannot disable automatic deformation");
  }
  std::vector<std::string> sent, previews;
  bool failNext = false;
  bool ignoreNext = false;
  PostprocessorDeformationControls* activePage = nullptr;
  auto dispatch = [&](bool preview, const std::string& command, std::string& issue)
  {
    (preview ? previews : sent).push_back(command);
    if (failNext)
    {
      failNext = false;
      issue = "Injected failure";
      return false;
    }
    if (ignoreNext)
    {
      ignoreNext = false;
      return true;
    }
    const auto suffix = command.rfind(" source ");
    if (suffix == std::string::npos)
    {
      issue = "Missing source qualifier";
      return false;
    }
    std::istringstream source(command.substr(suffix+8));
    std::string name;
    source >> std::quoted(name);
    std::istringstream stream(command.substr(0,suffix));
    std::string post, deform, operation;
    stream >> post >> deform >> operation;
    bool accepted = false;
    if (operation == "field")
    {
      std::string field;
      stream >> std::quoted(field);
      std::string qualifier;
      std::optional<std::size_t> dataset;
      if (stream >> qualifier)
      {
        std::size_t value;
        stream >> value;
        dataset = value;
      }
      accepted = field == "auto" ? app->deform_auto(name,issue) : app->deform_field(name,field,dataset,issue);
    }
    else
    {
      auto settings = app->deformation(name,issue);
      if (!settings)
      {
        return false;
      }
      if (operation == "scale")
      {
        stream >> settings->scale;
      }
      else if (operation == "overlay")
      {
        std::string value;
        stream >> value;
        settings->overlay = value == "on";
      }
      else
      {
        settings->enabled = operation == "on";
      }
      accepted = app->set_deformation(name,*settings,issue);
    }
    if (activePage)
    {
      activePage->refresh(); // Reentrant observers must not reset a scale edit.
    }
    return accepted;
  };
  PostprocessorDeformationControls page(app,[&](const std::string& command,std::string& issue)
  {
    return dispatch(false,command,issue);
  },[&](const std::string& command,std::string& issue)
  {
    return dispatch(true,command,issue);
  },nullptr);
  activePage = &page;
  auto* enabled = page.findChild<QCheckBox*>("Postprocessor.Deformation.Enabled");
  auto* fields = page.findChild<QComboBox*>("Postprocessor.Deformation.Field");
  auto* scale = page.findChild<QLineEdit*>("Postprocessor.Deformation.Scale");
  auto* overlay = page.findChild<QCheckBox*>("Postprocessor.Deformation.Overlay");
  auto* timer = page.findChild<QTimer*>("Postprocessor.Deformation.ScalePreviewTimer");
  auto* issue = page.findChild<QLabel*>("Postprocessor.Deformation.Error");
  check(enabled && fields && scale && overlay && timer && issue,"Missing deformation controls");
  if (!enabled || !fields || !scale || !overlay || !timer || !issue)
  {
    return 1;
  }
  check(!page.findChild<QWidget*>("Postprocessor.Deformation.Apply"),"Apply button still exists");
  auto edit = [&](const QString& text)
  {
    scale->setText(text);
    QMetaObject::invokeMethod(scale,"textEdited",Qt::DirectConnection,Q_ARG(QString,text));
  };
  auto preview = [&]()
  {
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
  };
  auto finish = [&]()
  {
    QMetaObject::invokeMethod(scale,"editingFinished",Qt::DirectConnection);
  };
  auto choose = [&](int index)
  {
    fields->setCurrentIndex(index);
    QMetaObject::invokeMethod(fields,"activated",Qt::DirectConnection,Q_ARG(int,index));
  };
  page.refresh();
  check(sent.empty() && previews.empty(),"Refresh submitted commands");
  check(fields->currentIndex() == 0 && fields->currentText().contains("Automatic"),"Toolbar did not default to Automatic displacement");
  fields->setCurrentIndex(1);
  check(sent.empty(),"Programmatic field refresh submitted a command");
  choose(1);
  auto settings = app->deformation("Mesh",error);
  check(settings && settings->field_mode == DeformationFieldMode::Explicit && sent.size() == 1,"Field selection was not immediate");
  enabled->setChecked(true);
  check(app->deformation("Mesh",error)->enabled && sent.size() == 2,"Enable checkbox was not immediate");
  const auto beforeScale = sent.size();
  edit("2e0");
  check(timer->isActive() && previews.empty() && sent.size() == beforeScale,"Scale edit bypassed debounce");
  preview();
  check(previews.size() == 1 && app->deformation("Mesh",error)->scale == 2 && sent.size() == beforeScale,"Scale preview was journaled or did not update");
  page.refresh();
  check(scale->text() == "2e0","Preview refresh replaced active input formatting");
  finish();
  check(sent.size() == beforeScale+1 && sent.back().find("post deform scale 2 ") == 0,"Scale completion was not journaled once");
  finish();
  check(sent.size() == beforeScale+1,"Duplicate editingFinished journaled twice");
  overlay->setChecked(true);
  check(app->deformation("Mesh",error)->overlay,"Overlay checkbox was not immediate");
  settings = app->deformation("Mesh",error);
  const auto explicitField = settings->field;
  choose(0);
  settings = app->deformation("Mesh",error);
  check(settings && settings->field_mode == DeformationFieldMode::Automatic && settings->enabled &&
    same_field(settings->field,explicitField),"Automatic switch lost explicit selection or enabled state");
  check(sent.back().find("post deform field auto") == 0,"Automatic selection was not journaled");
  const auto view = app->session().active_view_id;
  std::array<double,6> bounds;
  check(app->visible_bounds(view,bounds,error) && bounds == std::array<double,6>{0,3,0,5,0,7},"Fit omitted deformation or overlay");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  PostprocessorScene scene(renderer,view);
  check(scene.synchronize(*app,error) && renderer->GetActors()->GetNumberOfItems() == 3,"Deformed surface/edges/overlay missing");
  check(app->color_field("Mesh","DISP",{},true,std::nullopt,error),"Cannot independently color deformed shape");
  check(scene.synchronize(*app,error),"Coloring rejected transformed mesh");
  const auto beforeIncomplete = sent.size();
  edit("3");
  preview();
  edit("1e-");
  check(!timer->isActive() && app->deformation("Mesh",error)->scale == 3,"Incomplete exponent replaced last valid display");
  QFocusEvent focusOut(QEvent::FocusOut);
  QApplication::sendEvent(scale,&focusOut);
  check(app->deformation("Mesh",error)->scale == 3 && sent.size() == beforeIncomplete+1,"Focus loss did not finish the last valid scale once");
  edit("2");
  finish();
  edit("3");
  check(app->set_frame("Mesh",2,error),"Cannot change deformed frame");
  page.refresh();
  check(scale->text() == "3","Frame change discarded scale input");
  preview();
  check(scene.synchronize(*app,error),"Cannot update deformed frame");
  check(app->visible_bounds(view,bounds,error) && bounds[1] == 7,"Fit retained old frame displacement or scale");
  finish();
  check(app->set_frame("Mesh",3,error) && scene.synchronize(*app,error),"Missing-field fallback failed");
  check(!scene.warning().empty(),"Missing vector warning absent");
  check(app->visible_bounds(view,bounds,error) && bounds == std::array<double,6>{0,1,0,1,0,1},"Missing vector retained stale coordinates");
  page.refresh();
  enabled->setChecked(false);
  enabled->setChecked(true);
  check(!enabled->isChecked() && !issue->text().isEmpty(),"Rejected enabling was not restored/reported");
  check(app->set_frame("Mesh",1,error),"Cannot restore first frame");
  page.refresh();
  const auto beforeInvalid = sent.size();
  edit("-1");
  preview();
  finish();
  check(sent.size() == beforeInvalid && app->deformation("Mesh",error)->scale == 3,"Invalid input changed or journaled scale");
  edit("4");
  settings = app->deformation("Mesh",error);
  settings->scale = 8;
  check(app->set_deformation("Mesh",*settings,error),"Cannot inject external scale change");
  page.refresh();
  preview();
  finish();
  check(scale->text() == "8" && app->deformation("Mesh",error)->scale == 8,"Pending edit overwrote external settings");
  ignoreNext = true;
  enabled->setChecked(true);
  check(!enabled->isChecked() && !issue->text().isEmpty(),"Accepted-without-mutation went undetected");
  failNext = true;
  edit("4");
  preview();
  check(scale->text() == "8" && !issue->text().isEmpty(),"Failed preview did not reload committed scale");
  const auto beforeFailedCommit = sent.size();
  edit("9");
  preview();
  failNext = true;
  finish();
  check(app->deformation("Mesh",error)->scale == 8 && sent.size() == beforeFailedCommit+1 &&
    !issue->text().isEmpty(),"Rejected final scale did not restore the owned preview");
  const auto beforeEnter = sent.size();
  edit("9");
  preview();
  edit("1e-");
  QKeyEvent enter(QEvent::KeyPress,Qt::Key_Return,Qt::NoModifier);
  QApplication::sendEvent(scale,&enter);
  check(app->deformation("Mesh",error)->scale == 9 && sent.size() == beforeEnter+1,"Enter did not finish incomplete input with the last valid scale");
  edit("8");
  finish();
  const auto beforeHide = sent.size();
  edit("9");
  preview();
  QHideEvent hidden;
  QApplication::sendEvent(&page,&hidden);
  check(app->deformation("Mesh",error)->scale == 8 && sent.size() == beforeHide && !timer->isActive(),"Hide did not cancel owned preview silently");
  edit("4");
  check(app->reset(error),"Cannot reset deformation fixture");
  page.refresh();
  const auto beforeReset = sent.size();
  const auto previewsBeforeReset = previews.size();
  preview();
  finish();
  check(sent.size() == beforeReset && previews.size() == previewsBeforeReset,"Reset allowed stale commands");
  check(!page.isEnabled(),"Reset did not disable deformation toolbar");
  return failures == 0 ? 0 : 1;
}
