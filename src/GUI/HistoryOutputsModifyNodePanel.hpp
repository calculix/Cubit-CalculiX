#ifndef HISTORYOUTPUTSMODIFYNODEPANEL_HPP
#define HISTORYOUTPUTSMODIFYNODEPANEL_HPP

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

class HistoryOutputsModifyNodePanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit HistoryOutputsModifyNodePanel(QWidget *parent = 0);
  ~HistoryOutputsModifyNodePanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  std::string log;
  CalculiXCoreInterface *ccx_iface;
  std::vector<std::string> keys;
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
  std::vector<QHBoxLayout*> HBoxLayout_keys;
  QLabel* label_0;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLabel* label_6;
  std::vector<QLabel*> label_keys;
  QLineEdit* lineEdit_0;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  QLineEdit* lineEdit_4;
  QComboBox* comboBox_5;
  QComboBox* comboBox_6;
  std::vector<QCheckBox*> checkBox_keys;
};

#endif // HISTORYOUTPUTSMODIFYNODEPANEL_HPP