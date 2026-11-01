#ifndef LOADSHEATFLUXESCREATEPANEL_HPP
#define LOADSHEATFLUXESCREATEPANEL_HPP

#include <string>

#include <QWidget>
#include <QPushButton>
#include <QObject>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QSpacerItem>
#include <QLineEdit>

class LoadsHeatfluxesCreatePanel : public QWidget
{
  Q_OBJECT

public:
  explicit LoadsHeatfluxesCreatePanel(QWidget *parent = 0);
  ~LoadsHeatfluxesCreatePanel();

  bool isInitialized;

private slots:
  void on_pushButton_apply_clicked(bool);

private:
  std::string log;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QSpacerItem* vertical_spacer;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_pushButton_apply;

  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QHBoxLayout* HBoxLayout_3;
  QHBoxLayout* HBoxLayout_4;
  QHBoxLayout* HBoxLayout_5;
  QHBoxLayout* HBoxLayout_6;

  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLabel* label_6;

  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  QComboBox* comboBox_4;
  QLineEdit* lineEdit_5;
  QLineEdit* lineEdit_6;
};

#endif // LOADSHEATFLUXESCREATEPANEL_HPP
