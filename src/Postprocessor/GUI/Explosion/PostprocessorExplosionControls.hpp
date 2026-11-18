#pragma once
#include <QWidget>
#include <functional>
#include <memory>
#include <array>
class PostprocessorApplicationInterface;
class QCheckBox;
class QComboBox;
class QDoubleSpinBox;
class QSlider;
class QTimer;
class QLabel;
class PostprocessorExplosionControls final : public QWidget
{
public:
  using Dispatch = std::function<void(const QString&)>;
  PostprocessorExplosionControls(std::weak_ptr<PostprocessorApplicationInterface> app,Dispatch submit,
    Dispatch preview,QWidget* parent = nullptr);
  void refresh();
  void discard_pending();
private:
  QString scope() const;
  void amount_changed(bool mesh,bool final);
  void pulse_command(const QString& operation);
  std::weak_ptr<PostprocessorApplicationInterface> application;
  Dispatch submit, preview;
  bool refreshing = false;
  QCheckBox* components;
  QCheckBox* mesh;
  QCheckBox* pulse;
  QComboBox* pulseTarget;
  QDoubleSpinBox* pulsePeriod;
  QDoubleSpinBox* pulseMinimum;
  QDoubleSpinBox* pulseMaximum;
  QComboBox* sources;
  QComboBox* blocks;
  QComboBox* grouping;
  QComboBox* direction;
  QDoubleSpinBox* amount;
  QDoubleSpinBox* meshAmount;
  QDoubleSpinBox* gap;
  QSlider* componentSlider;
  QSlider* meshSlider;
  QTimer* previewTimer;
  QString pending;
  QString motionScope;
  QLabel* offsetUnit;
  std::array<QDoubleSpinBox*,3> offsets;
};
