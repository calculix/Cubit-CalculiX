#ifndef MATERIALMANAGEMENTCYCLICHARDENING_HPP
#define MATERIALMANAGEMENTCYCLICHARDENING_HPP

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

class MaterialManagementCyclicHardeningCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementCyclicHardeningCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementCyclicHardeningCard();

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
  MaterialManagementTable* table_cyclic_hardening;
  };

#endif // MATERIALMANAGEMENTCYCLICHARDENING_HPP