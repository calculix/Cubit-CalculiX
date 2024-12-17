#ifndef DAMPINGMODIFYPANEL_HPP
#define DAMPINGMODIFYPANEL_HPP

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

class DampingModifyPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit DampingModifyPanel(QWidget *parent = 0);
  ~DampingModifyPanel();

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
  QLabel* label_0;
  QLabel* label_1;
  QLineEdit* lineEdit_0;
  QLineEdit* lineEdit_1;
};

#endif // DAMPINGMODIFYPANEL_HPP