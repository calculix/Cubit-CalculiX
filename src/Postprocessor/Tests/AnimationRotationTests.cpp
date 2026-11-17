#include "AnimationRotationTests.hpp"
#include "Postprocessor/Core/representations/AnimationRotationRuntime.hpp"
#include "Postprocessor/Core/representations/AnimationRotationCodec.hpp"
#include "Postprocessor/Core/views/CameraOperations.hpp"
#include "Postprocessor/GUI/Navigation/PostprocessorFrameNavigation.hpp"
#include <QCoreApplication>
#include <QToolButton>
#include <QTimer>
#include <QJsonObject>
#include <cmath>
#include <limits>
#include <iostream>

int animation_rotation_tests()
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
  AnimationRotationRuntime rotation;
  rotation.base.position = {0,-10,0};
  rotation.base.view_up = {0,0,1};
  rotation.settings[2].enabled = true;
  rotation.settings[2].degrees = 360; // Explicit full-turn fixture, independent of defaults.
  CameraDefinition camera;
  std::string error;
  check(rotation.advance(0.25,1.0,camera,error) && std::abs(camera.position[0]-10) < 1e-10 &&
    std::abs(camera.position[1]) < 1e-10,"Elapsed rotation did not advance between mesh frames");
  check(rotation.advance(1.0,1.0,camera,error) && std::abs(camera.position[0]-10) < 1e-10 &&
    std::abs(rotation.phase-1.25) < 1e-12,"Rotation phase reset at the loop boundary");
  rotation.phase = 0;
  rotation.settings[2].degrees = 180;
  check(rotation.advance(2.0,2.0,camera,error) && std::abs(camera.position[1]-10) < 1e-10,
    "Custom angle did not complete over the pass duration");
  check(rotation.advance(1.0,2.0,camera,error) && std::abs(camera.position[0]+10) < 1e-10,
    "Custom angle snapped back on the following pass");
  const auto previous = camera;
  const double phase = rotation.phase;
  check(!rotation.advance(-1,2,camera,error) && CameraOperations::equal(previous,camera) && rotation.phase == phase,
    "Invalid elapsed time changed the rotation state");
  rotation.settings[0] = {true,-90};
  rotation.settings[1] = {true,45};
  CameraDefinition combined;
  check(CameraOperations::rotate_world(rotation.base,{-90,45,180},combined,error) &&
    CameraOperations::valid(combined) && combined.focal_point == rotation.base.focal_point &&
    combined.parallel_scale == rotation.base.parallel_scale,"Combined rotations changed camera framing");
  auto encoded = animation_rotation_to_json(rotation.settings);
  AnimationRotation restored;
  check(animation_rotation_from_json(encoded,restored,error) && same_animation_rotation(restored,rotation.settings),
    "Rotation persistence lost axis flags or custom angles");
  auto invalid = encoded;
  invalid[0] = QJsonObject{{"enabled",true},{"degrees","invalid"}};
  check(!animation_rotation_from_json(invalid,restored,error) && same_animation_rotation(restored,rotation.settings),
    "Malformed rotation restore changed committed settings");
  check(animation_rotation_from_json(QJsonValue(QJsonValue::Undefined),restored,error),"Legacy rotation restore failed");
  for (const auto& axis : restored)
  {
    check(!axis.enabled && axis.degrees == 180,"Legacy rotation defaults differ");
  }
  restored[0].degrees = std::numeric_limits<double>::infinity();
  check(!valid_animation_rotation(restored,error),"Nonfinite rotation angle was accepted");

  auto data = std::make_shared<SimResults::VtkResultsData>();
  data->increments.resize(3);
  const auto hasMesh = [](std::size_t frame)
  {
    return frame < 3;
  };
  PostprocessorFrameNavigation* owner = nullptr;
  AnimationDefinition settings;
  std::size_t shown = 0;
  int commands = 0, commits = 0, previews = 0, cameraTicks = 0;
  bool resetOnReady = false, resetOnPreview = false, resetOnRotation = false;
  const auto reset = [&]()
  {
    owner->reset_navigation();
    owner->refresh({},std::nullopt,{},"",{});
  };
  PostprocessorFrameNavigation navigation([&](std::size_t frame)
  {
    ++commits;
    shown = frame-1;
  },nullptr,[&](std::size_t frame)
  {
    ++previews;
    shown = frame-1;
    if (resetOnPreview)
    {
      reset();
    }
    return true;
  },[&](const QString& command)
  {
    ++commands;
    if (command == "play")
    {
      settings.playing = true;
    }
    else if (command == "pause")
    {
      settings.playing = false;
    }
    else if (command == "rotation x on")
    {
      settings.rotation[0].enabled = true;
    }
    owner->refresh(data,shown,hasMesh,"owner",settings);
  },[&]()
  {
    if (resetOnReady)
    {
      reset();
    }
    return true;
  },[&](double seconds)
  {
    check(seconds >= 0 && std::isfinite(seconds),"Rotation timer produced invalid elapsed time");
    ++cameraTicks;
    if (resetOnRotation)
    {
      reset();
    }
    return true;
  });
  owner = &navigation;
  navigation.refresh(data,shown,hasMesh,"owner",settings);
  navigation.show();
  auto* play = navigation.findChild<QToolButton*>("Postprocessor.Animation.Play");
  auto* meshTimer = navigation.findChild<QTimer*>("Postprocessor.Animation.Timer");
  auto* cameraTimer = navigation.findChild<QTimer*>("Postprocessor.Animation.RotationTimer");
  auto* x = navigation.findChild<QToolButton*>("Postprocessor.Animation.Rotation.x");
  auto* y = navigation.findChild<QToolButton*>("Postprocessor.Animation.Rotation.y");
  auto* z = navigation.findChild<QToolButton*>("Postprocessor.Animation.Rotation.z");
  check(play && meshTimer && cameraTimer && x && y && z,"Rotation controls/timers are missing");
  if (!play || !meshTimer || !cameraTimer || !x || !y || !z)
  {
    return failures;
  }
  check(!x->isChecked() && !y->isChecked() && !z->isChecked() && commands == 0,"Rotation buttons did not default off");
  x->click();
  check(commands == 1 && settings.rotation[0].enabled,"Rotation toggle bypassed command submission");
  play->click();
  check(meshTimer->isActive() && cameraTimer->isActive() && cameraTimer->interval() == 16,
    "Rotation did not start its independent smooth timer");
  QMetaObject::invokeMethod(cameraTimer,"timeout",Qt::DirectConnection);
  check(cameraTicks == 1 && previews == 0,"Camera motion advanced a result frame");
  const int before = commands;
  resetOnReady = true;
  QMetaObject::invokeMethod(meshTimer,"timeout",Qt::DirectConnection);
  check(!meshTimer->isActive() && !cameraTimer->isActive() && previews == 0 && commands == before,
    "Reset inside readiness callback resumed playback or submitted nested commands");
  resetOnReady = false;
  shown = 0;
  settings.playing = true;
  navigation.refresh(data,shown,hasMesh,"owner",settings);
  resetOnPreview = true;
  QMetaObject::invokeMethod(meshTimer,"timeout",Qt::DirectConnection);
  check(!meshTimer->isActive() && !cameraTimer->isActive() && commands == before,
    "Reset inside frame dispatch restarted timers");
  resetOnPreview = false;
  shown = 0;
  navigation.refresh(data,shown,hasMesh,"owner",settings);
  resetOnRotation = true;
  QMetaObject::invokeMethod(cameraTimer,"timeout",Qt::DirectConnection);
  check(!meshTimer->isActive() && !cameraTimer->isActive() && commands == before,
    "Reset inside rotation dispatch restarted timers");
  resetOnRotation = false;
  shown = 0;
  navigation.refresh(data,shown,hasMesh,"owner",settings);
  navigation.stop_playback();
  const int stopped = commands;
  reset();
  // Recreate the same apparent context before the old completion callback runs.
  settings.playing = false;
  navigation.refresh(data,shown,hasMesh,"owner",settings);
  QCoreApplication::processEvents();
  check(commits == 0 && commands == stopped,"A queued pre-reset frame commit survived reset");
  settings.playing = true;
  navigation.refresh(data,shown,hasMesh,"owner",settings);
  reset();
  navigation.hide();
  QCoreApplication::processEvents();
  check(commands == stopped && !meshTimer->isActive() && !cameraTimer->isActive(),
    "Hiding navigation during model reset submitted pause or retained timers");
  return failures;
}
