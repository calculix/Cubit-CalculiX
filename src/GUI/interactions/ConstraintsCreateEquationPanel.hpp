#ifndef CONSTRAINTSCREATEEQUATIONPANEL_HPP
#define CONSTRAINTSCREATEEQUATIONPANEL_HPP
#include <string>
#include <vector>

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

class ConstraintsCreateEquationPanel : public QWidget
{
  Q_OBJECT
  
public:
  explicit ConstraintsCreateEquationPanel(QWidget *parent = 0);
  ~ConstraintsCreateEquationPanel();

  bool isInitialized;
  
private slots:
  void on_pushButton_apply_clicked(bool);

private:
  std::string log;

  std::vector<std::vector<double>> matrix;

  QFrame*      frame_2;
  QGridLayout* GridLayout;
  QVBoxLayout* VBoxLayout;
  QSpacerItem* vertical_spacer;
  QPushButton* pushButton_apply;
  QHBoxLayout* HBoxLayout_pushButton_apply;
  QSpacerItem* horizontal_spacer_pushButton_apply;
  QHBoxLayout* HBoxLayout_1;
  QHBoxLayout* HBoxLayout_2;
  QLabel* label_1;
  QLineEdit* lineEdit_1;
  PanelTable* widget_2;
};

#endif // CONSTRAINTSCREATEEQUATIONPANEL_HPP