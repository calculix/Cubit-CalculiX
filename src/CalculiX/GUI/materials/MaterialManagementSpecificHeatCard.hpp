#ifndef MATERIALMANAGEMENTSPECIFICHEATCARD_HPP
#define MATERIALMANAGEMENTSPECIFICHEATCARD_HPP

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

class MaterialManagementSpecificHeatCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementSpecificHeatCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementSpecificHeatCard();

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
  MaterialManagementTable* table_specific_heat;
  };

#endif // MATERIALMANAGEMENTSPECIFICHEATCARD_HPP