#ifndef MATERIALMANAGEMENTHYPERELASTICCARD_HPP
#define MATERIALMANAGEMENTHYPERELASTICCARD_HPP

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

class MaterialManagementHyperelasticCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementHyperelasticCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementHyperelasticCard();

  bool isInitialized;
  
  void update(MaterialManagementItem *material);

private slots:
  void hyperelastic_type_currentIndexChanged(int index);
  
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
  QComboBox* hyperelastic_type;
  MaterialManagementTable* table_arruda_boyce;
  MaterialManagementTable* table_mooney_rivlin;
  MaterialManagementTable* table_neo_hooke;
  MaterialManagementTable* table_ogden_n1;
  MaterialManagementTable* table_ogden_n2;
  MaterialManagementTable* table_ogden_n3;
  MaterialManagementTable* table_polynomial_n1;
  MaterialManagementTable* table_polynomial_n2;
  MaterialManagementTable* table_polynomial_n3;
  MaterialManagementTable* table_reduced_polynomial_n1;
  MaterialManagementTable* table_reduced_polynomial_n2;
  MaterialManagementTable* table_reduced_polynomial_n3;
  MaterialManagementTable* table_yeoh;
};

#endif // MATERIALMANAGEMENTHYPERELASTICCARD_HPP