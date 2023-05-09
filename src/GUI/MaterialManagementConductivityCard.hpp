#ifndef MATERIALMANAGEMENTCONDUCTIVITYCARD_HPP
#define MATERIALMANAGEMENTCONDUCTIVITYCARD_HPP

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QSpacerItem>
#include <QTableWidget>
#include <QLineEdit>

class MaterialManagementItem;
class MaterialManagementTable;

class MaterialManagementConductivityCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementConductivityCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementConductivityCard();

  bool isInitialized;
  
  void update(MaterialManagementItem *material);

private slots:
  void conductivity_type_currentIndexChanged(int index);
  
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
  QComboBox* conductivity_type;
  MaterialManagementTable* table_isotropic;
  MaterialManagementTable* table_orthotropic;
  MaterialManagementTable* table_anisotropic;
};

#endif // MATERIALMANAGEMENTCONDUCTIVITYCARD_HPP