#ifndef STEPSMODIFYHEATTRANSFERPANEL_HPP
#define STEPSMODIFYHEATTRANSFERPANEL_HPP

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

class CalculiXCoreInterface;

class StepsModifyHeatTransferPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit StepsModifyHeatTransferPanel(QWidget *parent = 0);
  ~StepsModifyHeatTransferPanel();

  bool isInitialized;
  CalculiXCoreInterface *ccx_iface;
  
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
  QHBoxLayout* HBoxLayout_15;
  QHBoxLayout* HBoxLayout_16;
  QHBoxLayout* HBoxLayout_17;
  QHBoxLayout* HBoxLayout_18;
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
  QLabel* label_15;
  QLabel* label_16;
  QLabel* label_17;
  QLabel* label_18;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QComboBox* comboBox_3;
  QComboBox* comboBox_4;
  QComboBox* comboBox_5;
  QComboBox* comboBox_6;
  QComboBox* comboBox_7;
  QComboBox* comboBox_8;
  QLineEdit* lineEdit_9;
  QComboBox* comboBox_10;
  QLineEdit* lineEdit_11;
  QLineEdit* lineEdit_12;
  QLineEdit* lineEdit_13;
  QLineEdit* lineEdit_14;
  QLineEdit* lineEdit_15;
  QLineEdit* lineEdit_16;
  QLineEdit* lineEdit_17;
  QLineEdit* lineEdit_18;
};

#endif // STEPSMODIFYHEATTRANSFERPANEL_HPP