#include "Postprocessor/Core/views/CameraOperations.hpp"
#include "Postprocessor/Commands/views/CameraCommandText.hpp"
#include "Postprocessor/GUI/Viewport/CameraInteraction.hpp"
#include "Postprocessor/GUI/Viewport/CameraTimings.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Framework/Interfaces/ResultSourceRegistryInterface.hpp"
#include "Framework/Interfaces/UserOptionsInterface.hpp"
#include <QApplication>
#include <QComboBox>
#include "Postprocessor/GUI/Options/PostprocessorMouseOptionsPanel.hpp"
#include "Postprocessor/Commands/options/PostprocessorOptionCommands.hpp"
#include <QKeyEvent>
#include <QMouseEvent>
#include <QWidget>
#include <cmath>
#include <iostream>
#include <limits>
#include <locale>
#include <sstream>

namespace
{
int failures = 0;
void check(bool value, const char* message)
{
  if (!value)
  {
    ++failures;
    std::cerr << message << '\n';
  }
}

class Registry : public ResultSourceRegistryInterface
{
  bool resolve_result_provider(const std::string&, ResolvedResultSourceProvider&, std::string&) const override
  {
    return false;
  }
};

class Options : public UserOptionsInterface
{
public:
  std::string path = ".";
  UserOptionValues saved;
  bool fail = false;
  const std::string& component_directory() const noexcept override
  {
    return path;
  }
  const std::string& options_filename() const noexcept override
  {
    return path;
  }
  bool load(const std::string&, const std::vector<UserOption>& definitions, UserOptionValues& values, std::string&) override
  {
    for (const auto& definition : definitions)
    {
      values[definition.key] = saved.count(definition.key) ? saved.at(definition.key) : definition.defaultValue;
    }
    return true;
  }
  bool save(const std::string&, const std::vector<UserOption>&, const UserOptionValues& values, std::string&) override
  {
    if (fail)
    {
      return false;
    }
    saved = values;
    return true;
  }
};

bool parse(const QString& text, Postprocessor::CameraDefinition& camera)
{
  std::istringstream stream(text.toStdString());
  stream.imbue(std::locale::classic());
  std::string word;
  stream >> word >> word >> word >> word;
  for (auto& value : camera.position)
  {
    stream >> value;
  }
  stream >> word;
  for (auto& value : camera.focal_point)
  {
    stream >> value;
  }
  stream >> word;
  for (auto& value : camera.view_up)
  {
    stream >> value;
  }
  stream >> word >> word;
  camera.parallel_projection = word == "parallel";
  stream >> word >> camera.vertical_view_angle_degrees >> word >> camera.parallel_scale;
  return !stream.fail();
}
}

int main(int argc, char** argv)
{
  QApplication qt(argc, argv);
  using namespace Postprocessor;
  CameraDefinition base;
  CameraDefinition moved;
  std::string error;
  check(CameraOperations::rotate(base, 90, 0, moved, error) &&
    std::abs(moved.position[0] - 1) < 1e-12 && std::abs(moved.position[2]) < 1e-12,
    "Right-hand yaw should rotate +Z to +X.");
  check(CameraOperations::pan(base, 2, 3, moved, error) && moved.position[0] == 2 &&
    moved.focal_point[1] == 3 && moved.position[2] == 1, "Pan must preserve direction/distance.");
  check(CameraOperations::zoom(base, 2, moved, error) && moved.position[2] == 0.5,
    "Perspective zoom must dolly.");
  auto parallel = base;
  parallel.parallel_projection = true;
  check(CameraOperations::zoom(parallel, 2, moved, error) && moved.position == base.position &&
    moved.parallel_scale == 0.5, "Parallel zoom must change half-span only.");
  const auto unchanged = moved;
  check(!CameraOperations::zoom(base, 0, moved, error) && CameraOperations::equal(moved, unchanged),
    "Invalid zoom must preserve output.");
  check(!CameraOperations::rotate(base, std::numeric_limits<double>::infinity(), 0, moved, error),
    "Non-finite rotation must be rejected.");
  std::string command;
  base.position[0] = 0.12345678901234567;
  check(format_camera_command(base, command, error) && parse(QString::fromStdString(command), moved) &&
    CameraOperations::equal(base, moved), "Absolute camera text must round-trip exactly.");

  Registry registry;
  auto app = std::make_shared<PostprocessorApplicationInterface>(registry);
  Options options;
  check(app->initialize_user_options(options, error) && !app->user_options().journalCamera,
    "Camera journal must default off.");
  check(!app->user_options().diagnosticsCamera, "Camera diagnostics must default off.");
  check(app->apply_user_options({{"diagnostics/camera", true}}, error) && app->user_options().diagnosticsCamera,
    "Camera diagnostics must enable through validated options.");
  options.fail = true;
  check(!app->apply_user_options({{"diagnostics/camera", false}}, error) && app->user_options().diagnosticsCamera,
    "Failed diagnostic-option persistence must preserve effective state.");
  options.fail = false;
  const auto& defaults = app->user_options();
  check(defaults.backgroundTop == DisplayDefaults::background_top &&
    defaults.backgroundBottom == DisplayDefaults::background_bottom && defaults.triadVisible,
    "Display defaults must be white/white with a visible triad.");
  check(defaults.leftDrag == MouseDragAction::Rotate && defaults.middleDrag == MouseDragAction::Zoom &&
    defaults.rightDrag == MouseDragAction::Pan, "Independent bindings must default to rotate/zoom/pan.");
  check(!app->apply_user_options({{"mouse/left", std::string("invalid")}}, error) &&
    app->user_options().leftDrag == MouseDragAction::Rotate, "Unknown mouse actions must preserve bindings.");
  check(!app->apply_user_options({{"background/top", std::string("1 2 0")}}, error), "Out-of-range RGB must fail.");
  check(!app->apply_user_options({{"background/top", std::string("1 1 1 trailing")}}, error), "Extra RGB tokens must fail.");
  check(app->apply_user_options({{"background/top", std::string("0.2 0.3 0.4")}}, error) &&
    app->user_options().backgroundTop[1] == 0.3, "Valid RGB must publish.");
  for (const auto& definition : app->user_option_definitions())
  {
    if (definition.key == "background/top")
    {
      check(PostprocessorOptionCommands::format(definition, std::string("1 1 1"), command, error) &&
        command == "post useroptions background top 1 1 1", "RGB commands must remain readable triples.");
    }
  }
  std::vector<std::pair<std::string, std::string>> optionCommands;
  QJsonObject mouseBaseline;
  mouseBaseline["mouse/left"] = QStringLiteral("rotate");
  auto mouseEdited = mouseBaseline;
  mouseEdited["mouse/left"] = QStringLiteral("pan");
  check(PostprocessorOptionCommands::edited_commands(app->user_option_definitions(), mouseEdited, mouseBaseline, optionCommands, error) &&
    optionCommands.size() == 1 && optionCommands[0].second == "post useroptions mouse left pan",
    "The mouse page must emit only its changed key using the agreed unquoted command.");
  check(!app->apply_user_options({{"mouse/sensitivity", 0.0}}, error), "Nonpositive sensitivity must fail.");
  check(app->set_camera(base, error), "Absolute camera must initialize an empty view.");
  const auto id = app->session().active_view_id;
  const auto request = app->session().views.at(id).fit_request;
  check(!app->complete_camera_fit(id, request - 1, app->display_revision(), base, error),
    "Explicit camera must invalidate outstanding fits.");
  QWidget surface;
  surface.resize(800, 600);
  int normal = 0;
  int silent = 0;
  auto dispatch = [&](const QString& text, std::string& issue)
  {
    const CameraCommandTiming::Handler handler;
    CameraDefinition camera;
    if (!parse(text, camera))
    {
      return false;
    }
    const CameraCommandTiming::CoreUpdate core;
    return app->set_camera(camera, issue);
  };
  CameraInteraction input(&surface, id, app,
    [&](const QString& text, std::string& issue)
    {
      ++normal;
      return dispatch(text, issue);
    },
    [&](const QString& text, std::string& issue)
    {
      ++silent;
      return dispatch(text, issue);
    }, []()
    {
    }, [&](const std::string&)
    {
      ++failures;
    });
  auto drag = [&]()
  {
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(100, 100), QPointF(100, 100),
      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    input.handle(&press);
    QMouseEvent release(QEvent::MouseButtonRelease, QPointF(200, 150), QPointF(200, 150),
      Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
    input.handle(&release);
  };
  drag();
  check(normal == 0 && silent == 0, "Default drag must update Core without either command dispatcher.");
  check(app->apply_user_options({{"journal/camera", true}}, error), "Enable camera journal.");
  drag();
  check(normal == 1 && silent == 0, "Enabled drag must restore baseline then commit exactly once.");
  options.fail = true;
  check(!app->apply_user_options({{"journal/camera", false}}, error) && app->user_options().journalCamera,
    "Failed persistence must preserve the effective option.");
  const auto revision = app->session().views.at(id).camera_revision;
  auto invalid = base;
  invalid.view_up = {0, 0, 0};
  check(!app->set_camera(invalid, error) && app->session().views.at(id).camera_revision == revision,
    "Invalid camera must preserve revision and published state.");
  auto preview_drag = [&]()
  {
    QMouseEvent press(QEvent::MouseButtonPress, QPointF(100, 100), QPointF(100, 100),
      Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
    input.handle(&press);
    QMouseEvent move(QEvent::MouseMove, QPointF(250, 200), QPointF(250, 200),
      Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    input.handle(&move);
    check(input.preview_pending(), "Movement should queue a preview without dispatching it.");
    input.flush_preview();
  };
  const auto beforeCancel = *app->session().views.at(id).camera;
  preview_drag();
  check(!CameraOperations::equal(beforeCancel, *app->session().views.at(id).camera),
    "A frame preview should update Core before release.");
  QKeyEvent escape(QEvent::KeyPress, Qt::Key_Escape, Qt::NoModifier);
  input.handle(&escape);
  check(CameraOperations::equal(beforeCancel, *app->session().views.at(id).camera) && normal == 1 && silent == 0,
    "Escape must silently restore the baseline without a commit.");
  preview_drag();
  check(app->set_camera(base, error), "Competing command should publish a newer camera.");
  input.synchronize();
  input.cancel();
  check(CameraOperations::equal(base, *app->session().views.at(id).camera),
    "Cancellation must preserve a newer external camera.");
  options.fail = false;
  check(app->apply_user_options({{"journal/camera", true}}, error), "Cannot set reset-preserved option");
  preview_drag();
  const int commandsBeforeReset = normal;
  check(app->reset(error), "Cannot reset during a camera gesture");
  input.synchronize();
  input.handle(&escape);
  check(!app->session().views.at(id).camera && normal == commandsBeforeReset && silent == 0 &&
    app->user_options().journalCamera, "Reset restored an old gesture or changed persistent options");
  check(app->set_camera(base, error), "Cannot initialize camera after reset");
  QMouseEvent press(QEvent::MouseButtonPress, QPointF(400, 300), QPointF(400, 300),
    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  input.handle(&press);
  const auto focal = app->session().views.at(id).camera->focal_point;
  const int beforeBurst = silent;
  const auto beforeBurstRevision = app->session().views.at(id).camera_revision;
  for (int i = 1; i <= 100; ++i)
  {
    QMouseEvent move(QEvent::MouseMove, QPointF(400 + i, 300), QPointF(400 + i, 300),
      Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
    input.handle(&move);
  }
  check(silent == beforeBurst, "Input events must not dispatch before a frame is requested.");
  input.flush_preview();
  check(silent == beforeBurst && app->session().views.at(id).camera->focal_point == focal &&
    app->session().views.at(id).camera_revision == beforeBurstRevision + 1,
    "One frame must consume the latest orbit input exactly once and preserve its center.");
  input.flush_preview();
  check(silent == beforeBurst && app->session().views.at(id).camera_revision == beforeBurstRevision + 1,
    "A clean frame must not publish another camera preview.");
  input.cancel();
  check(CameraOperations::roll(base, 90, moved, error) && moved.position == base.position &&
    moved.focal_point == base.focal_point, "Spin must preserve camera position and rotation center.");
  CameraDefinition standard;
  check(CameraOperations::standard_view(base, CameraOperations::StandardView::Front, standard, error) &&
    standard.position[1] < standard.focal_point[1] && standard.view_up[2] == 1,
    "Front must look from -Y with Z up.");
  check(CameraOperations::standard_view(base, CameraOperations::StandardView::Orthographic, standard, error) &&
    standard.parallel_projection && standard.position == base.position && standard.parallel_scale > 0,
    "Ortho must preserve position and establish a finite parallel span.");
  const int beforePreset = normal;
  QKeyEvent top(QEvent::KeyPress, Qt::Key_F10, Qt::NoModifier);
  input.handle(&top);
  check(normal == beforePreset + 1 && app->session().views.at(id).camera->position[2] >
    app->session().views.at(id).camera->focal_point[2], "F10 should commit Top through the camera command.");
  const auto lastColor = app->user_options().backgroundTop;
  options.fail = true;
  check(!app->apply_user_options({{"background/top", std::string("1 0 0")}}, error) &&
    app->user_options().backgroundTop == lastColor, "Failed color persistence must preserve published appearance.");
  options.fail = false;
  check(app->apply_user_options({{"journal/camera", false}, {"mouse/left", std::string("zoom")}}, error),
    "Remapping left drag must succeed.");
  check(app->set_camera(base, error), "Prepare a remapping test camera.");
  QMouseEvent remapPress(QEvent::MouseButtonPress, QPointF(400, 300), QPointF(400, 300),
    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  input.handle(&remapPress);
  check(app->apply_user_options({{"mouse/left", std::string("pan")}}, error), "Change next-gesture action.");
  QMouseEvent remapMove(QEvent::MouseMove, QPointF(400, 200), QPointF(400, 200),
    Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
  input.handle(&remapMove);
  input.flush_preview();
  check(app->session().views.at(id).camera->focal_point == base.focal_point &&
    app->session().views.at(id).camera->position != base.position,
    "Changing a binding must not turn an active zoom into a pan.");
  input.cancel();
  const int beforeClick = silent;
  input.handle(&remapPress);
  QMouseEvent clickRelease(QEvent::MouseButtonRelease, QPointF(400, 300), QPointF(400, 300),
    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
  input.handle(&clickRelease);
  check(silent == beforeClick, "A click without a drag must not dispatch navigation.");
  bool reject = false;
  PostprocessorMouseOptionsPanel panel(app, [&](const std::string& text, std::string& issue)
  {
    if (reject)
    {
      // The native dispatcher can accept submission despite handler failure.
      return true;
    }
    std::istringstream stream(text);
    std::string word, key, value;
    stream >> word >> word >> word >> key >> value;
    return app->apply_user_options({{"mouse/" + key, value}}, issue);
  }, &surface);
  auto* combo = panel.findChild<QComboBox*>("Postprocessor.Mouse.Left");
  check(combo != nullptr, "Mouse page must expose its independent left-drag control.");
  if (combo != nullptr)
  {
    combo->setCurrentIndex(combo->findData(QStringLiteral("rotate")));
    check(panel.save_settings(error) && app->user_options().leftDrag == MouseDragAction::Rotate,
      "Mouse page must apply through its command callback.");
    combo->setCurrentIndex(combo->findData(QStringLiteral("zoom")));
    panel.refresh_settings();
    check(combo->currentData().toString() == "rotate", "Refresh must discard unapplied edits.");
    combo->setCurrentIndex(combo->findData(QStringLiteral("zoom")));
    reject = true;
    check(!panel.save_settings(error) && app->user_options().leftDrag == MouseDragAction::Rotate,
      "Accepted-but-rejected submission must not be mistaken for a saved binding.");
  }
  options.fail = false;
  check(app->apply_user_options({{"diagnostics/camera", true}}, error), "Enable integrated gesture timing.");
  CameraTimings capture;
  CameraInteraction measured(&surface, id, app, dispatch, dispatch, [&]()
  {
    capture.queue_frame();
  }, [&](const std::string&)
  {
    ++failures;
  }, &capture);
  QMouseEvent measuredPress(QEvent::MouseButtonPress, QPointF(400, 300), QPointF(400, 300),
    Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
  measured.handle(&measuredPress);
  QMouseEvent measuredMove(QEvent::MouseMove, QPointF(500, 330), QPointF(500, 330),
    Qt::NoButton, Qt::LeftButton, Qt::NoModifier);
  measured.handle(&measuredMove);
  measured.flush_preview();
  QMouseEvent measuredRelease(QEvent::MouseButtonRelease, QPointF(500, 330), QPointF(500, 330),
    Qt::LeftButton, Qt::NoButton, Qt::NoModifier);
  measured.handle(&measuredRelease);
  check(!capture.reports_ready(), "Measured gesture must retain its final queued frame after release.");
  capture.begin_frame();
  capture.await_presentation();
  capture.presented();
  const auto summary = capture.take_report();
  check(summary && !summary->cancelled && summary->presented == 1 &&
    summary->metrics[static_cast<std::size_t>(CameraTimingStage::CommandDispatch)].count > 0 &&
    summary->metrics[static_cast<std::size_t>(CameraTimingStage::Formatting)].count > 0 &&
    summary->metrics[static_cast<std::size_t>(CameraTimingStage::InputWait)].count > 0 &&
    summary->metrics[static_cast<std::size_t>(CameraTimingStage::CommandHandler)].count > 0 &&
    summary->metrics[static_cast<std::size_t>(CameraTimingStage::CoreCameraUpdate)].count > 0 &&
    summary->metrics[static_cast<std::size_t>(CameraTimingStage::CommandOutsideHandler)].count > 0 &&
    summary->unpairedDispatches == 0,
    "Controller dispatches must pair handler/Core timings with their originating gesture.");
  return failures == 0 ? 0 : 1;
}
