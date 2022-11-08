#ifndef MYPANEL2_HPP
#define MYPANEL2_HPP

#include <QWidget>

namespace Ui {
class MyPanel2;
}

class MyPanel2 : public QWidget
{
  Q_OBJECT
  
public:
  explicit MyPanel2(QWidget *parent = 0);
  ~MyPanel2();
  
private slots:
  void on_pbApply_clicked();

private:
  Ui::MyPanel2 *ui;
};

#endif // MYPANEL2_HPP
