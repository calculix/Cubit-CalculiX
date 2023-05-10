#ifndef SURFACEINTERACTIONCREATEPANEL_HPP
#define SURFACEINTERACTIONCREATEPANEL_HPP

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

class SurfaceInteractionsCreatePanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit SurfaceInteractionsCreatePanel(QWidget *parent = 0);
  ~SurfaceInteractionsCreatePanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);
  void comboBox_currentIndexChanged(int index);

private:
  std::string log;
  
  QFrame*      frame_1;
  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QVBoxLayout* VBoxLayout_1;
  QVBoxLayout* VBoxLayout_2;
  QVBoxLayout* VBoxLayout_3;
  QSpacerItem* vertical_spacer;
  QSpacerItem* vertical_spacer_1;
  QSpacerItem* vertical_spacer_2;
  QSpacerItem* vertical_spacer_3;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_1;
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
  QLineEdit* lineEdit_1;
  QLineEdit* lineEdit_4;
  QLineEdit* lineEdit_5;
  QLineEdit* lineEdit_6;
  QLineEdit* lineEdit_7;
  QLineEdit* lineEdit_8;
  QLineEdit* lineEdit_9;
  QWidget* widget_1;
  QWidget* widget_2;
  PanelTable* widget_3;
  QWidget* widget_4;
  QWidget* widget_5;
};

#endif // SURFACEINTERACTIONCREATEPANEL_HPP