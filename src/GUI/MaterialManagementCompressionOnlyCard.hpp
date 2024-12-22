#ifndef MATERIALMANAGEMENTCOMPRESSIONONLYCARD_HPP
#define MATERIALMANAGEMENTCOMPRESSIONONLYCARD_HPP

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

class MaterialManagementCompressionOnlyCard : public QWidget
{
  Q_OBJECT
  
public:
  MaterialManagementCompressionOnlyCard(QWidget *parent, MaterialManagementItem *material);
  ~MaterialManagementCompressionOnlyCard();

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
  MaterialManagementTable* table_compression_only;
  };

#endif // MATERIALMANAGEMENTCOMPRESSIONONLYCARD_HPP