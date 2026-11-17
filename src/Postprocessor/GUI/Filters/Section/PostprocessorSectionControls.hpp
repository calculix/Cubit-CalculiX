#pragma once
#include "Postprocessor/Core/filters/FilterDefinition.hpp"
#include <QWidget>
#include <memory>
#include <functional>
class PostprocessorApplicationInterface;
class QComboBox;
class QLineEdit;
class QLabel;
class QCheckBox;
class QSlider;
class QTimer;
class QHideEvent;
class PostprocessorSectionControls : public QWidget
{
public:
  PostprocessorSectionControls(std::weak_ptr<PostprocessorApplicationInterface> application,std::function<void(const QString&)> submit,QWidget* parent,
    std::function<void(const QString&)> silent = {});
  ~PostprocessorSectionControls() override;
  void refresh();
protected:
  bool eventFilter(QObject* watched,QEvent* event) override;
  void hideEvent(QHideEvent* event) override;
private:
  bool begin_motion();
  bool owns_motion() const;
  void send_motion(bool journal);
  void cancel_motion();
  void submit_plane(bool create);
  void send(const QString& command);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  std::function<void(const QString&)> submit;
  std::function<void(const QString&)> silent;
  QSlider* position;
  QLabel* positionValue;
  QTimer* motionTimer;
  QCheckBox* enabled;
  std::optional<Postprocessor::FilterDefinition> motionStart;
  Postprocessor::PlaneDefinition motionTarget;
  std::array<double,2> motionRange{};
  std::string motionView;
  std::string positionUnit;
  std::uint64_t motionRevision = 0;
  bool motionPreviewed = false;
  bool motionDispatching = false;
  QComboBox* filters;
  QComboBox* side;
  QLineEdit* name;
  QLineEdit* coordinates[6];
  std::array<QLabel*,3> originLabels;
  QCheckBox* visible;
  std::string lastActive;
  bool loading = false;
};
