#ifndef MATERIALMANAGEMENTHYPERFOAMCARD_HPP
#define MATERIALMANAGEMENTHYPERFOAMCARD_HPP

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

class MaterialManagementHyperfoamCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementHyperfoamCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementHyperfoamCard();

  bool isInitialized;
  
  void update(MaterialManagementItem *material);

private slots:
  void hyperfoam_type_currentIndexChanged(int index);
  
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
  QComboBox* hyperfoam_type;
  MaterialManagementTable* table_n1;
  MaterialManagementTable* table_n2;
  MaterialManagementTable* table_n3;
};

#endif // MATERIALMANAGEMENTHYPERFOAMCARD_HPP