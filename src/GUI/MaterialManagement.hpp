#ifndef MATERIALMANAGEMENT_HPP
#define MATERIALMANAGEMENT_HPP

#include <QWidget>
#include <QList>
#include <QPushButton>


class MaterialManagement : public QWidget
{
  Q_OBJECT
  
public:
  explicit MaterialManagement();
  ~MaterialManagement();
  
private slots:
  void on_pushButton_ok_clicked();
  void on_pushButton_apply_clicked();
  void on_pushButton_close_clicked();
  void on_pushButton_new_clicked();
  void on_pushButton_delete_clicked();
  void on_pushButton_add_clicked();
  void on_pushButton_remove_clicked();

private:
  std::string log;
  QPushButton* pushButton_ok;
  QPushButton* pushButton_apply;
  QPushButton* pushButton_close;
  QPushButton* pushButton_new;
  QPushButton* pushButton_delete;
  QPushButton* pushButton_add;
  QPushButton* pushButton_remove;
};

#endif // MATERIALMANAGEMENT_HPP
