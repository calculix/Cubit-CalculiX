/*!
 *  \file MyPanel1.cpp
 *  \brief
 *    Provides an example command panel that uses a pickwidget to get select
 *    volumes, finds the centerpoint of each volume, then calculates the average
 *    centerpoint of all the selected volumes.
 *  \author Michael Plooster
 *  \date 17 Feb 2014
 */

#ifndef MYPANEL1_HPP
#define MYPANEL1_HPP

#include <QWidget>

class PickWidget;

namespace Ui {class MyPanel1;}

class MyPanel1 : public QWidget
{
  Q_OBJECT
  
public:
  MyPanel1(QWidget *parent = 0);
  ~MyPanel1();
  
private slots:
  // Handle the apply button click.
  void on_pushButton_clicked();

private:
  Ui::MyPanel1 *ui; // Reference to the ui
  PickWidget* pwVolumeIds;  // Reference to our pickwidget.
};

#endif // MYPANEL1_HPP
