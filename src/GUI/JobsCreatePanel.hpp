#ifndef JOBSCREATEPANEL_HPP
#define JOBSCREATEPANEL_HPP

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

class JobsCreatePanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit JobsCreatePanel(QWidget *parent = 0);
  ~JobsCreatePanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  std::string log;
  std::vector<std::vector<double>> matrix;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QSpacerItem* vertical_spacer;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_pushButton_apply;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QLabel* label_1;
  QLabel* label_2;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
};

#endif // JOBSCREATEPANEL_HPP