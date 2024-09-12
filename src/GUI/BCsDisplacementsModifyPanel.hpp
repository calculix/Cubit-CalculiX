#ifndef BCSDISPLACEMENTSMODIFYPANEL_HPP
#define BCSDISPLACEMENTSMODIFYPANEL_HPP

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
#include <QCheckBox>
#include <QSpacerItem>
#include <QTableWidget>
#include <QLineEdit>
#include <QRadioButton>

class CalculiXCoreInterface;

class BCsDisplacementsModifyPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit BCsDisplacementsModifyPanel(QWidget *parent = 0);
  ~BCsDisplacementsModifyPanel();

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
  QHBoxLayout* HBoxLayout_0;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QHBoxLayout* HBoxLayout_3;
  QHBoxLayout* HBoxLayout_4;
  QHBoxLayout* HBoxLayout_5;
  QHBoxLayout* HBoxLayout_6;
  QHBoxLayout* HBoxLayout_7;
  QHBoxLayout* HBoxLayout_8;
  QHBoxLayout* HBoxLayout_9;
  QLabel* label_0;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLabel* label_6;
  QLabel* label_7;
  QLabel* label_8;
  QLabel* label_9;
  QLineEdit* lineEdit_0;
  QComboBox* comboBox_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  QCheckBox* checkBox_4;
  QCheckBox* checkBox_5;
  QCheckBox* checkBox_6;
  QCheckBox* checkBox_7;
  QCheckBox* checkBox_8;
  QCheckBox* checkBox_9;
};

#endif // BCSDISPLACEMENTSMODIFYPANEL_HPP