#ifndef HISTORYOUTPUTSCREATEPANEL_HPP
#define HISTORYOUTPUTSCREATEPANEL_HPP

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

class PanelTable;

class HistoryOutputsCreatePanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit HistoryOutputsCreatePanel(QWidget *parent = 0);
  ~HistoryOutputsCreatePanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  std::string log;
  std::vector<std::vector<double>> matrix;

  QFrame*      frame_5;
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
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  QLineEdit* lineEdit_4;
  QCheckBox* checkBox_4;
  PanelTable* widget_5;
};

#endif // HISTORYOUTPUTSCREATEPANEL_HPP