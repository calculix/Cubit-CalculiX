#ifndef MATERIALMANAGEMENTEXPANSIONCARD_HPP
#define MATERIALMANAGEMENTEXPANSIONCARD_HPP

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

class MaterialManagementExpansionCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementExpansionCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementExpansionCard();

  bool isInitialized;
  
  void update(MaterialManagementItem *material);

private slots:
  void expansion_type_currentIndexChanged(int index);
  void lineEdit_zero_textChanged(QString text);
  
private:
  std::string log;
  MaterialManagementItem* material;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QHBoxLayout* HBoxLayout;
  QSpacerItem* vertical_spacer;
  QSpacerItem* horizontal_spacer;
  QSpacerItem* horizontal_spacer_zero;
  QSpacerItem* horizontal_spacer_type;
  QLabel* label_title;
  QHBoxLayout* HBoxLayout_zero;
  QLabel* label_zero;
  QLineEdit* lineEdit_zero;
  QHBoxLayout* HBoxLayout_type;
  QLabel* label_type;
  QComboBox* expansion_type;
  MaterialManagementTable* table_isotropic;
  MaterialManagementTable* table_orthotropic;
  MaterialManagementTable* table_anisotropic;
};

#endif // MATERIALMANAGEMENTEXPANSIONCARD_HPP