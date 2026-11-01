#ifndef HISTORYOUTPUTSMODIFYSECTIONPANEL_HPP
#define HISTORYOUTPUTSMODIFYSECTIONPANEL_HPP
#include <string>
#include <vector>

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
class HistoryOutputsModifySectionPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit HistoryOutputsModifySectionPanel(QWidget *parent = 0);
  ~HistoryOutputsModifySectionPanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  std::string log;
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
  std::vector<QHBoxLayout*> HBoxLayout_keys;
  QLabel* label_0;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  std::vector<QLabel*> label_keys;
  QLineEdit* lineEdit_0;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  std::vector<QCheckBox*> checkBox_keys;
};

#endif // HISTORYOUTPUTSMODIFYSECTIONPANEL_HPP