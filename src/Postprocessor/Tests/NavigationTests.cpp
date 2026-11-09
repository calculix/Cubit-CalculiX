#include "Postprocessor/GUI/ResultsTree/PostprocessorResultsTree.hpp"
#include "Postprocessor/GUI/Navigation/PostprocessorFrameNavigation.hpp"
#include <QApplication>
#include <QComboBox>
#include <QSlider>
#include <QCheckBox>
#include <QSpinBox>
#include <QToolButton>
#include <QTimer>
#include <iostream>

namespace
{
  int failures = 0;
  void check(bool value, const char* message)
  {
    if (!value)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  }
}

int main(int argc, char** argv)
{
  QApplication app(argc, argv);
  auto data = std::make_shared<SimResults::VtkResultsData>();
  data->increments.resize(3);
  data->increments[0].step = 1;
  data->increments[0].increment = 1;
  data->increments[2].step = 2;
  data->increments[2].increment = 1;
  data->increments[2].time = 0.3;
  SimResults::Field field;
  field.name = "Displacement";
  field.components = {"X", "Y", "Z"};
  data->increments[0].fields.push_back(field);
  SimResults::Dataset dataset;
  dataset.name = "Solid";
  dataset.id = 7; // Deliberately differs from metadata vector position zero.
  data->datasets.push_back(dataset);
  Postprocessor::PostprocessorSession session;
  Postprocessor::DataSource source;
  source.id = "s1";
  source.name = "Mesh";
  session.sources.emplace(source.id, source);
  session.active_source_id = source.id;
  Postprocessor::Representation representation;
  representation.id = "r1";
  representation.input = {Postprocessor::OutputOwner::Source, source.id, 0};
  representation.view_id = "view1";
  session.representations.emplace(representation.id, representation);
  int selects = 0;
  int visibility = 0;
  int frames = 0;
  std::size_t chosen = 0;
  // Repeat the same physical result across frames and mesh datasets.
  field.array_name = "FRD/DISP";
  data->increments[0].fields = {field};
  field.dataset_id = 1;
  data->increments[0].fields.push_back(field);
  data->increments[2].fields = data->increments[0].fields;
  PostprocessorResultsTree tree([&](const QString& name)
  {
    check(name == "Mesh","Tree used a display label instead of the source name");
    ++selects;
  },[&](const QString& name,bool)
  {
    check(name == "Mesh","Visibility used the wrong source");
    ++visibility;
  });
  int datasetChanges = 0;
  std::optional<std::size_t> datasetId;
  Postprocessor::DatasetVisibilityAction datasetAction = Postprocessor::DatasetVisibilityAction::Show;
  tree.dataset_action = [&](const QString& name,Postprocessor::DatasetVisibilityAction action,std::optional<std::size_t> id)
  {
    check(name == "Mesh","Dataset visibility used the wrong source name");
    ++datasetChanges;
    datasetId = id;
    datasetAction = action;
  };
  auto refresh = [&]()
  {
    tree.refresh(session, [&](const std::string&)
    {
      return data;
    }, [](const std::string&, std::size_t index)
    {
      return index != 1;
    });
  };
  refresh();
  check(selects == 0 && frames == 0 && visibility == 0, "Tree refresh emitted commands");
  check(datasetChanges == 0,"Tree refresh emitted dataset visibility commands");
  auto* root = tree.topLevelItem(0);
  check(root != nullptr && root->childCount() == 2,"Tree must contain Mesh and Results, without a Frames branch");
  if (root != nullptr && root->childCount() == 2)
  {
    auto* results = root->child(1);
    check(results->text(0) == "Results" && results->childCount() == 1,"Repeated frames/datasets created duplicate result items");
    results->setExpanded(true);
    tree.setCurrentItem(results->child(0));
    check(selects == 1 && frames == 0,"Selecting a result should only select its source");
    session.representations.at("r1").increment_index = 2;
    refresh();
    check(selects == 1 && tree.topLevelItem(0)->child(1)->isExpanded() &&
      tree.currentItem() == tree.topLevelItem(0)->child(1)->child(0),"Frame changes lost stable result selection or expansion");
    session.representations.at("r1").increment_index = 1;
    refresh();
    check(!tree.topLevelItem(0)->child(1)->child(0)->flags().testFlag(Qt::ItemIsEnabled),
      "An unavailable result must remain in the tree, disabled");
    tree.topLevelItem(0)->setCheckState(0,Qt::Unchecked);
    check(visibility == 1,"Source checkbox did not emit visibility");
  }
  auto* datasetItem = tree.topLevelItem(0)->child(0)->child(0);
  check(datasetItem->flags().testFlag(Qt::ItemIsUserCheckable),"Dataset row has no checkbox");
  datasetItem->setCheckState(0,Qt::Unchecked);
  check(datasetChanges == 1 && datasetId == 7 && datasetAction == Postprocessor::DatasetVisibilityAction::Hide,
    "Dataset checkbox used metadata vector position rather than published ID");
  session.representations.at("r1").hidden_datasets.insert(7);
  session.representations.at("r1").visible = false;
  refresh();
  check(datasetChanges == 1 && tree.topLevelItem(0)->child(0)->child(0)->checkState(0) == Qt::Unchecked,
    "Refresh lost dataset preference or emitted a command");
  datasetItem = tree.topLevelItem(0)->child(0)->child(0);
  datasetItem->setCheckState(0,Qt::Checked);
  check(datasetChanges == 2 && datasetId == 7 && datasetAction == Postprocessor::DatasetVisibilityAction::Show,
    "Dataset show checkbox did not submit an explicit dataset operation");
  int previews = 0;
  PostprocessorFrameNavigation navigation([&](std::size_t frame)
  {
    chosen = frame;
    ++frames;
  },nullptr,[&](std::size_t frame)
  {
    chosen = frame;
    ++previews;
    return true;
  });
  const auto mesh = [](std::size_t index)
  {
    return index != 1;
  };
  const int before = frames;
  navigation.refresh(data,0,mesh,"s1:view1");
  auto* combo = navigation.findChild<QComboBox*>("Postprocessor.Frames");
  auto* slider = navigation.findChild<QSlider*>("Postprocessor.FrameSlider");
  check(combo && slider && combo->count() == 2 && frames == before,"Navigation refresh must skip gaps and emit no commands");
  if (combo && slider)
  {
    QMetaObject::invokeMethod(combo,"activated",Qt::DirectConnection,Q_ARG(int,1));
    check(chosen == 3 && frames == before+1,"Dropdown renumbered a frame across a gap");
    navigation.refresh(data,0,mesh,"s1:view1");
    slider->setSliderDown(true);
    slider->setValue(1);
    slider->setValue(0);
    slider->setValue(1);
    QCoreApplication::processEvents();
    check(previews == 1 && chosen == 3 && frames == before+1,"Slider previews must coalesce without journal commits");
    navigation.refresh(data,2,mesh,"s1:view1");
    check(slider->isSliderDown(),"The preview's own refresh cancelled its drag");
    slider->setSliderDown(false);
    check(frames == before+2 && chosen == 3,"Slider release must commit one absolute frame");
    slider->setSliderDown(true);
    slider->setValue(0);
    navigation.refresh({},std::nullopt,{},"");
    QCoreApplication::processEvents();
    check(frames == before+2 && !slider->isEnabled() && combo->count() == 0,"Reset retained a pending slider preview or commit");
  }
  Postprocessor::AnimationDefinition playback;
  std::size_t shown = 0;
  int commits = 0;
  int ticks = 0;
  PostprocessorFrameNavigation* animatedPage = nullptr;
  PostprocessorFrameNavigation animated([&](std::size_t frame)
  {
    shown = frame-1;
    ++commits;
  },nullptr,[&](std::size_t frame)
  {
    shown = frame-1;
    ++ticks;
    return true;
  },[&](const QString& operation)
  {
    if (operation == "play")
    {
      playback.playing = true;
      if (shown == 2)
      {
        shown = 0;
      }
    }
    else if (operation == "pause")
    {
      playback.playing = false;
    }
    else if (operation.startsWith("loop "))
    {
      playback.loop = operation == "loop on";
    }
    else if (operation.startsWith("fps "))
    {
      playback.frames_per_second = operation.mid(4).toInt();
    }
    animatedPage->refresh(data,shown,mesh,"animation",playback);
  });
  animatedPage = &animated;
  animated.refresh(data,shown,mesh,"animation",playback);
  animated.show();
  auto* playButton = animated.findChild<QToolButton*>("Postprocessor.Animation.Play");
  auto* loopBox = animated.findChild<QCheckBox*>("Postprocessor.Animation.Loop");
  auto* fpsBox = animated.findChild<QSpinBox*>("Postprocessor.Animation.Fps");
  auto* timer = animated.findChild<QTimer*>("Postprocessor.Animation.Timer");
  check(playButton && loopBox && fpsBox && timer,"Animation controls are missing");
  if (playButton && loopBox && fpsBox && timer)
  {
    playButton->click();
    check(playback.playing && timer->isActive(),"Play did not start the animation timer");
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
    animated.refresh(data,shown,mesh,"animation",playback);
    check(shown == 2 && ticks == 1 && commits == 0,"Animation must skip unavailable frames and avoid per-frame journal commits");
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
    QCoreApplication::processEvents();
    check(!playback.playing && !timer->isActive() && commits == 1,"Non-loop animation did not stop/commit at the last frame");
    loopBox->setChecked(true);
    playButton->click();
    fpsBox->setValue(5);
    check(shown == 0 && playback.frames_per_second == 5 && timer->interval() == 200,"Restart at last frame or live FPS update failed");
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
    animated.refresh(data,shown,mesh,"animation",playback);
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
    animated.refresh(data,shown,mesh,"animation",playback);
    check(shown == 0 && playback.playing && commits == 1,"Loop did not wrap silently to the first frame");
    loopBox->setChecked(false);
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
    animated.refresh(data,shown,mesh,"animation",playback);
    QMetaObject::invokeMethod(timer,"timeout",Qt::DirectConnection);
    QCoreApplication::processEvents();
    check(!playback.playing && commits == 2,"Disabling Loop during playback did not stop at the end");
    animated.refresh({},std::nullopt,{},"",{});
    check(!timer->isActive() && !playButton->isEnabled(),"Reset retained an active animation timer");
  }
  animated.hide();
  session = Postprocessor::PostprocessorSession{};
  refresh();
  check(tree.topLevelItemCount() == 0, "Reset left stale tree items");
  return failures == 0 ? 0 : 1;
}
