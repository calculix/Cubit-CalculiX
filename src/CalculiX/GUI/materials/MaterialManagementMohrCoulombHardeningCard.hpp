#ifndef MATERIALMANAGEMENTMOHRCOULOMBHARDENINGCARD_HPP
#define MATERIALMANAGEMENTMOHRCOULOMBHARDENINGCARD_HPP

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

class MaterialManagementMohrCoulombHardeningCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementMohrCoulombHardeningCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementMohrCoulombHardeningCard();

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
  MaterialManagementTable* table_mohr_coulomb_hardening;
  };

#endif // MATERIALMANAGEMENTMOHRCOULOMBHARDENINGCARD_HPP