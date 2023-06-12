#ifndef CCXBLOCKSELEMENTTYPEPANEL_HPP
#define CCXBLOCKSELEMENTTYPEPANEL_HPP

#include <QWidget>
#include <QList>
#include <QRadioButton>
#include <QButtonGroup>
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
#include <QTabWidget>

class PickWidget;

class CCXBlocksElementType : public QWidget
{
  Q_OBJECT
  
public:
  explicit CCXBlocksElementType(QWidget *parent = 0);
  ~CCXBlocksElementType();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  QGridLayout* GridLayout;
  QGridLayout* GridLayout_beamtruss;
  QGridLayout* GridLayout_shellmembrane;
  QGridLayout* GridLayout_plainstressstrain;
  QGridLayout* GridLayout_axi;
  QGridLayout* GridLayout_volume;
  QVBoxLayout* VBoxLayout;
  QSpacerItem* vertical_spacer;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_pushButton_apply;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_Tab;
  QLabel* label_1;
  PickWidget* PickWidget_1;
  QTabWidget* TabWidget;
  QWidget* widget_beamtruss;
  QWidget* widget_shellmembrane;
  QWidget* widget_plainstressstrain;
  QWidget* widget_axi;
  QWidget* widget_volume;
  std::vector<QRadioButton*> RadioButton_beamtruss;
  std::vector<QRadioButton*> RadioButton_shellmembrane;
  std::vector<QRadioButton*> RadioButton_plainstressstrain;
  std::vector<QRadioButton*> RadioButton_axi;
  std::vector<QRadioButton*> RadioButton_volume;
  QButtonGroup group;
};

#endif // CCXBLOCKSELEMENTTYPEPANEL_HPP
