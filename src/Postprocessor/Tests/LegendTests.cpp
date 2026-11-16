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
  check(app->set_coloring("Model",*saved,error),"Cannot restore legend fixture");
  return failures;
}
