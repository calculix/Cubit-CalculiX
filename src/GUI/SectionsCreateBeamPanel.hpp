#ifndef SECTIONSCREATEBEAMPANEL_HPP
#define SECTIONSCREATEBEAMPANEL_HPP

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QSpacerItem>
#include <QTableWidget>
#include <QLineEdit>

class SectionsCreateBeamPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit SectionsCreateBeamPanel(QWidget *parent = 0);
  ~SectionsCreateBeamPanel();

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
  QHBoxLayout* HBoxLayout_beam_type;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QHBoxLayout* HBoxLayout_3;
  QHBoxLayout* HBoxLayout_4;
  QHBoxLayout* HBoxLayout_5;
  QHBoxLayout* HBoxLayout_6;
  QHBoxLayout* HBoxLayout_7;
  QHBoxLayout* HBoxLayout_8;
  QHBoxLayout* HBoxLayout_9;
  QHBoxLayout* HBoxLayout_10;
  QHBoxLayout* HBoxLayout_11;
  QHBoxLayout* HBoxLayout_12;
  QHBoxLayout* HBoxLayout_13;
  QHBoxLayout* HBoxLayout_14;
  QLabel* label_beam_type;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLabel* label_6;
  QLabel* label_7;
  QLabel* label_8;
  QLabel* label_9;
  QLabel* label_10;
  QLabel* label_11;
  QLabel* label_12;
  QLabel* label_13;
  QLabel* label_14;
  QComboBox* beam_type;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  QLineEdit* lineEdit_4;
  QLineEdit* lineEdit_5;
  QLineEdit* lineEdit_6;
  QLineEdit* lineEdit_7;
  QLineEdit* lineEdit_8;
  QLineEdit* lineEdit_9;
  QLineEdit* lineEdit_10;
  QLineEdit* lineEdit_11;
  QLineEdit* lineEdit_12;
  QLineEdit* lineEdit_13;
  QLineEdit* lineEdit_14;  
};

#endif // SECTIONSCREATEBEAMPANEL_HPP
