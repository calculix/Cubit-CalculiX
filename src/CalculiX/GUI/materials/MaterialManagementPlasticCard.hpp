#ifndef MATERIALMANAGEMENTPLASTICCARD_HPP
#define MATERIALMANAGEMENTPLASTICCARD_HPP

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

class MaterialManagementPlasticCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementPlasticCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementPlasticCard();

  bool isInitialized;
  
  void update(MaterialManagementItem *material);

private slots:
  void plastic_type_currentIndexChanged(int index);
  
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
  QComboBox* plastic_type;
  MaterialManagementTable* table_isotropic;
  MaterialManagementTable* table_kinematic;
  MaterialManagementTable* table_combined;
  MaterialManagementTable* table_johnsoncook;
  };

#endif // MATERIALMANAGEMENTPLASTICCARD_HPP