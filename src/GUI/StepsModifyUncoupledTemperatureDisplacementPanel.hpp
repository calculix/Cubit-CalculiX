#ifndef STEPSMODIFYUNCOUPLEDTEMPERATUREDISPLACEMENTPANEL_HPP
#define STEPSMODIFYUNCOUPLEDTEMPERATUREDISPLACEMENTPANEL_HPP

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
#include <QRadioButton>
#include <QCheckBox>

class StepsModifyUncoupledTemperatureDisplacementPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit StepsModifyUncoupledTemperatureDisplacementPanel(QWidget *parent = 0);
  ~StepsModifyUncoupledTemperatureDisplacementPanel();

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
  QHBoxLayout* HBoxLayout_7;
  QHBoxLayout* HBoxLayout_8;
  QHBoxLayout* HBoxLayout_9;
  QHBoxLayout* HBoxLayout_10;
  QHBoxLayout* HBoxLayout_11;
  QHBoxLayout* HBoxLayout_12;
  QHBoxLayout* HBoxLayout_13;
  QHBoxLayout* HBoxLayout_14;
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
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QComboBox* comboBox_3;
  QComboBox* comboBox_4;
  QLineEdit* lineEdit_5;
  QComboBox* comboBox_6;
  QLineEdit* lineEdit_7;
  QComboBox* comboBox_8;
  QComboBox* comboBox_9;
  QLineEdit* lineEdit_10;
  QLineEdit* lineEdit_11;
  QLineEdit* lineEdit_12;
  QLineEdit* lineEdit_13;
  QLineEdit* lineEdit_14;
};

#endif // STEPSMODIFYUNCOUPLEDTEMPERATUREDISPLACEMENTPANEL_HPP