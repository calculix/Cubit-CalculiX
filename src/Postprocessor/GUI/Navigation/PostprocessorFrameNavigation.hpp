#ifndef POSTPROCESSOR_FRAMENAVIGATION_HPP
#define POSTPROCESSOR_FRAMENAVIGATION_HPP

#include <QWidget>
#include <QTimer>
#include <functional>
#include <memory>
#include <optional>
#include <vector>
#include <cstddef>
#include "Framework/Results/VtkResultsData.hpp"
#include "Postprocessor/Core/representations/AnimationDefinition.hpp"

class QComboBox;
class QToolButton;
class QSlider;
class QHideEvent;
class QShowEvent;
class QCheckBox;
class QSpinBox;
class PostprocessorFrameNavigation : public QWidget
{
public:
  explicit PostprocessorFrameNavigation(std::function<void(std::size_t)> select, QWidget* parent = nullptr,
    std::function<bool(std::size_t)> preview = {}, std::function<void(const QString&)> animation = {});
  ~PostprocessorFrameNavigation() override;
  void stop_playback(bool commit = true);
  void refresh(const std::shared_ptr<const SimResults::VtkResultsData>& data,
    std::optional<std::size_t> current, const std::function<bool(std::size_t)>& has_mesh, const std::string& context = {}, const Postprocessor::AnimationDefinition& animation = {}, bool allow_animation = true);
  bool matches_context(const std::string& key, const std::shared_ptr<const SimResults::VtkResultsData>& data, std::optional<std::size_t> current) const;
protected:
  void hideEvent(QHideEvent* event) override;
  void showEvent(QShowEvent* event) override;

private:
  void choose(int position);
  void advance_animation();
  void synchronize_animation(const Postprocessor::AnimationDefinition& animation);
  void commit_animation_frame();
  void flush_preview();
  void finish_drag();
  void abandon_drag();
  void update_controls(int position);
  std::function<void(std::size_t)> select;
  std::function<bool(std::size_t)> preview;
  std::function<void(const QString&)> animationCommand;
  Postprocessor::AnimationDefinition playback;
  bool running = false;
  bool playbackAllowed = true;
  QTimer animationTimer;
  QToolButton* play;
  QCheckBox* loop;
  QSpinBox* fps;
  QComboBox* frames;
  QSlider* slider;
  QTimer previewTimer;
  std::shared_ptr<const SimResults::VtkResultsData> snapshot;
  std::string context;
  std::optional<std::size_t> expected;
  std::optional<std::size_t> baseline;
  std::optional<int> pending;
  bool dragging = false;
  QToolButton* first;
  QToolButton* previous;
  QToolButton* next;
  QToolButton* last;
  std::vector<std::size_t> indices;
};
#endif
