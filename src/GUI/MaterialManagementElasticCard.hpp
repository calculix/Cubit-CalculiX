#ifndef MATERIALMANAGEMENTELASTICCARD_HPP
#define MATERIALMANAGEMENTELASTICCARD_HPP

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QSpacerItem>
#include <QTableWidget>

class MaterialManagementItem;
class MaterialManagementTable;

class MaterialManagementElasticCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementElasticCard(QWidget *parent);
  ~MaterialManagementElasticCard();

  bool isInitialized;
  
/*
private slots:
  void on_pushButton_ok_clicked(bool);
*/
  
private:
  std::string log;
  MaterialManagementItem* material_item;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QHBoxLayout* HBoxLayout;
  QSpacerItem* vertical_spacer;
  QSpacerItem* horizontal_spacer;
  QSpacerItem* horizontal_spacer_type;
  QLabel* label_title;
  QHBoxLayout* HBoxLayout_type;
  QLabel* label_type;
  QComboBox* elastic_type;
  MaterialManagementTable* table_isotropic;
};

#endif // MATERIALMANAGEMENTELASTICCARD_HPP