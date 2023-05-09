#ifndef MATERIALMANAGEMENTDENSITYCARD_HPP
#define MATERIALMANAGEMENTDENSITYCARD_HPP

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

class MaterialManagementDensityCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementDensityCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementDensityCard();

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
  MaterialManagementTable* table_density;
  };

#endif // MATERIALMANAGEMENTDENSITYCARD_HPP