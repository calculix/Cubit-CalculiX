#ifndef MATERIALMANAGEMENTDEFORMATIONPLASTICITYCARD_HPP
#define MATERIALMANAGEMENTDEFORMATIONPLASTICITYCARD_HPP

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

class MaterialManagementDeformationPlasticityCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementDeformationPlasticityCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementDeformationPlasticityCard();

  bool isInitialized;
  
  void update(MaterialManagementItem *material);
  
private:
  std::string log;
  MaterialManagementItem* material;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QHBoxLayout* HBoxLayout;
  QSpacerItem* vertical_spacer;
  QSpacerItem* horizontal_spacer;
  QLabel* label_title;
  MaterialManagementTable* table_deformation_plasticity;
  };

#endif // MATERIALMANAGEMENTDEFORMATIONPLASTICITYCARD_HPP