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
  MaterialManagementElasticCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementElasticCard();

  bool isInitialized;
  
  void update(MaterialManagementItem *material);

private slots:
  void elastic_type_currentIndexChanged(int index);
  
private:
  std::string log;
  MaterialManagementItem* material;

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
  MaterialManagementTable* table_orthotropic;
  MaterialManagementTable* table_anisotropic;
  MaterialManagementTable* table_ec;
};

#endif // MATERIALMANAGEMENTELASTICCARD_HPP