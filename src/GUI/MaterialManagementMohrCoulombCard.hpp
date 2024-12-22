#ifndef MATERIALMANAGEMENTMOHRCOULOMBCARD_HPP
#define MATERIALMANAGEMENTMOHRCOULOMBCARD_HPP

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

class MaterialManagementMohrCoulombCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementMohrCoulombCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementMohrCoulombCard();

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
  MaterialManagementTable* table_mohr_coulomb;
  };

#endif // MATERIALMANAGEMENTMOHRCOULOMBCARD_HPP