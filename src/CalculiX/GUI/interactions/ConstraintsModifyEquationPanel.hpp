#ifndef CONSTRAINTSMODIFYEQUATIONPANEL_HPP
#define CONSTRAINTSMODIFYEQUATIONPANEL_HPP

#include "Core/interactions/ConstraintsSettings.hpp"
#include <string>
#include <vector>

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
class PanelTable;

class ConstraintsModifyEquationPanel : public QWidget
{
  Q_OBJECT
  
public:
  bool setConstraint(int id);
  explicit ConstraintsModifyEquationPanel(QWidget *parent = 0);
  ~ConstraintsModifyEquationPanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  int loadedEntityId = -1;
  QHash<QString, QString> originalValues;
  bool fieldChanged(const QString& field, const QString& value) const;
  std::string log;

  std::vector<std::vector<double>> matrix;

  QFrame*      frame_2;
  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QSpacerItem* vertical_spacer;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_pushButton_apply;
  QHBoxLayout* HBoxLayout_0;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QLabel* label_0;
  QLabel* label_1;
  QLineEdit* lineEdit_0;
  QLineEdit* lineEdit_1;
  PanelTable* widget_2;
};

#endif // CONSTRAINTSMODIFYEQUATIONPANEL_HPP
