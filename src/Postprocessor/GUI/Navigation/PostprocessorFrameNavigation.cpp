#include "Framework/Results/ResultsUnits.hpp"
#include "Framework/Core/units/FrameworkUnitsInterface.hpp"
#include <type_traits>
#include "PostprocessorFrameNavigation.hpp"
#include <QComboBox>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSlider>
#include <QHideEvent>
#include <QShowEvent>
#include <QSpinBox>
#include <QAbstractSlider>
#include <QSignalBlocker>
#include <QStyle>
#include <QToolButton>
#include <cmath>
#include <algorithm>
#include <utility>
#include <exception>
#include "CubitMessage.hpp"

PostprocessorFrameNavigation::PostprocessorFrameNavigation(std::function<void(std::size_t)> callback, QWidget* parent,
  std::function<bool(std::size_t)> previewCallback, std::function<void(const QString&)> animationCallback,std::function<bool()> ready,
  std::function<bool(double)> rotation)
  : QWidget(parent), select(std::move(callback)), preview(std::move(previewCallback)), animationCommand(std::move(animationCallback)),
    animationReady(std::move(ready)),rotationStep(std::move(rotation))
{
  setObjectName("Postprocessor.FrameNavigation");
  setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
  auto* layout = new QHBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  layout->setSpacing(4);
  auto button = [this, layout](QStyle::StandardPixmap icon, const QString& tooltip)
  {
    auto* control = new QToolButton(this);
    control->setIcon(style()->standardIcon(icon));
    control->setToolTip(tooltip);
    layout->addWidget(control);
    return control;
  };
  first = button(QStyle::SP_MediaSkipBackward, tr("First displayable frame"));
  previous = button(QStyle::SP_MediaSeekBackward, tr("Previous displayable frame"));
  play = button(QStyle::SP_MediaPlay,tr("Play result animation"));
  play->setObjectName("Postprocessor.Animation.Play");
  frames = new QComboBox(this);
  frames->setObjectName("Postprocessor.Frames");
  frames->setToolTip(tr("Result frame of the active source in the active view"));
  frames->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLengthWithIcon);
  frames->setMinimumContentsLength(18);
  layout->addWidget(frames);
  next = button(QStyle::SP_MediaSeekForward, tr("Next displayable frame"));
  last = button(QStyle::SP_MediaSkipForward, tr("Last displayable frame"));
  fps = new QSpinBox(this);
  fps->setObjectName("Postprocessor.Animation.Fps");
  fps->setRange(Postprocessor::AnimationDefaults::minimum_fps,Postprocessor::AnimationDefaults::maximum_fps);
  fps->setValue(Postprocessor::AnimationDefaults::frames_per_second);
  fps->setSuffix(tr(" fps"));
  fps->setToolTip(tr("Target animation frames per second"));
  layout->addWidget(fps);
  loop = new QToolButton(this);
  loop->setCheckable(true);
  loop->setToolButtonStyle(Qt::ToolButtonIconOnly);
  loop->setIconSize(QSize(24,24));
  loop->setText(tr("Loop"));
  loop->setAccessibleName(tr("Loop"));
  loop->setObjectName("Postprocessor.Animation.Loop");
  loop->setChecked(Postprocessor::AnimationDefaults::loop);
  loop->setToolTip(tr("Restart at the first displayable frame after the last frame"));
  layout->addWidget(loop);
  const char* axes[] = {"x","y","z"};
  for (std::size_t axis = 0; axis < rotations.size(); ++axis)
  {
    auto* control = new QToolButton(this);
    control->setObjectName("Postprocessor.Animation.Rotation."+QString::fromLatin1(axes[axis]));
    control->setCheckable(true);
    control->setToolButtonStyle(Qt::ToolButtonIconOnly);
    control->setIconSize(QSize(24,24));
    const auto label = tr("Rotate around %1").arg(QString::fromLatin1(axes[axis]).toUpper());
    control->setText(label);
    control->setAccessibleName(label);
    layout->addWidget(control);
    rotations[axis] = control;
    connect(control,&QToolButton::toggled,this,[this,axis](bool checked)
    {
      if (animationCommand)
      {
        const char* names[] = {"x","y","z"};
        QString state = "off";
        if (checked)
        {
          state = "on";
        }
        animationCommand("rotation "+QString::fromLatin1(names[axis])+" "+state);
      }
    });
  }
  rotationTimer.setParent(this);
  rotationTimer.setObjectName("Postprocessor.Animation.RotationTimer");
  rotationTimer.setTimerType(Qt::PreciseTimer);
  rotationTimer.setInterval(16);
  connect(&rotationTimer,&QTimer::timeout,this,[this]()
  {
    advance_rotation();
  });
  animationTimer.setParent(this);
  animationTimer.setObjectName("Postprocessor.Animation.Timer");
  animationTimer.setTimerType(Qt::PreciseTimer);
  animationTimer.setSingleShot(true);
  connect(&animationTimer,&QTimer::timeout,this,[this]()
  {
    try
    {
      advance_animation();
    }
    catch (const std::exception& exception)
    {
      if (QString::fromUtf8(exception.what()).trimmed() == QStringLiteral("Cubit is busy"))
      {
        if (running)
        {
          animationTimer.start(Postprocessor::AnimationDefaults::presentation_retry_ms);
        }
        return;
      }
      reset_navigation();
      PRINT_ERROR("Cannot advance animation: %s\n",exception.what());
    }
    catch (...)
    {
      reset_navigation();
      PRINT_ERROR("Cannot advance animation: unknown failure.\n");
    }
  });
  connect(play,&QToolButton::clicked,this,[this]()
  {
    if (running)
    {
      stop_playback();
    }
    else if (animationCommand)
    {
      finish_drag();
      animationCommand(QStringLiteral("play"));
    }
  });
  connect(fps,&QSpinBox::valueChanged,this,[this](int value)
  {
    if (animationCommand)
    {
      animationCommand("fps "+QString::number(value));
    }
  });
  connect(loop,&QToolButton::toggled,this,[this](bool checked)
  {
    if (animationCommand)
    {
      animationCommand([&]() -> std::common_type_t<decltype(QStringLiteral("loop on")), decltype(QStringLiteral("loop off"))> {
        if (checked) {
          return QStringLiteral("loop on");
        } else {
          return QStringLiteral("loop off");
        }
      }());
    }
  });
  connect(frames, &QComboBox::activated, this, [this](int index)
  {
    choose(index);
  });
  connect(first, &QToolButton::clicked, this, [this]()
  {
    choose(0);
  });
  connect(previous, &QToolButton::clicked, this, [this]()
  {
    choose(frames->currentIndex() - 1);
  });
  connect(next, &QToolButton::clicked, this, [this]()
  {
    choose(frames->currentIndex() + 1);
  });
  connect(last, &QToolButton::clicked, this, [this]()
  {
    choose(static_cast<int>(indices.size()) - 1);
  });
  slider = new QSlider(Qt::Horizontal,this);
  slider->setObjectName("Postprocessor.FrameSlider");
  slider->setToolTip(tr("Drag to preview frames; release to journal the selected frame"));
  slider->setMinimumWidth(100);
  slider->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Fixed);
  layout->insertWidget(layout->indexOf(fps),slider,1);
  previewTimer.setSingleShot(true);
  connect(&previewTimer,&QTimer::timeout,this,[this]()
  {
    flush_preview();
  });
  connect(slider,&QSlider::sliderPressed,this,[this]()
  {
    const auto ownerGeneration = generation;
    stop_playback(false);
    if (generation != ownerGeneration || indices.empty())
    {
      return;
    }
    dragging = true;
    baseline = expected;
    pending.reset();
  });
  connect(slider,&QSlider::valueChanged,this,[this](int position)
  {
    update_controls(position);
    if (dragging)
    {
      pending = position;
      if (!previewTimer.isActive())
      {
        previewTimer.start(0);
      }
    }
    else
    {
      choose(position);
    }
  });
  connect(slider,&QSlider::sliderReleased,this,[this]()
  {
    finish_drag();
  });
  refresh({}, std::nullopt, {});
}

void PostprocessorFrameNavigation::choose(int position)
{
  if (position >= 0 && static_cast<std::size_t>(position) < indices.size())
  {
    const auto ownerGeneration = generation;
    const auto frame = indices[static_cast<std::size_t>(position)];
    stop_playback(false);
    if (generation == ownerGeneration && select)
    {
      select(frame+1);
    }
  }
}

void PostprocessorFrameNavigation::refresh(const std::shared_ptr<const SimResults::VtkResultsData>& data,
  std::optional<std::size_t> current, const std::function<bool(std::size_t)>& has_mesh, const std::string& key,
  const Postprocessor::AnimationDefinition& animation, bool allow_animation)
{
  const auto unitGeneration = framework_units().generation();
  if (!labelUnitGeneration || *labelUnitGeneration != unitGeneration)
  {
    refresh_frame_labels();
    labelUnitGeneration = unitGeneration;
  }
  playbackAllowed = allow_animation;
  const bool owned = snapshot == data && context == key && current == expected;
  const bool externalFrame = running && snapshot == data && context == key && current != expected;
  if (owned && (dragging || running))
  {
    const bool completed = running && !animation.playing;
    synchronize_animation(animation);
    if (completed)
    {
      commit_animation_frame();
    }
    return;
  }
  if (!owned)
  {
    ++generation;
  }
  animationTimer.stop();
  rotationTimer.stop();
  rotationClock.invalidate();
  running = false;
  abandon_drag();
  snapshot = data;
  context = key;
  expected = current;
  const QSignalBlocker blocker(frames);
  const QSignalBlocker sliderBlocker(slider);
  frames->clear();
  indices.clear();
  int selected = -1;
  if (data)
  {
    for (std::size_t i = 0; i < data->increments.size(); ++i)
    {
      if (!has_mesh(i))
      {
        continue;
      }
      const auto& value = data->increments[i];
      if (current && *current == i)
      {
        selected = static_cast<int>(indices.size());
      }
      indices.push_back(i);
      if (value.topology_iteration > 0)
      {
        frames->addItem(parameter_text(value));
        continue;
      }
      frames->addItem(tr("Frame %1 | Step %2 | Increment %3 | %4")
        .arg(static_cast<qulonglong>(i + 1))
        .arg([&]() -> std::common_type_t<decltype(tr("unknown")), decltype(QString::number(value.step))> {
          if (value.step < 0) {
            return tr("unknown");
          } else {
            return QString::number(value.step);
          }
        }())
        .arg([&]() -> std::common_type_t<decltype(tr("unknown")), decltype(QString::number(value.increment))> {
          if (value.increment < 0) {
            return tr("unknown");
          } else {
            return QString::number(value.increment);
          }
        }())
        .arg(parameter_text(value)));
    }
  }
  slider->setRange(0,std::max(0,static_cast<int>(indices.size())-1));
  slider->setValue([&]() -> std::common_type_t<decltype(0), decltype(selected)> {
    if (selected < 0) {
      return 0;
    } else {
      return selected;
    }
  }());
  slider->setEnabled(indices.size() > 1 && selected >= 0);
  auto effective = animation;
  if (externalFrame)
  {
    effective.playing = false;
  }
  synchronize_animation(effective);
  update_controls(selected);
  if (externalFrame && animation.playing)
  {
    const auto frame = expected;
    const auto source = snapshot;
    const auto owner = context;
    const auto ownerGeneration = generation;
    QTimer::singleShot(0,this,[this,frame,source,owner,ownerGeneration]()
    {
      if (generation == ownerGeneration && !running && expected == frame && snapshot == source && context == owner && animationCommand)
      {
        animationCommand(QStringLiteral("pause"));
      }
    });
  }
}

bool PostprocessorFrameNavigation::matches_context(const std::string& key,
  const std::shared_ptr<const SimResults::VtkResultsData>& data, std::optional<std::size_t> current) const
{
  return context == key && snapshot == data && current == expected;
}

void PostprocessorFrameNavigation::update_controls(int selected)
{
  const QSignalBlocker blocker(frames);
  frames->setCurrentIndex(selected);
  frames->setToolTip([&]() -> std::common_type_t<decltype(frames->itemText(selected)), decltype(tr("Result frame of the active source in the active view"))> {
    if (selected >= 0) {
      return frames->itemText(selected);
    } else {
      return tr("Result frame of the active source in the active view");
    }
  }());
  frames->setEnabled(!indices.empty());
  first->setEnabled(!dragging && !indices.empty() && selected != 0);
  previous->setEnabled(!dragging && selected > 0);
  next->setEnabled(!dragging && !indices.empty() && selected + 1 < static_cast<int>(indices.size()));
  last->setEnabled(!dragging && !indices.empty() && selected != static_cast<int>(indices.size())-1);
}

void PostprocessorFrameNavigation::flush_preview()
{
  previewTimer.stop();
  if (!dragging || !pending || *pending < 0 || static_cast<std::size_t>(*pending) >= indices.size())
  {
    return;
  }
  const auto frame = indices[static_cast<std::size_t>(*pending)];
  pending.reset();
  if (expected == frame)
  {
    return;
  }
  const auto ownerGeneration = generation;
  if (preview && !preview(frame+1))
  {
    abandon_drag();
    return;
  }
  if (generation == ownerGeneration && dragging)
  {
    expected = frame;
  }
}

void PostprocessorFrameNavigation::finish_drag()
{
  if (!dragging)
  {
    return;
  }
  flush_preview();
  if (!dragging)
  {
    return;
  }
  const auto chosen = expected;
  const bool changed = chosen != baseline;
  dragging = false;
  pending.reset();
  if (changed && chosen)
  {
    select(*chosen+1);
  }
  update_controls(frames->currentIndex());
}

void PostprocessorFrameNavigation::abandon_drag()
{
  previewTimer.stop();
  pending.reset();
  dragging = false;
  const QSignalBlocker blocker(slider);
  slider->setSliderDown(false);
}

void PostprocessorFrameNavigation::hideEvent(QHideEvent* event)
{
  const auto ownerGeneration = generation;
  stop_playback();
  const auto restore = baseline;
  const bool changed = dragging && expected != baseline;
  abandon_drag();
  // A workspace switch cancels a drag rather than retaining an unjournaled
  // final preview. The owner callback guards source/view/snapshot identity.
  if (generation == ownerGeneration && changed && restore && preview && preview(*restore+1) && generation == ownerGeneration)
  {
    expected = restore;
  }
  QWidget::hideEvent(event);
}

PostprocessorFrameNavigation::~PostprocessorFrameNavigation()
{
  reset_navigation();
  rotationStep = {};
  animationCommand = {};
  preview = {};
}

void PostprocessorFrameNavigation::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  synchronize_animation(playback);
}

void PostprocessorFrameNavigation::set_loop_icon(const QIcon& icon)
{
  loop->setIcon(icon);
}

void PostprocessorFrameNavigation::synchronize_animation(const Postprocessor::AnimationDefinition& animation)
{
  const int previousRate = playback.frames_per_second;
  const bool wasRunning = running;
  const auto previousRotation = playback.rotation;
  playback = animation;
  const QSignalBlocker loopBlocker(loop);
  const QSignalBlocker fpsBlocker(fps);
  loop->setChecked(playback.loop);
  fps->setValue(playback.frames_per_second);
  const bool canPlay = playbackAllowed && indices.size() > 1 && expected.has_value() && static_cast<bool>(animationCommand) && static_cast<bool>(preview);
  running = playback.playing && canPlay && isVisible();
  if (!running)
  {
    animationTimer.stop();
  }
  else if (!animationTimer.isActive() || previousRate != playback.frames_per_second)
  {
    animationTimer.start(static_cast<int>(std::ceil(1000.0/playback.frames_per_second)));
  }
  play->setIcon(style()->standardIcon([&]() -> std::common_type_t<decltype(QStyle::SP_MediaPause), decltype(QStyle::SP_MediaPlay)> {
    if (running) {
      return QStyle::SP_MediaPause;
    } else {
      return QStyle::SP_MediaPlay;
    }
  }()));
  play->setToolTip([&]() -> std::common_type_t<decltype(tr("Pause result animation")), decltype(tr("Play result animation"))> {
    if (running) {
      return tr("Pause result animation");
    } else {
      return tr("Play result animation");
    }
  }());
  bool rotate = false;
  const char* axes[] = {"X","Y","Z"};
  for (std::size_t axis = 0; axis < rotations.size(); ++axis)
  {
    const QSignalBlocker blocker(rotations[axis]);
    rotations[axis]->setChecked(playback.rotation[axis].enabled);
    rotations[axis]->setEnabled(playbackAllowed && !indices.empty() && static_cast<bool>(animationCommand));
    rotations[axis]->setToolTip(tr("Rotate around world %1: %2 degrees per pass")
      .arg(QString::fromLatin1(axes[axis])).arg(QString::number(playback.rotation[axis].degrees,'g',8)));
    rotate = rotate || playback.rotation[axis].enabled;
  }
  if (running && rotate && rotationStep)
  {
    if (!wasRunning || !rotationClock.isValid() || !Postprocessor::same_animation_rotation(previousRotation,playback.rotation))
    {
      rotationClock.start();
    }
    if (!rotationTimer.isActive())
    {
      rotationTimer.start();
    }
  }
  else
  {
    rotationTimer.stop();
    rotationClock.invalidate();
  }
  play->setEnabled(canPlay);
  fps->setEnabled(playbackAllowed && !indices.empty() && static_cast<bool>(animationCommand));
  loop->setEnabled(playbackAllowed && !indices.empty() && static_cast<bool>(animationCommand));
}

void PostprocessorFrameNavigation::commit_animation_frame()
{
  const auto frame = expected;
  const auto data = snapshot;
  const auto key = context;
  const auto ownerGeneration = generation;
  QTimer::singleShot(0,this,[this,frame,data,key,ownerGeneration]()
  {
    if (generation == ownerGeneration && frame && expected == frame && snapshot == data && context == key && !running && select)
    {
      select(*frame+1);
    }
  });
}

void PostprocessorFrameNavigation::stop_playback(bool commit)
{
  if (!running)
  {
    return;
  }
  advance_rotation();
  if (!running)
  {
    return;
  }
  const auto ownerGeneration = generation;
  running = false;
  playback.playing = false;
  animationTimer.stop();
  rotationTimer.stop();
  rotationClock.invalidate();
  if (animationCommand)
  {
    animationCommand(QStringLiteral("pause"));
  }
  if (generation != ownerGeneration)
  {
    return;
  }
  synchronize_animation(playback);
  if (commit)
  {
    commit_animation_frame();
  }
}

void PostprocessorFrameNavigation::advance_animation()
{
  if (!running || !expected)
  {
    return;
  }
  const auto ownerGeneration = generation;
  if (animationReady && !animationReady())
  {
    if (generation == ownerGeneration && running)
    {
      animationTimer.start(Postprocessor::AnimationDefaults::presentation_retry_ms);
    }
    return;
  }
  if (generation != ownerGeneration || !running || !expected)
  {
    return;
  }
  const auto found = std::find(indices.begin(),indices.end(),*expected);
  if (found == indices.end())
  {
    stop_playback(false);
    return;
  }
  auto position = static_cast<std::size_t>(found-indices.begin())+1;
  if (position >= indices.size())
  {
    if (!playback.loop)
    {
      stop_playback();
      return;
    }
    position = 0;
  }
  const auto frame = indices[position];
  if (!preview || !preview(frame+1))
  {
    stop_playback(false);
    return;
  }
  if (generation != ownerGeneration || !running)
  {
    return;
  }
  expected = frame;
  {
    const QSignalBlocker blocker(slider);
    slider->setValue(static_cast<int>(position));
  }
  update_controls(static_cast<int>(position));
  // Schedule after completing work. Avoid queued catch-up frames when rendering
  // takes longer than the target interval.
  animationTimer.start(static_cast<int>(std::ceil(1000.0/playback.frames_per_second)));
}

QString PostprocessorFrameNavigation::parameter_text(const SimResults::Increment& frame) const
{
  if (frame.topology_iteration > 0)
  {
    return tr("Topology iteration %1 | Job %2").arg(frame.topology_iteration).arg(frame.topology_job);
  }
  double displayed = 0.0;
  std::string unit;
  std::string error;
  if (!SimResults::ResultsUnits::display_value(frame.time,frame.independent_variable,
        "postprocessor",displayed,unit,error) || !std::isfinite(displayed))
  {
    return tr("Parameter unavailable");
  }
  QString name = tr("Analysis parameter");
  if (frame.independent_variable.quantity == "time")
  {
    name = tr("Time");
  }
  else if (frame.independent_variable.quantity == "frequency")
  {
    name = tr("Frequency");
  }
  return tr("%1 %2 [%3]").arg(name).arg(QString::number(displayed,'g',16)).arg(QString::fromStdString(unit));
}
void PostprocessorFrameNavigation::refresh_frame_labels()
{
  if (!snapshot)
  {
    return;
  }
  const QSignalBlocker blocker(frames);
  for (std::size_t row = 0; row < indices.size(); ++row)
  {
    if (indices[row] >= snapshot->increments.size())
    {
      continue;
    }
    const auto& frame = snapshot->increments[indices[row]];
    if (frame.topology_iteration > 0)
    {
      frames->setItemText(static_cast<int>(row),parameter_text(frame));
      continue;
    }
    QString step = tr("unknown");
    QString increment = tr("unknown");
    if (frame.step >= 0)
    {
      step = QString::number(frame.step);
    }
    if (frame.increment >= 0)
    {
      increment = QString::number(frame.increment);
    }
    frames->setItemText(static_cast<int>(row),tr("Frame %1 | Step %2 | Increment %3 | %4")
      .arg(static_cast<qulonglong>(indices[row]+1)).arg(step).arg(increment).arg(parameter_text(frame)));
  }
}

void PostprocessorFrameNavigation::set_rotation_icons(const std::array<QIcon,3>& icons)
{
  for (std::size_t axis = 0; axis < rotations.size(); ++axis)
  {
    rotations[axis]->setIcon(icons[axis]);
  }
}

void PostprocessorFrameNavigation::reset_navigation()
{
  ++generation;
  running = false;
  playback.playing = false;
  animationTimer.stop();
  rotationTimer.stop();
  rotationClock.invalidate();
  abandon_drag();
  baseline.reset();
  expected.reset();
  snapshot.reset();
  indices.clear();
  context.clear();
}

void PostprocessorFrameNavigation::advance_rotation()
{
  if (!running || !rotationStep || !rotationClock.isValid())
  {
    return;
  }
  const auto ownerGeneration = generation;
  const double seconds = static_cast<double>(rotationClock.nsecsElapsed())/1e9;
  rotationClock.restart();
  try
  {
    if (!rotationStep(seconds) && generation == ownerGeneration)
    {
      reset_navigation();
    }
  }
  catch (const std::exception& exception)
  {
    if (QString::fromUtf8(exception.what()).trimmed() == QStringLiteral("Cubit is busy"))
    {
      return;
    }
    reset_navigation();
    PRINT_ERROR("Cannot advance animation rotation: %s\n",exception.what());
  }
  catch (...)
  {
    reset_navigation();
    PRINT_ERROR("Cannot advance animation rotation: unknown failure.\n");
  }
}
