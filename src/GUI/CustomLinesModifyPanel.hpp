#ifndef CUSTOMLINESMODIFYPANEL_HPP
#define CUSTOMLINESMODIFYPANEL_HPP

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
#include <QString>

class CustomLinesModifyPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit CustomLinesModifyPanel(QWidget *parent = 0);
  ~CustomLinesModifyPanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);
  void on_comboBox_index_changed(int index);

private:
  std::string log;
  std::vector<std::vector<double>> matrix;

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
  QHBoxLayout* HBoxLayout_4;
  QHBoxLayout* HBoxLayout_5;
  QLabel* label_0;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLineEdit* lineEdit_0;
  QLineEdit* lineEdit_1;
  QComboBox* comboBox_2;
  QComboBox* comboBox_3;
  QLineEdit* lineEdit_4;
  QLineEdit* lineEdit_5;
};

#endif // CUSTOMLINESMODIFYPANEL_HPP