#ifndef SECTIONSCREATE_HPP
#define SECTIONSCREATE_HPP

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

class SectionsCreate : public QWidget
{
  Q_OBJECT
  
public:
  explicit SectionsCreate(QWidget *parent = 0);
  ~SectionsCreate();

  bool isInitialized;
  
private slots:
  void on_pbApply_clicked();

private:
  std::string log;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QSpacerItem* vertical_spacer;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QHBoxLayout* HBoxLayout_3;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_3;
};

#endif // SECTIONSCREATE_HPP
