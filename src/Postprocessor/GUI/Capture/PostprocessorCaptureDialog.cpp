#include "PostprocessorCaptureDialog.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include <QDialog>
#include <QFormLayout>
#include <QComboBox>
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QSignalBlocker>
#include <QMessageBox>
#include <cmath>
#include <algorithm>
QString postprocessor_capture_dialog(QWidget* parent,const PostprocessorApplicationInterface& application,bool report)
{
  using namespace Postprocessor;
  QDialog dialog(parent);
  dialog.setWindowTitle(report ? QObject::tr("Add image to Report") : QObject::tr("Render PNG image"));
  auto* form = new QFormLayout(&dialog);
  auto* target = new QComboBox(&dialog);
  for (const auto& view : application.session().views)
  {
    target->addItem(QObject::tr("View: %1").arg(QString::fromUtf8(view.second.name.c_str())),QString::fromStdString(view.first));
  }
  const int viewCount = target->count();
  for (const auto& plot : application.session().plots)
  {
    target->addItem(QObject::tr("Plot: %1").arg(QString::fromUtf8(plot.second.name.c_str())),QString::fromUtf8(plot.second.name.c_str()));
  }
  form->addRow(QObject::tr("Target"),target);
  auto* width = new QSpinBox(&dialog);
  auto* height = new QSpinBox(&dialog);
  for (auto* size : {width,height})
  {
    size->setRange(CaptureDefaults::minimum_side,CaptureDefaults::maximum_side);
    size->setSuffix(QObject::tr(" px"));
  }
  width->setValue(CaptureDefaults::width);
  height->setValue(CaptureDefaults::height);
  auto* lock = new QCheckBox(QObject::tr("Lock aspect ratio"),&dialog);
  lock->setChecked(true);
  double aspect = static_cast<double>(width->value())/height->value();
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
      const QSignalBlocker block(height);
      height->setValue(static_cast<int>(std::lround(value/aspect)));
    }
  });
  QObject::connect(height,&QSpinBox::valueChanged,&dialog,[&](int value)
  {
    if (lock->isChecked())
    {
      const QSignalBlocker block(width);
      width->setValue(static_cast<int>(std::lround(value*aspect)));
    }
  });
  form->addRow(QObject::tr("Width"),width);
  form->addRow(QObject::tr("Height"),height);
  form->addRow(lock);
  auto* background = new QComboBox(&dialog);
  background->addItems({QObject::tr("Current"),QObject::tr("White"),QObject::tr("Transparent")});
  form->addRow(QObject::tr("Background"),background);
  const auto option = [&](const QString& title)
  {
    auto* combo = new QComboBox(&dialog);
    combo->addItems({QObject::tr("Current"),QObject::tr("On"),QObject::tr("Off")});
    form->addRow(title,combo);
    return combo;
  };
  auto* legend = option(QObject::tr("Legend"));
  auto* labels = option(QObject::tr("Labels"));
  auto* triad = option(QObject::tr("Triad"));
  auto* marker = option(QObject::tr("Frame marker"));
  const auto update = [&]()
  {
    const bool plot = target->currentIndex() >= viewCount;
    labels->setEnabled(!plot);
    triad->setEnabled(!plot);
    marker->setEnabled(plot);
  };
  QObject::connect(target,&QComboBox::currentIndexChanged,&dialog,[&](int)
  {
    update();
  });
  const auto viewIndex = target->findData(QString::fromStdString(application.session().active_view_id));
  if (viewIndex >= 0)
  {
    target->setCurrentIndex(viewIndex);
  }
  const auto activeView = application.session().views.find(application.session().active_view_id);
  if (activeView != application.session().views.end() && activeView->second.plot_active && activeView->second.plot_id)
  {
    const auto plot = application.session().plots.find(*activeView->second.plot_id);
    if (plot != application.session().plots.end())
    {
      const int index = target->findData(QString::fromUtf8(plot->second.name.c_str()));
      if (index >= viewCount)
      {
        target->setCurrentIndex(index);
      }
    }
  }
  update();
  QLineEdit* caption = nullptr;
  if (report)
  {
    caption = new QLineEdit(&dialog);
    form->addRow(QObject::tr("Caption"),caption);
  }
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);
  buttons->button(QDialogButtonBox::Ok)->setText(report ? QObject::tr("Add to Report") : QObject::tr("Save PNG…"));
  form->addRow(buttons);
  QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
  QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]()
  {
    CaptureDefinition request;
    request.width = width->value();
    request.height = height->value();
    std::string error;
    if (target->currentIndex() < 0 || !valid_capture(request,error))
    {
      QMessageBox::warning(&dialog,QObject::tr("Capture image"),QString::fromStdString(error));
      return;
    }
    dialog.accept();
  });
  if (dialog.exec() != QDialog::Accepted)
  {
    return {};
  }
  const bool plot = target->currentIndex() >= viewCount;
  const auto name = target->currentData().toString().toUtf8().toStdString();
  QString command = report ? "report image add postprocessor " : "post capture ";
  command += plot ? "plot " : "view ";
  command += plot_quote(name);
  if (report)
  {
    if (!caption->text().isEmpty())
    {
      command += " caption " + plot_quote(caption->text().toUtf8().toStdString());
    }
  }
  else
  {
    auto file = QFileDialog::getSaveFileName(parent,QObject::tr("Save PNG image"),{},QObject::tr("PNG images (*.png)"),nullptr,QFileDialog::DontConfirmOverwrite);
    if (file.isEmpty())
    {
      return {};
    }
    if (!file.endsWith(".png",Qt::CaseInsensitive))
    {
      file += ".png";
    }
    command += " file " + plot_quote(file.toUtf8().toStdString());
  }
  const char* backgrounds[] = {"current","white","transparent"};
  command += QString(" width %1 height %2 background %3").arg(width->value()).arg(height->value()).arg(backgrounds[background->currentIndex()]);
  const auto flag = [&](const char* key,const QComboBox* control)
  {
    if (control->isEnabled() && control->currentIndex() != 0)
    {
      command += QString(" %1 %2").arg(key,control->currentIndex() == 1 ? "on" : "off");
    }
  };
  flag("legend",legend);
  flag("labels",labels);
  flag("triad",triad);
  flag("marker",marker);
  return command;
}
