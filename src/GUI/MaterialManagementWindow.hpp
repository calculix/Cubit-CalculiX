#ifndef MATERIALMANAGEMENTWINDOW_HPP
#define MATERIALMANAGEMENTWINDOW_HPP

#include <QWidget>
#include <QList>

namespace Ui {
class MaterialManagement;
}

class MaterialManagement : public QWidget
{
  Q_OBJECT
  
public:
  explicit MaterialManagement(QWidget *parent = 0);
  ~MaterialManagement();
  
private slots:
  void on_pbApply_clicked();

private:
  Ui::MaterialManagement *ui;
};

#endif // MATERIALMANAGEMENTWINDOW_HPP
