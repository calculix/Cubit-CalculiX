#ifndef DAMPINGMODIFYPANEL_HPP
#define DAMPINGMODIFYPANEL_HPP

#include "Core/steps/DampingSettings.hpp"
#include <string>

#include <QHash>
#include <QString>
#include <QWidget>
#include <QList>
#include <QPushButton>
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
class DampingModifyPanel : public QWidget
{
  Q_OBJECT
  
public:
  bool loadDamping();
  bool setDamping(int id);
  explicit DampingModifyPanel(QWidget *parent = 0);
  ~DampingModifyPanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  int loadedEntityId = -1;
  QHash<QString, QString> originalValues;
  bool fieldChanged(const QString& field, const QString& value) const;
  std::string log;

  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QSpacerItem* vertical_spacer;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_pushButton_apply;
  QHBoxLayout* HBoxLayout_0;
  QHBoxLayout* HBoxLayout_1;
  QLabel* label_0;
  QLabel* label_1;
  QLineEdit* lineEdit_0;
  QLineEdit* lineEdit_1;
};

#endif // DAMPINGMODIFYPANEL_HPP
