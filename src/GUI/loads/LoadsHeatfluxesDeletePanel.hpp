#ifndef LOADSHEATFLUXESDELETEPANEL_HPP
#define LOADSHEATFLUXESDELETEPANEL_HPP
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
class LoadsHeatfluxesDeletePanel : public QWidget
{
  Q_OBJECT

public:
  explicit LoadsHeatfluxesDeletePanel(QWidget *parent = 0);
  ~LoadsHeatfluxesDeletePanel();

  bool isInitialized;

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
  QLabel* label_1;
  QLineEdit* lineEdit_1;
};

#endif // LOADSHEATFLUXESDELETEPANEL_HPP
