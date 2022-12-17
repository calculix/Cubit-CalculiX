#ifndef MATERIALSCCXCREATE_HPP
#define MATERIALSCCXCREATE_HPP

#include <QWidget>
#include <QList>
#include <QRadioButton>
#include <QButtonGroup>


namespace Ui {
class MaterialsCCXcreate;
}

class MaterialsCCXcreate : public QWidget
{
  Q_OBJECT
  
public:
  explicit MaterialsCCXcreate(QWidget *parent = 0);
  ~MaterialsCCXcreate();
  
private slots:
  void on_pbApply_clicked();

private:
  Ui::MaterialsCCXcreate *ui;
};

#endif // MATERIALSCCXCREATEPANEL_HPP
