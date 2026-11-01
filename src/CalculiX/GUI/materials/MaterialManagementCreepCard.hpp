#ifndef MATERIALMANAGEMENTCREEPCARD_HPP
#define MATERIALMANAGEMENTCREEPCARD_HPP

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

class MaterialManagementCreepCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementCreepCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementCreepCard();

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
  MaterialManagementTable* table_creep;
  };

#endif // MATERIALMANAGEMENTCREEPCARD_HPP