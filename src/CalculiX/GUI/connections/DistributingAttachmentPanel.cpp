#include "DistributingAttachmentPanel.hpp"
#include "Core/connections/CalculiXConnectionsInterface.hpp"
#include "GUI/application/GuiSendCmd.hpp"
#include "GUI/properties/PropertyCommandText.hpp"
#include "CubitInterface.hpp"
#include "CubitMessage.hpp"
#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QLabel>
#include <QPushButton>
#include <QSpinBox>
#include <limits>

DistributingAttachmentPanel::DistributingAttachmentPanel(QWidget* parent) : QWidget(parent)
{
  auto* form = new QFormLayout(this);
  auto* id = new QSpinBox(this);
  id->setRange(1,(std::numeric_limits<int>::max)());
  auto existing = CalculiXConnectionsInterface().attachments();
  if (!existing.empty() && existing.back().id < id->maximum())
  {
    id->setValue(existing.back().id+1);
  }
  auto* name = new QLineEdit(this);
  points = new QComboBox(this);
  points->addItem("Create at surface center",0);
  for (const auto& p : CalculiXConnectionsInterface().points())
  {
    points->addItem(QString("%1 / %2").arg(p.id).arg(QString::fromStdString(p.name)),p.id);
  }
  auto* geometry = new QComboBox(this);
  geometry->addItems({"surface","sideset"});
  auto* entity = new QSpinBox(this);
  entity->setRange(1,id->maximum());
  auto* pick = new QPushButton("Use selected geometry",this);
  connect(pick,&QPushButton::clicked,this,[=]()
  {
    const auto selection = CubitInterface::get_selected_entities();
    if (selection.size() == 1)
    {
      const auto index = geometry->findText(QString::fromStdString(selection.front().first).toLower());
      if (index >= 0)
      {
        geometry->setCurrentIndex(index);
        entity->setValue(selection.front().second);
        return;
      }
    }
    PRINT_ERROR("Select one surface or sideset in Cubit.\n");
  });
  form->addRow("ID",id);
  form->addRow("Name",name);
  form->addRow("Reference point",points);
  form->addRow("Attach to",geometry);
  form->addRow("Entity ID",entity);
  form->addRow(pick);
  form->addRow("Transfer forces",new QLabel("FX, FY, FZ",this));
  auto* row = new QHBoxLayout();
  std::array<QCheckBox*,3> moment;
  for (int i=0; i<3; ++i)
  {
    moment[i] = new QCheckBox(QString("M")+QChar("XYZ"[i]),this);
    moment[i]->setChecked(true);
    row->addWidget(moment[i]);
  }
  form->addRow("Transfer moments",row);
  auto* resultant = new QCheckBox("Surface force and moment",this);
  resultant->setChecked(true);
  auto* mean = new QCheckBox("Mean surface displacement",this);
  form->addRow("History output",resultant);
  form->addRow(mean);
  auto* create = new QPushButton("Create",this);
  form->addRow(create);
  connect(geometry,&QComboBox::currentTextChanged,this,[=](const QString& type)
  {
    const bool surface = type == "surface";
    if (!surface && points->findData(0) >= 0)
    {
      points->removeItem(points->findData(0));
    }
    else if (surface && points->findData(0) < 0)
    {
      points->insertItem(0,"Create at surface center",0);
      if (points->currentIndex() < 0)
      {
        points->setCurrentIndex(0);
      }
    }
    create->setEnabled(points->count() > 0);
    create->setToolTip(points->count() > 0 ? QString() : QString("Create a reference point for this sideset first."));
  });
  connect(create,&QPushButton::clicked,this,[=]()
  {
    QString quoted,error;
    if (!PropertyCommandText::quoteName(name->text(),quoted,error))
    {
      PRINT_ERROR("%s\n",error.toUtf8().constData());
      return;
    }
    const int point = points->currentData().toInt();
    int mask = 0;
    for (int i=0; i<3; ++i)
    {
      if (moment[i]->isChecked())
      {
        mask |= 1 << i;
      }
    }
    QString command = QString("ccx create distributing attachment %1 name %2 %3 %4 %5 moments %6")
      .arg(id->value()).arg(quoted).arg(geometry->currentText()).arg(entity->value())
      .arg(point ? "rp "+QString::number(point) : QString("center")).arg(mask);
    if (!resultant->isChecked())
    {
      command += " no_resultant";
    }
    if (mean->isChecked())
    {
      command += " mean_displacement";
    }
    if (GuiSendCmd::gui_cmd(command))
    {
      id->stepUp();
    }
  });
}

void DistributingAttachmentPanel::selectPoint(int point)
{
  points->setCurrentIndex(points->findData(point));
}
