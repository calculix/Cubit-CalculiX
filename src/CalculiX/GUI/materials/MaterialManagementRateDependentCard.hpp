#ifndef MATERIALMANAGEMENTRATEDEPENDENTCARD_HPP
#define MATERIALMANAGEMENTRATEDEPENDENTCARD_HPP

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

class MaterialManagementRateDependentCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementRateDependentCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementRateDependentCard();

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
  MaterialManagementTable* table_rate_dependent;
  };

#endif // MATERIALMANAGEMENTRATEDEPENDENTCARD_HPP