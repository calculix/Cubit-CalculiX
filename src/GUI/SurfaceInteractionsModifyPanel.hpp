#ifndef SURFACEINTERACTIONMODIFYPANEL_HPP
#define SURFACEINTERACTIONMODIFYPANEL_HPP

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

class PanelTable;

class SurfaceInteractionsModifyPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit SurfaceInteractionsModifyPanel(QWidget *parent = 0);
  ~SurfaceInteractionsModifyPanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);
  void comboBox_currentIndexChanged(int index);

private:
  std::string log;
  std::vector<std::vector<double>> matrix;
  
  QFrame*      frame_1;
  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QVBoxLayout* VBoxLayout_1;
  QVBoxLayout* VBoxLayout_2;
  QVBoxLayout* VBoxLayout_3;
  QVBoxLayout* VBoxLayout_4;
  QVBoxLayout* VBoxLayout_5;
  QSpacerItem* vertical_spacer;
  QSpacerItem* vertical_spacer_1;
  QSpacerItem* vertical_spacer_2;
  QSpacerItem* vertical_spacer_3;
  QSpacerItem* vertical_spacer_4;
  QSpacerItem* vertical_spacer_5;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_1;
  QSpacerItem* horizontal_spacer_pushButton_apply;
  QHBoxLayout* HBoxLayout_frame;
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
  QHBoxLayout* HBoxLayout_11;
  QHBoxLayout* HBoxLayout_12;
  QHBoxLayout* HBoxLayout_13;
  QHBoxLayout* HBoxLayout_14;
  QComboBox* comboBox_1;
  QLabel* label_1;
  QLabel* label_2;
  QLabel* label_3;
  QLabel* label_4;
  QLabel* label_5;
  QLabel* label_6;
  QLabel* label_7;
  QLabel* label_8;
  QLabel* label_9;
  QLabel* label_10;
  QLabel* label_11;
  QLabel* label_12;
  QLabel* label_13;
  QLabel* label_14;
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_2;
  QLineEdit* lineEdit_4;
  QLineEdit* lineEdit_5;
  QLineEdit* lineEdit_6;
  QLineEdit* lineEdit_7;
  QLineEdit* lineEdit_8;
  QLineEdit* lineEdit_9;
  QLineEdit* lineEdit_10;
  QLineEdit* lineEdit_11;
  QLineEdit* lineEdit_12;
  QLineEdit* lineEdit_13;
  QLineEdit* lineEdit_14;
  QWidget* widget_1;
  QWidget* widget_2;
  PanelTable* widget_3;
  QWidget* widget_4;
  PanelTable* widget_5;
  QWidget* widget_6;
  QWidget* widget_7;
};

#endif // SURFACEINTERACTIONMODIFYPANEL_HPP