#ifndef CONTACTPAIRSCREATEFROMCUBITCONTACTPAIRPANEL_HPP
#define CONTACTPAIRSCREATEFROMCUBITCONTACTPAIRPANEL_HPP

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

class CalculiXCoreInterface;

class ContactPairsCreateFromCubitContactPairPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit ContactPairsCreateFromCubitContactPairPanel(QWidget *parent = 0);
  ~ContactPairsCreateFromCubitContactPairPanel();

  bool isInitialized;
  CalculiXCoreInterface *ccx_iface;
  
private slots:
  void on_pushButton_apply_clicked(bool);
  void on_radioButton_3_toggled(bool toggled);
  void on_radioButton_4_toggled(bool toggled);

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
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLineEdit* lineEdit_1;
  QComboBox* comboBox_2;
  QRadioButton* radioButton_3;
  QLineEdit* lineEdit_3;
  QRadioButton* radioButton_4;
  QLineEdit* lineEdit_4;
};

#endif // CONTACTPAIRSCREATEFROMCUBITCONTACTPAIRPANEL_HPP