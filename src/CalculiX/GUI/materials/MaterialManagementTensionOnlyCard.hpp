#ifndef MATERIALMANAGEMENTTENSIONONLYCARD_HPP
#define MATERIALMANAGEMENTTENSIONONLYCARD_HPP

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

class MaterialManagementTensionOnlyCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementTensionOnlyCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementTensionOnlyCard();

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
  MaterialManagementTable* table_tension_only;
  };

#endif // MATERIALMANAGEMENTTENSIONONLYCARD_HPP