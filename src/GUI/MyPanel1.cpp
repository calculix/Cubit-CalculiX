/*!
 *  \file MyPanel1.cpp
 *  \brief
 *    Provides an example command panel that uses a pickwidget to get select
 *    volumes, finds the centerpoint of each volume, then calculates the average
 *    centerpoint of all the selected volumes.
 *  \author Michael Plooster
 *  \date 17 Feb 2014
 */

#include "MyPanel1.hpp"
#include "ui_MyPanel1.h"

#include <QGridLayout>
#include <vector>

#include "Claro.hpp"
#include "CommandWindow.hpp"
#include "CubitInterface.hpp"
#include "PickWidget.hpp"

MyPanel1::MyPanel1(QWidget *parent) :
  QWidget(parent),
  ui(new Ui::MyPanel1)
{
  ui->setupUi(this);

  // Create the pickwidget and set it's pick type
  pwVolumeIds = new PickWidget();
  pwVolumeIds->setPickType(PickWidget::Volume);

  // Add the pickwidget to our pickwidget container in the ui file. Note
  // that our container needs a Qt Layout first, so we will create one
  // and add our pickwidget to the layout.
  QGridLayout* pickwidget_layout = new QGridLayout(ui->pwContainer);
  pickwidget_layout->addWidget(pwVolumeIds);
}

MyPanel1::~MyPanel1()
{
  delete ui;

  if(pwVolumeIds)
    delete pwVolumeIds;
}

void MyPanel1::on_pushButton_clicked()
{
  // This is just to be cautious. We should have the pickwidget at this point.
  if(!pwVolumeIds)
    return;

  // Get the entity ids contained in our pickwidget. Note that this is not always
  // necessary. For example, if you are sending a Cubit command, just adding the
  // text from the pickwidget will be sufficient in most cases.
  std::string pickwidget_text = pwVolumeIds->text().toUtf8().data();
  std::vector<int> ids =
    CubitInterface::parse_cubit_list("volume", pickwidget_text);

  // Get and average the centerpoints for each volume.
  std::vector<double> coords(3, 0.0);
  for(size_t i = 0; i < ids.size(); i++)
  {
    std::array<double, 3> center = CubitInterface::get_center_point("volume", ids[i]);
    for(size_t j = 0; j < 3; j++)
      coords[j] += center[j];
  }

  double num_vols = (double) ids.size();
  for(size_t j = 0; j < 3; j++)
  {
    coords[j] /= num_vols;
  }

  // Output the result to the command line
  QString result = "\n\nCalculated average centerpoint is at x: %1 y: %2 z %3\n\n";
  result = result.arg(coords[0]).arg(coords[1]).arg(coords[2]);

  CommandWindow* cw = Claro::instance()->command_line();
  cw->print_data(result);
}
