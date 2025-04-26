#ifndef CONSTRAINTSCREATEEQUATIONFROMGROUPPANEL_HPP
#define CONSTRAINTSCREATEEQUATIONFROMGROUPPANEL_HPP

#include <QWidget>
#include <QList>
#include <QPushButton>
#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QLabel>
#include <QSpacerItem>
#include <QTableWidget>
#include <QLineEdit>

class CalculiXCoreInterface;
class PanelTable;

class ConstraintsCreateEquationFromGroupPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit ConstraintsCreateEquationFromGroupPanel(QWidget *parent = 0);
  ~ConstraintsCreateEquationFromGroupPanel();

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
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLabel* label_6;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  QCheckBox* checkBox_4;
  QCheckBox* checkBox_5;
  QCheckBox* checkBox_6;
  
};

#endif // CONSTRAINTSCREATEEQUATIONFROMGROUPPANEL_HPP