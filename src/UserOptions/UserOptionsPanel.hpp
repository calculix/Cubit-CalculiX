#ifndef USEROPTIONSPANEL_HPP
#define USEROPTIONSPANEL_HPP

#include <QWidget>
#include <QFrame>
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

class UserOptionsPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit UserOptionsPanel(QWidget *parent = 0);
  ~UserOptionsPanel();

  bool isInitialized;
  
public slots:
  void refresh_settings();
  void save_settings();

private:
  std::string log;
  
  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QVBoxLayout* VBoxLayout_1;
  QVBoxLayout* VBoxLayout_2;
  QVBoxLayout* VBoxLayout_3;
  QVBoxLayout* VBoxLayout_4;
  QVBoxLayout* VBoxLayout_5;
  QSpacerItem* vertical_spacer;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QHBoxLayout* HBoxLayout_3;
  QHBoxLayout* HBoxLayout_4;
  QHBoxLayout* HBoxLayout_5;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  QLineEdit* lineEdit_4;
  QLineEdit* lineEdit_5;
};

#endif // USEROPTIONSPANEL_HPP