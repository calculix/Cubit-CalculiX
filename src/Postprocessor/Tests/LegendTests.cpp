#include "LegendTests.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/GUI/Viewport/PostprocessorLegendInteraction.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWidget>
#include <vtkGenericOpenGLRenderWindow.h>
#include <vtkOpenGLRenderer.h>
#include <cmath>
#include <iostream>

int legend_cases(const std::shared_ptr<PostprocessorApplicationInterface>& app)
{
  int failures = 0;
  const auto check = [&](bool condition,const char* message)
  {
    if (!condition)
    {
      ++failures;
      std::cerr << message << '\n';
    }
  };
  std::string error;
  const auto saved = app->coloring("Model",error);
  if (!saved)
  {
    check(false,"Legend fixture has no coloring");
    return failures;
  }
  auto settings = *saved;
  settings.legend_visible = true;
  settings.legend_position = std::array<double,2>{0.7,0.06};
  check(app->set_coloring("Model",settings,error),"Cannot prepare legend interaction fixture");
  auto renderer = vtkSmartPointer<vtkOpenGLRenderer>::New();
  auto window = vtkSmartPointer<vtkGenericOpenGLRenderWindow>::New();
  window->SetSize(800,600);
  window->AddRenderer(renderer);
  const auto view = app->session().active_view_id;
  PostprocessorScene scene(renderer,view);
  check(scene.synchronize(*app,error),"Cannot prepare legend scene");
  const auto hit = scene.legend_at(0.72,0.1);
  check(hit && hit->source_id == app->session().active_source_id,"Legend hit testing lost its source");
  if (!hit)
  {
    app->set_coloring("Model",*saved,error);
    return failures;
  }
  check(hit->frame[2]*800 <= 141 && hit->frame[3]*600 <= 221 && hit->frame[1] < 0.1,
    "Legend did not use compact bottom layout");
  QWidget surface;
  surface.resize(800,600);
  int journals = 0,previews = 0;
  QString properties;
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
    if (command.startsWith("post properties"))
    {
      properties = command;
      return true;
    }
    check(command.endsWith("source \"Model\""),"Legend command did not name its source");
    if (command.startsWith("post vector legend"))
    {
      auto vectors = app->vector_plot("Model",error);
      if (!vectors)
      {
        return false;
      }
      if (command.section(' ',3,3) == "auto")
      {
        vectors->legend.legend_position.reset();
      }
      else
      {
        bool a = false,b = false;
        const double x = command.section(' ',4,4).toDouble(&a),y = command.section(' ',5,5).toDouble(&b);
        check(a && b,"Vector drag lost numeric coordinates");
        vectors->legend.legend_position = std::array<double,2>{x,y};
      }
      return app->set_vector_plot("Model",*vectors,error);
    }
    auto color = app->coloring("Model",error);
    if (!color)
    {
      return false;
    }
    if (command.section(' ',3,3) == "automatic")
    {
      color->legend_position.reset();
    }
    else
    {
      bool xValid = false,yValid = false;
      const double x = command.section(' ',3,3).toDouble(&xValid);
      const double y = command.section(' ',4,4).toDouble(&yValid);
      check(xValid && yValid,"Legend command lost numeric coordinates");
      color->legend_position = std::array<double,2>{x,y};
    }
    return app->set_coloring("Model",*color,error);
  };
  PostprocessorLegendInteraction interaction(&surface,&scene,view,app,
    [&](const QString& command,std::string&)
    {
      return dispatch(command,true);
    },[&](const QString& command,std::string&)
    {
      return dispatch(command,false);
    },[]()
    {
    },[&](const std::string&)
    {
      check(false,"Legend interaction reported an error");
    });
  const QPointF start(0.72*800,0.9*600);
  const QPointF end = start+QPointF(-80,-60);
  QMouseEvent press(QEvent::MouseButtonPress,start,start,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
  QMouseEvent move(QEvent::MouseMove,end,end,Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
  QMouseEvent release(QEvent::MouseButtonRelease,end,end,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
  check(interaction.handle(&press) && interaction.handle(&move),"Legend did not consume drag before camera picking");
  interaction.flush_preview();
  check(previews == 1 && journals == 0,"Legend preview was journaled");
  check(interaction.handle(&release) && journals == 1,"Legend release did not journal exactly once");
  const auto moved = app->coloring("Model",error);
  check(moved && moved->legend_position && std::abs((*moved->legend_position)[0]-0.6) < 1e-12 &&
    std::abs((*moved->legend_position)[1]-0.16) < 1e-12,"Legend drag did not use bottom-left normalized coordinates");
  check(scene.synchronize(*app,error),"Cannot refresh moved legend");
  const QPointF second(0.62*800,0.8*600);
  QMouseEvent doubleClick(QEvent::MouseButtonDblClick,second,second,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
  check(interaction.handle(&doubleClick) && properties == "post properties legend source \"Model\"",
    "Legend double-click did not journal source properties");
  QMouseEvent secondPress(QEvent::MouseButtonPress,second,second,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
  QMouseEvent secondMove(QEvent::MouseMove,second+QPointF(-40,0),second+QPointF(-40,0),Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
  check(interaction.handle(&secondPress) && interaction.handle(&secondMove),"Cannot start cancellable legend drag");
  interaction.flush_preview();
  QKeyEvent escape(QEvent::KeyPress,Qt::Key_Escape,Qt::NoModifier);
  const bool escaped = interaction.handle(&escape);
  const auto restored = app->coloring("Model",error);
  check(escaped && restored && moved && restored->legend_position == moved->legend_position && journals == 2,
    "Escape did not restore the legend silently");
  const auto savedVectors = app->vector_plot("Model",error);
  check(app->vector_field("Model","DISP",std::nullopt,error),"Cannot choose vector legend fixture field");
  auto vectors = app->vector_plot("Model",error);
  if (vectors)
  {
    vectors->enabled = true;
    vectors->legend.legend_visible = true;
    vectors->legend.legend_position = std::array<double,2>{.1,.06};
    check(app->set_vector_plot("Model",*vectors,error),"Cannot show vector legend fixture");
    check(scene.synchronize(*app,error),"Cannot synchronize vector legend fixture");
    const auto vectorHit = scene.legend_at(.12,.1);
    check(vectorHit && vectorHit->vector && vectorHit->frame[2]*800 <= 141 && vectorHit->frame[3]*600 <= 221,
      "Vector legend is not an independent compact sibling");
    const QPointF vectorStart(.12*800,.9*600),vectorEnd = vectorStart+QPointF(80,-60);
    QMouseEvent vectorPress(QEvent::MouseButtonPress,vectorStart,vectorStart,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent vectorMove(QEvent::MouseMove,vectorEnd,vectorEnd,Qt::NoButton,Qt::LeftButton,Qt::NoModifier);
    QMouseEvent vectorRelease(QEvent::MouseButtonRelease,vectorEnd,vectorEnd,Qt::LeftButton,Qt::NoButton,Qt::NoModifier);
    const auto beforeJournals = journals,beforePreviews = previews;
    check(interaction.handle(&vectorPress) && interaction.handle(&vectorMove),"Cannot drag vector legend");
    interaction.flush_preview();
    check(interaction.handle(&vectorRelease) && journals == beforeJournals+1 && previews == beforePreviews+1,
      "Vector drag did not use silent preview and one journal entry");
    const auto resultUnchanged = app->coloring("Model",error);
    check(resultUnchanged && moved && resultUnchanged->legend_position == moved->legend_position,
      "Vector drag moved the result legend");
    check(scene.synchronize(*app,error),"Cannot refresh dragged vector legend");
    const QPointF vectorClick(.22*800,.8*600);
    QMouseEvent vectorDoubleClick(QEvent::MouseButtonDblClick,vectorClick,vectorClick,Qt::LeftButton,Qt::LeftButton,Qt::NoModifier);
    check(interaction.handle(&vectorDoubleClick) && properties == "post properties vectorlegend source \"Model\"",
      "Vector double-click did not open vector legend properties");
    auto hideResult = app->coloring("Model",error);
    if (hideResult)
    {
      hideResult->legend_visible = false;
      check(app->set_coloring("Model",*hideResult,error) && scene.synchronize(*app,error) && scene.legend_at(.22,.2).has_value(),
        "Hiding the result legend also hid the vector legend");
    }
  }
  if (savedVectors)
  {
    check(app->set_vector_plot("Model",*savedVectors,error),"Cannot restore vector legend fixture");
  }
  check(app->set_coloring("Model",*saved,error),"Cannot restore legend fixture");
  return failures;
}
