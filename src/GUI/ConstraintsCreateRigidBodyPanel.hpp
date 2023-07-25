#ifndef CONSTRAINTSCREATERIGIDBODYPANEL_HPP
#define CONSTRAINTSCREATERIGIDBODYPANEL_HPP

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

class ConstraintsCreateRigidBodyPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit ConstraintsCreateRigidBodyPanel(QWidget *parent = 0);
  ~ConstraintsCreateRigidBodyPanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);
  void on_radioButton_1_toggled(bool toggled);
  void on_radioButton_2_toggled(bool toggled);

private:
  std::string log;

  QFrame*      frame_1;
  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QVBoxLayout* VBoxLayout_1;
  QSpacerItem* vertical_spacer;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_pushButton_apply;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QHBoxLayout* HBoxLayout_3;
  QHBoxLayout* HBoxLayout_4;
  QRadioButton* radioButton_1;
  QRadioButton* radioButton_2;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
  QLineEdit* lineEdit_4;
};

#endif // CONSTRAINTSCREATERIGIDBODYPANEL_HPP