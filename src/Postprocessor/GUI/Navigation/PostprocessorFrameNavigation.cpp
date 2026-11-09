#include "PostprocessorFrameNavigation.hpp"
#include <QComboBox>
#include <QHBoxLayout>
#include <QSizePolicy>
#include <QSlider>
#include <QHideEvent>
#include <QShowEvent>
#include <QCheckBox>
#include <QSpinBox>
#include <QAbstractSlider>
#include <QSignalBlocker>
#include <QStyle>
#include <QToolButton>
#include <cmath>
#include <algorithm>
#include <utility>

PostprocessorFrameNavigation::PostprocessorFrameNavigation(std::function<void(std::size_t)> callback, QWidget* parent,
  std::function<bool(std::size_t)> previewCallback, std::function<void(const QString&)> animationCallback)
  : QWidget(parent), select(std::move(callback)), preview(std::move(previewCallback)), animationCommand(std::move(animationCallback))
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
  loop = new QCheckBox(tr("Loop"),this);
  loop->setObjectName("Postprocessor.Animation.Loop");
  loop->setChecked(Postprocessor::AnimationDefaults::loop);
  loop->setToolTip(tr("Restart at the first displayable frame after the last frame"));
  layout->addWidget(loop);
  animationTimer.setParent(this);
  animationTimer.setObjectName("Postprocessor.Animation.Timer");
  animationTimer.setTimerType(Qt::PreciseTimer);
  animationTimer.setSingleShot(true);
  connect(&animationTimer,&QTimer::timeout,this,[this]()
  {
    advance_animation();
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
  connect(loop,&QCheckBox::toggled,this,[this](bool checked)
  {
    if (animationCommand)
    {
      animationCommand(checked ? QStringLiteral("loop on") : QStringLiteral("loop off"));
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
    stop_playback(false);
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
    stop_playback(false);
    select(indices[static_cast<std::size_t>(position)] + 1);
  }
}

void PostprocessorFrameNavigation::refresh(const std::shared_ptr<const SimResults::VtkResultsData>& data,
  std::optional<std::size_t> current, const std::function<bool(std::size_t)>& has_mesh, const std::string& key,
  const Postprocessor::AnimationDefinition& animation)
{
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
  animationTimer.stop();
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
      frames->addItem(tr("Frame %1 | Step %2 | Increment %3 | Time %4")
        .arg(static_cast<qulonglong>(i + 1))
        .arg(value.step < 0 ? tr("unknown") : QString::number(value.step))
        .arg(value.increment < 0 ? tr("unknown") : QString::number(value.increment))
        .arg(std::isfinite(value.time) ? QString::number(value.time, 'g', 16) : tr("unknown")));
    }
  }
  slider->setRange(0,std::max(0,static_cast<int>(indices.size())-1));
  slider->setValue(selected < 0 ? 0 : selected);
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
    QTimer::singleShot(0,this,[this,frame,source,owner]()
    {
      if (!running && expected == frame && snapshot == source && context == owner && animationCommand)
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
  frames->setToolTip(selected >= 0 ? frames->itemText(selected) : tr("Result frame of the active source in the active view"));
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
  if (preview && !preview(frame+1))
  {
    abandon_drag();
    return;
  }
  expected = frame;
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
  stop_playback();
  const auto restore = baseline;
  const bool changed = dragging && expected != baseline;
  abandon_drag();
  // A workspace switch cancels a drag rather than retaining an unjournaled
  // final preview. The owner callback guards source/view/snapshot identity.
  if (changed && restore && preview && preview(*restore+1))
  {
    expected = restore;
  }
  QWidget::hideEvent(event);
}

PostprocessorFrameNavigation::~PostprocessorFrameNavigation()
{
  animationTimer.stop();
  previewTimer.stop();
  animationCommand = {};
  preview = {};
}

void PostprocessorFrameNavigation::showEvent(QShowEvent* event)
{
  QWidget::showEvent(event);
  synchronize_animation(playback);
}

void PostprocessorFrameNavigation::synchronize_animation(const Postprocessor::AnimationDefinition& animation)
{
  const int previousRate = playback.frames_per_second;
  playback = animation;
  const QSignalBlocker loopBlocker(loop);
  const QSignalBlocker fpsBlocker(fps);
  loop->setChecked(playback.loop);
  fps->setValue(playback.frames_per_second);
  const bool canPlay = indices.size() > 1 && expected.has_value() && static_cast<bool>(animationCommand) && static_cast<bool>(preview);
  running = playback.playing && canPlay && isVisible();
  if (!running)
  {
    animationTimer.stop();
  }
  else if (!animationTimer.isActive() || previousRate != playback.frames_per_second)
  {
    animationTimer.start(static_cast<int>(std::ceil(1000.0/playback.frames_per_second)));
  }
  play->setIcon(style()->standardIcon(running ? QStyle::SP_MediaPause : QStyle::SP_MediaPlay));
  play->setToolTip(running ? tr("Pause result animation") : tr("Play result animation"));
  play->setEnabled(canPlay);
  fps->setEnabled(!indices.empty() && static_cast<bool>(animationCommand));
  loop->setEnabled(!indices.empty() && static_cast<bool>(animationCommand));
}

void PostprocessorFrameNavigation::commit_animation_frame()
{
  const auto frame = expected;
  const auto data = snapshot;
  const auto key = context;
  QTimer::singleShot(0,this,[this,frame,data,key]()
  {
    if (frame && expected == frame && snapshot == data && context == key && !running && select)
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
  running = false;
  playback.playing = false;
  animationTimer.stop();
  if (animationCommand)
  {
    animationCommand(QStringLiteral("pause"));
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
