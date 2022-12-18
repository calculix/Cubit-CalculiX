#ifndef SECTIONSCREATE_HPP
#define SECTIONSCREATE_HPP

#include <QWidget>
#include <QList>

namespace Ui {
class SectionsCreate;
}

class SectionsCreate : public QWidget
{
  Q_OBJECT
  
public:
  explicit SectionsCreate(QWidget *parent = 0);
  ~SectionsCreate();
  
private slots:
  void on_pbApply_clicked();

private:
  Ui::SectionsCreate *ui;
};

#endif // SECTIONSCREATE_HPP
