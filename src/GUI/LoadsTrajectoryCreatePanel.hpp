#ifndef LOADSTRAJECTORYCREATEPANEL_HPP
#define LOADSTRAJECTORYCREATEPANEL_HPP

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

class CalculiXCoreInterface;
class PickWidget;
class PanelTable;

class LoadsTrajectoryCreatePanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit LoadsTrajectoryCreatePanel(QWidget *parent = 0);
  ~LoadsTrajectoryCreatePanel();

  bool isInitialized;
  CalculiXCoreInterface *ccx_iface;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  std::string log;
  std::vector<std::vector<double>> matrix;

  QFrame*      frame_10;
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
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLabel* label_6;
  QLabel* label_7;
  QLabel* label_8;
  QLabel* label_9;
  QLineEdit* lineEdit_1;
  QComboBox* comboBox_2;
  PickWidget* PickWidget_3;
  PickWidget* PickWidget_4;
  PickWidget* PickWidget_5;
  QLineEdit* lineEdit_6;
  QLineEdit* lineEdit_7;
  QLineEdit* lineEdit_8;
  QComboBox* comboBox_9;
  PanelTable* widget_10;
};

#endif // LOADSTRAJECTORYCREATEPANEL_HPP