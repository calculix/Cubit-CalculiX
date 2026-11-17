#include "PostprocessorAnimationCaptureDialog.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "CubitMessage.hpp"
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QDialogButtonBox>
#include <QPushButton>
#include <QFileDialog>
#include <QSignalBlocker>
#include <QLabel>
#include <QStandardItemModel>
#include <algorithm>
#include <cmath>
#include <limits>
QString postprocessor_animation_capture_dialog(QWidget* parent,const PostprocessorApplicationInterface& application)
{
  QDialog dialog(parent);
  dialog.setWindowTitle(QObject::tr("Render animation"));
  auto* form = new QFormLayout(&dialog);
  auto* view = new QComboBox(&dialog);
  for (const auto& item : application.session().views)
  {
    view->addItem(QString::fromStdString(item.second.name),QString::fromStdString(item.first));
  }
  view->setCurrentIndex(view->findData(QString::fromStdString(application.session().active_view_id)));
  form->addRow(QObject::tr("Target view"),view);
  auto* source = new QComboBox(&dialog);
  for (const auto& item : application.session().sources)
  {
    source->addItem(QString::fromStdString(item.second.name),QString::fromStdString(item.first));
  }
  source->setCurrentIndex(source->findData(QString::fromStdString(application.session().active_source_id)));
  form->addRow(QObject::tr("Animated source"),source);
  auto* format = new QComboBox(&dialog);
  format->addItems({QObject::tr("GIF animation"),QObject::tr("MP4 video (FFmpeg/libx264)"),QObject::tr("PNG sequence")});
  form->addRow(QObject::tr("Format"),format);
  auto* width = new QSpinBox(&dialog);
  auto* height = new QSpinBox(&dialog);
  for (auto* input : {width,height})
  {
    input->setRange(64,8192);
    input->setSuffix(QObject::tr(" px"));
  }
  width->setValue(1920);
  height->setValue(1080);
  form->addRow(QObject::tr("Width"),width);
  form->addRow(QObject::tr("Height"),height);
  auto* lock = new QCheckBox(QObject::tr("Lock aspect ratio"),&dialog);
  lock->setChecked(true);
  form->addRow(lock);
  double aspect = 1920.0/1080.0;
  QObject::connect(lock,&QCheckBox::toggled,&dialog,[&](bool checked)
  {
    if (checked)
    {
      aspect = static_cast<double>(width->value())/height->value();
    }
  });
  QObject::connect(width,&QSpinBox::valueChanged,&dialog,[&](int value)
  {
    if (lock->isChecked())
    {
      const QSignalBlocker guard(height);
      height->setValue(static_cast<int>(std::lround(value/aspect)));
    }
  });
  QObject::connect(height,&QSpinBox::valueChanged,&dialog,[&](int value)
  {
    if (lock->isChecked())
    {
      const QSignalBlocker guard(width);
      width->setValue(static_cast<int>(std::lround(value*aspect)));
    }
  });
  auto* first = new QSpinBox(&dialog);
  auto* last = new QSpinBox(&dialog);
  auto* resultFps = new QSpinBox(&dialog);
  auto* outputFps = new QSpinBox(&dialog);
  auto* passes = new QSpinBox(&dialog);
  first->setRange(1,1);
  last->setRange(1,1);
  resultFps->setRange(1,120);
  outputFps->setRange(1,100);
  outputFps->setValue(30);
  passes->setRange(1,1000);
  form->addRow(QObject::tr("First result frame"),first);
  form->addRow(QObject::tr("Last result frame"),last);
  form->addRow(QObject::tr("Result FPS"),resultFps);
  form->addRow(QObject::tr("Output FPS"),outputFps);
  form->addRow(QObject::tr("Passes"),passes);
  auto* repeat = new QCheckBox(QObject::tr("Loop GIF playback"),&dialog);
  repeat->setChecked(true);
  form->addRow(repeat);
  auto* background = new QComboBox(&dialog);
  background->addItems({QObject::tr("Current"),QObject::tr("White"),QObject::tr("Transparent (PNG only)")});
  form->addRow(QObject::tr("Background"),background);
  auto* legend = new QComboBox(&dialog);
  auto* labels = new QComboBox(&dialog);
  auto* triad = new QComboBox(&dialog);
  for (auto* control : {legend,labels,triad})
  {
    control->addItems({QObject::tr("Current"),QObject::tr("On"),QObject::tr("Off")});
  }
  form->addRow(QObject::tr("Legend"),legend);
  form->addRow(QObject::tr("Labels"),labels);
  form->addRow(QObject::tr("Triad"),triad);
  auto* currentRotation = new QCheckBox(QObject::tr("Use current axis rotation settings"),&dialog);
  currentRotation->setChecked(true);
  form->addRow(currentRotation);
  std::array<QCheckBox*,3> rotate;
  std::array<QDoubleSpinBox*,3> angles;
  const char* axisNames[] = {"X","Y","Z"};
  for (std::size_t axis = 0; axis < rotate.size(); ++axis)
  {
    rotate[axis] = new QCheckBox(QObject::tr("Rotate %1").arg(axisNames[axis]),&dialog);
    angles[axis] = new QDoubleSpinBox(&dialog);
    angles[axis]->setRange(-1000000,1000000);
    angles[axis]->setDecimals(6);
    angles[axis]->setValue(180);
    angles[axis]->setSuffix(QObject::tr(" deg/pass"));
    rotate[axis]->setEnabled(false);
    angles[axis]->setEnabled(false);
    form->addRow(rotate[axis],angles[axis]);
  }
  QObject::connect(currentRotation,&QCheckBox::toggled,&dialog,[&](bool current)
  {
    for (std::size_t axis = 0; axis < rotate.size(); ++axis)
    {
      rotate[axis]->setEnabled(!current);
      angles[axis]->setEnabled(!current);
    }
  });
  form->addRow(new QLabel(QObject::tr("Uses the selected view's camera, display and axis-rotation settings."),&dialog));
  const auto updateSource = [&]()
  {
    const auto data = application.result_data(source->currentData().toString().toStdString());
    int count = 1;
    if (data)
    {
      count = static_cast<int>(std::min<std::size_t>(data->increments.size(),std::numeric_limits<int>::max()));
    }
    first->setRange(1,std::max(1,count));
    last->setRange(1,std::max(1,count));
    first->setValue(1);
    last->setValue(std::max(1,count));
    Postprocessor::AnimationDefinition settings;
    for (const auto& entry : application.session().representations)
    {
      if (entry.second.view_id == view->currentData().toString().toStdString() && entry.second.visible &&
          application.output_source(entry.second.input) == source->currentData().toString().toStdString())
      {
        settings = entry.second.animation;
      }
    }
    resultFps->setValue(settings.frames_per_second);
    repeat->setChecked(settings.loop);
    for (std::size_t axis = 0; axis < rotate.size(); ++axis)
    {
      rotate[axis]->setChecked(settings.rotation[axis].enabled);
      angles[axis]->setValue(settings.rotation[axis].degrees);
    }
  };
  QObject::connect(source,&QComboBox::currentIndexChanged,&dialog,[&](int)
  {
    updateSource();
  });
  QObject::connect(view,&QComboBox::currentIndexChanged,&dialog,[&](int)
  {
    updateSource();
  });
  const auto updateFormat = [&]()
  {
    outputFps->setMaximum(format->currentIndex() == 0 ? 100 : 120);
    repeat->setEnabled(format->currentIndex() == 0);
    const bool transparent = format->currentIndex() == 2;
    if (!transparent && background->currentIndex() == 2)
    {
      background->setCurrentIndex(0);
    }
    if (auto* model = qobject_cast<QStandardItemModel*>(background->model()))
    {
      model->item(2)->setEnabled(transparent);
    }
  };
  QObject::connect(format,&QComboBox::currentIndexChanged,&dialog,[&](int)
  {
    updateFormat();
  });
  updateSource();
  updateFormat();
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Save|QDialogButtonBox::Cancel,&dialog);
  buttons->button(QDialogButtonBox::Save)->setText(QObject::tr("Render..."));
  form->addRow(buttons);
  QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
  QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
  if (dialog.exec() != QDialog::Accepted)
  {
    return {};
  }
  const char* formats[] = {"gif","mp4","png"};
  const char* backgrounds[] = {"current","white","transparent"};
  const auto extension = QString::fromLatin1(formats[format->currentIndex()]);
  QString filter = QObject::tr("New PNG sequence directory (*)");
  if (format->currentIndex() != 2)
  {
    filter = QObject::tr("Animation (*.%1)").arg(extension);
  }
  auto path = QFileDialog::getSaveFileName(parent,QObject::tr("New animation output"),{},filter,nullptr,QFileDialog::DontConfirmOverwrite);
  if (path.isEmpty())
  {
    return {};
  }
  if (format->currentIndex() != 2 && !path.endsWith("."+extension,Qt::CaseInsensitive))
  {
    path += "."+extension;
  }
  const auto quote = [](const QString& text)
  {
    return Postprocessor::plot_quote(text.toUtf8().toStdString());
  };
  const auto viewName = quote(view->currentData().toString());
  const auto sourceName = quote(source->currentText());
  const auto file = quote(path);
  if (viewName.isEmpty() || sourceName.isEmpty() || file.isEmpty())
  {
    PRINT_ERROR("Cannot quote the animation view, source or output path.\n");
    return {};
  }
  QString loop = "off";
  if (repeat->isChecked())
  {
    loop = "on";
  }
  QString command = "post capture animation view "+viewName+" format "+extension+" file "+file+" source "+sourceName+
    QString(" width %1 height %2 first %3 last %4 resultfps %5 fps %6 passes %7 loop %8")
      .arg(width->value()).arg(height->value()).arg(first->value()).arg(last->value()).arg(resultFps->value())
      .arg(outputFps->value()).arg(passes->value()).arg(loop);
  if (!currentRotation->isChecked())
  {
    const char* names[] = {"x","y","z"};
    for (std::size_t axis = 0; axis < rotate.size(); ++axis)
    {
      QString state = "off";
      if (rotate[axis]->isChecked())
      {
        state = "on";
      }
      command += QString(" rotate%1 %2 angle%1 %3").arg(names[axis]).arg(state)
        .arg(QString::number(angles[axis]->value(),'g',17));
    }
  }
  command += " background "+QString::fromLatin1(backgrounds[background->currentIndex()]);
  const auto flag = [&](const char* name,const QComboBox* control)
  {
    if (control->currentIndex() == 1)
    {
      command += QString(" %1 on").arg(name);
    }
    else if (control->currentIndex() == 2)
    {
      command += QString(" %1 off").arg(name);
    }
  };
  flag("legend",legend);
  flag("labels",labels);
  flag("triad",triad);
  return command;
}
