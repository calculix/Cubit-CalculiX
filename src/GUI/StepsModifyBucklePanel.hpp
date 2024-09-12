#ifndef STEPSMODIFYBUCKLEPANEL_HPP
#define STEPSMODIFYBUCKLEPANEL_HPP

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

class StepsModifyBucklePanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit StepsModifyBucklePanel(QWidget *parent = 0);
  ~StepsModifyBucklePanel();

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
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLabel* label_6;
  QLabel* label_7;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QComboBox* comboBox_3;
  QLineEdit* lineEdit_4;
  QLineEdit* lineEdit_5;
  QLineEdit* lineEdit_6;
  QLineEdit* lineEdit_7;
};

#endif // STEPSMODIFYBUCKLEPANEL_HPP