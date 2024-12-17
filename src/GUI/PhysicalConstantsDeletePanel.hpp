#ifndef PHYSICALCONSTANTSDELETEPANEL_HPP
#define PHYSICALCONSTANTSDELETEPANEL_HPP

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

class PhysicalConstantsDeletePanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit PhysicalConstantsDeletePanel(QWidget *parent = 0);
  ~PhysicalConstantsDeletePanel();

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
  QLabel* label_1;
  QCheckBox* checkBox_1;
  QLabel* label_2;
  QCheckBox* checkBox_2;
  QLabel* label_3;
  QCheckBox* checkBox_3;
};

#endif // PHYSICALCONSTANTSDELETEPANEL_HPP