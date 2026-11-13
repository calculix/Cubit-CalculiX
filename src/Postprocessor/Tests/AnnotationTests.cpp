#include "AnnotationTests.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Core/annotations/AnnotationCodec.hpp"
#include "Postprocessor/GUI/Annotations/PostprocessorAnnotationDisplay.hpp"
#include "Postprocessor/GUI/Annotations/PostprocessorAnnotationInteraction.hpp"
#include "Postprocessor/GUI/Annotations/PostprocessorAnnotationMenu.hpp"
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <vtkSmartPointer.h>
#include <QImage>
#include <QJsonArray>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QWidget>
#include <cmath>
#include <iomanip>
#include <iostream>
#include <limits>
#include <sstream>

int annotation_cases(const std::shared_ptr<PostprocessorApplicationInterface>& app)
{
  using namespace Postprocessor;
  using Selector = PostprocessorApplicationInterface::DatasetSelector;
  constexpr std::int64_t largeNode = 9007199254740993LL;
  int failures = 0;
  const auto check = [&](bool condition,const char* message)
  {
    if (!condition)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  };
  const auto view = app->session().active_view_id;
  std::string error, assigned;
  const auto add = [&](const std::string& name,EntityKind kind,std::int64_t id)
  {
    return app->add_label(view,name,"Model",Selector{std::size_t{0}},kind,id,std::nullopt,"","",false,assigned,error);
  };
  check(add("",EntityKind::Node,largeNode) && assigned == "label_1","Anonymous label did not generate label_1");
  check(add("",EntityKind::Node,44) && assigned == "label_2","Anonymous label name collided");
  check(!add("label_1",EntityKind::Node,largeNode),"Duplicate label name was accepted");
  check(!add("bad",EntityKind::Node,123456789),"Missing entity created a label");
  check(!add("masked",EntityKind::Node,43),"Masked entity created a result label");
  check(!add("@minimum",EntityKind::Node,largeNode),"Reserved extrema name was accepted");
  check(app->add_label(view,"ip","Model",Selector{std::size_t{1}},EntityKind::IntegrationPoint,120,3,
    "ip_stress","SXX",false,assigned,error),"Cannot create a label for an original integration point");
  check(app->session().views.at(view).annotations.labels.at("ip").entity.integration_point == 3 &&
    app->change_label(view,"ip","remove","",{},error),"IP label lost its integration-point number");
  check(app->change_label(view,"label_1","rename","tip",{},error),"Cannot rename label");
  check(!app->change_label(view,"tip","rename","label_2",{},error),"Conflicting rename was accepted");
  check(app->change_label(view,"tip","position","",{0.25,0.4},error),"Cannot position label");
  check(!app->change_label(view,"tip","position","",{-0.1,0.4},error) &&
    !app->change_label(view,"tip","position","",{std::numeric_limits<double>::quiet_NaN(),0.4},error),
    "Invalid label positions were accepted");
  const auto encoded = annotations_to_json(app->session().views.at(view).annotations);
  AnnotationState restored;
  check(annotations_from_json(encoded,restored) && restored.labels.at("tip").entity.entity_id == largeNode &&
    restored.labels.at("tip").position == std::array<double,2>{0.25,0.4},"Annotation persistence lost exact ID or position");
  auto malformed = encoded;
  auto records = malformed.value("labels").toArray();
  records.append(records[0]);
  malformed.insert("labels",records);
  check(!annotations_from_json(malformed,restored),"Duplicate persisted label names were accepted");
  check(annotations_from_json(QJsonValue(QJsonValue::Undefined),restored) && restored.labels.empty(),"Legacy project did not receive annotation defaults");
  check(app->color_field("Model","cell_stress","SXX",false,0,error),"Cannot change active field for label test");
  auto evaluated = app->annotations(view);
  bool fixedField = false;
  for (const auto& label : evaluated)
  {
    fixedField = fixedField || (label.name == "tip" && label.text.find(": 10") != std::string::npos);
  }
  check(fixedField,"Existing label followed active coloring instead of its fixed field");
  check(add("cell",EntityKind::Element,120),"Element label has no geometric anchor");
  evaluated = app->annotations(view);
  bool anchored = false;
  for (const auto& label : evaluated)
  {
    if (label.name == "cell")
    {
      anchored = label.anchor == std::array<double,3>{0.25,0.25,0.25};
    }
  }
  check(anchored,"Element label anchor is not the mean of its original nodes");
  check(app->change_label(view,"cell","remove","",{},error),"Cannot remove element label");
  check(app->color_field("Model","stress","SXX",false,0,error),"Cannot restore stress field");
  check(app->set_labels(view,true,true,error),"Cannot enable extrema");
  evaluated = app->annotations(view);
  bool minimum = false, maximum = false;
  for (const auto& label : evaluated)
  {
    minimum = minimum || (label.name == "@minimum" && label.text.find("Minimum: 10") != std::string::npos);
    maximum = maximum || (label.name == "@maximum" && label.text.find("Maximum: 40") != std::string::npos);
  }
  check(minimum && maximum,"Current-frame extrema did not use valid original samples");
  check(app->set_dataset_visibility("Model",DatasetVisibilityAction::Hide,Selector{std::size_t{0}},error),
    "Cannot hide extrema dataset");
  evaluated = app->annotations(view);
  bool hiddenExtrema = false;
  for (const auto& label : evaluated)
  {
    hiddenExtrema = hiddenExtrema || label.name == "@minimum" || label.name == "@maximum";
  }
  check(!hiddenExtrema,"Hidden datasets contributed extrema markers");
  check(app->set_dataset_visibility("Model",DatasetVisibilityAction::Show,Selector{std::size_t{0}},error),
    "Cannot restore extrema dataset");
  check(app->set_labels(view,false,false,error) && app->annotations(view).empty(),"Global label visibility did not hide extrema");
  check(app->set_labels(view,false,true,error) && app->set_labels(view,true,false,error),"Cannot restore label visibility");
  check(app->set_frame("Model",2,error),"Cannot change annotation frame");
  evaluated = app->annotations(view);
  bool unavailable = false;
  for (const auto& label : evaluated)
  {
    unavailable = unavailable || (label.name == "tip" && !label.anchor && label.text.find("Unavailable") != std::string::npos);
  }
  check(unavailable,"Missing frame entity retained a stale anchor/value");
  check(app->set_frame("Model",1,error),"Cannot restore annotation frame");
  check(app->change_label(view,"label_2","remove","",{},error),"Cannot remove second label");

  // Paint the Qt overlay without rendering OpenGL; projection uses VTK matrices.
  QWidget surface;
  surface.resize(800,600);
  auto window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  window->SetSize(800,600);
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  window->AddRenderer(renderer);
  PostprocessorAnnotationDisplay overlay(renderer,view,&surface);
  overlay.synchronize(*app);
  QImage image(surface.size(),QImage::Format_ARGB32_Premultiplied);
  image.fill(Qt::transparent);
  overlay.render(&image);
  QPointF hit(-1,-1);
  for (int y = 0; y < surface.height() && hit.x() < 0; ++y)
  {
    for (int x = 0; x < surface.width(); ++x)
    {
      if (overlay.label_at(QPointF(x,y)) == "tip")
      {
        hit = QPointF(x+2,y+2);
        break;
      }
    }
  }
  check(hit.x() >= 0,"Label text cannot be hit-tested");
  int journaled = 0, previews = 0;
  const auto dispatch = [&](const QString& command,std::string& issue,bool journal)
  {
    std::istringstream input(command.toStdString());
    std::string post, label, operation, name, viewKeyword, targetView;
    std::array<double,2> position{};
    input >> post >> label >> operation >> std::quoted(name) >> position[0] >> position[1] >> viewKeyword >> std::quoted(targetView);
    if (!input || operation != "position")
    {
      return false;
    }
    if (journal)
    {
      ++journaled;
    }
    else
    {
      ++previews;
    }
    return app->change_label(targetView,name,operation,"",position,issue);
  };
  PostprocessorAnnotationInteraction interaction(&surface,&overlay,view,app,
    [&](const QString& command,std::string& issue)
    {
      return dispatch(command,issue,true);
    },[&](const QString& command,std::string& issue)
    {
      return dispatch(command,issue,false);
    },[]()
    {
    });
  if (hit.x() >= 0)
  {
    QMouseEvent press(QEvent::MouseButtonPress,hit,hit,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent move(QEvent::MouseMove,hit+QPointF(80,-60),hit+QPointF(80,-60),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent release(QEvent::MouseButtonRelease,hit+QPointF(80,-60),hit+QPointF(80,-60),Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    check(interaction.handle(&press) && interaction.handle(&move),"Label drag was not consumed before camera handling");
    interaction.flush_preview();
    check(previews == 1 && journaled == 0,"Drag preview was journaled");
    check(interaction.handle(&release) && journaled == 1,"Release did not record exactly one final position command");
    const auto finalPosition = app->session().views.at(view).annotations.labels.at("tip").position;
    check(std::abs(finalPosition[0]-0.35) < 1e-12 && std::abs(finalPosition[1]-0.5) < 1e-12,"Drag coordinates changed direction or units");
    interaction.handle(&press);
    interaction.handle(&move);
    interaction.flush_preview();
    QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
    check(interaction.handle(&escape) && journaled == 1 &&
      app->session().views.at(view).annotations.labels.at("tip").position == finalPosition,
      "Esc did not silently restore the drag baseline");
    interaction.handle(&press);
    interaction.handle(&move);
    interaction.flush_preview();
    check(app->set_frame("Model",2,error),"Cannot change frame during annotation gesture");
    interaction.handle(&move);
    check(journaled == 1 && app->session().views.at(view).annotations.labels.at("tip").position == finalPosition,
      "Frame change did not cancel the preview without journaling or overwriting the new frame");
    check(app->set_frame("Model",1,error),"Cannot restore frame after gesture cancellation");
  }
  const EntityReference entity{app->session().active_source_id,0,EntityKind::Node,largeNode,std::nullopt};
  const auto command = annotation_add_command(*app,view,entity);
  check(command.contains("post label add \"label_1\"") && command.contains("9007199254740993") &&
    command.contains(" field ") && command.contains(" component ") && command.contains(" view "),
    "GUI label creation did not encode explicit replayable identity and field");
  check(app->change_label(view,"tip","remove","",{},error),"Cannot clear annotation fixture");
  return failures;
}
