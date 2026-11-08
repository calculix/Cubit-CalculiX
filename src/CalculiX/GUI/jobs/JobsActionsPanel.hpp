#ifndef JOBSACTIONSPANEL_HPP
#define JOBSACTIONSPANEL_HPP
#include <string>

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
class JobsActionsPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit JobsActionsPanel(QWidget *parent = 0);
  ~JobsActionsPanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_run_clicked(bool);
  void on_pushButton_kill_clicked(bool);
  void on_pushButton_result_load_clicked(bool);
  void on_pushButton_results_clicked(bool);

private:
  std::string log;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QSpacerItem* vertical_spacer;
  QPushButton* pushButton_run;
  QHBoxLayout* HBoxLayout_pushButton_run;
  QSpacerItem* horizontal_spacer_pushButton_run;
  QPushButton* pushButton_kill;
  QHBoxLayout* HBoxLayout_pushButton_kill;
  QSpacerItem* horizontal_spacer_pushButton_kill;
  QPushButton* pushButton_result_load;
  QHBoxLayout* HBoxLayout_pushButton_result_load;
  QSpacerItem* horizontal_spacer_pushButton_result_load;
  QPushButton* pushButton_results;
  QHBoxLayout* HBoxLayout_pushButton_results;
  QSpacerItem* horizontal_spacer_pushButton_results;
  QHBoxLayout* HBoxLayout_1;
  QLabel* label_1;
  QLineEdit* lineEdit_1;
};

#endif // JOBSACTIONSPANEL_HPP