#ifndef MATERIALMANAGEMENTDAMPINGCARD_HPP
#define MATERIALMANAGEMENTDAMPINGCARD_HPP

#include <QObject>
#include <QWidget>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QSpacerItem>
#include <QTableWidget>

class MaterialManagementItem;
class MaterialManagementTable;

class MaterialManagementDampingCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementDampingCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementDampingCard();

  bool isInitialized;
  
  void update(MaterialManagementItem *material);


private slots:
  void lineEdit_structural_textChanged(QString text);
  
private:
  std::string log;
  MaterialManagementItem* material;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QHBoxLayout* HBoxLayout;
  QHBoxLayout* HBoxLayout_structural;
  QSpacerItem* vertical_spacer;
  QSpacerItem* horizontal_spacer;
  QSpacerItem* horizontal_spacer_structural;
  QLabel* label_title;
  QLabel* label_structural;
  QLineEdit* lineEdit_structural;
  };

#endif // MATERIALMANAGEMENTDAMPINGCARD_HPP