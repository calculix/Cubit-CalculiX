#ifndef BLOCKSCCXELEMENTTYPEPANEL_HPP
#define BLOCKSCCXELEMENTTYPEPANEL_HPP

#include <QWidget>
#include <QList>
#include <QRadioButton>
#include <QButtonGroup>


namespace Ui {
class BlocksCCXElementType;
}

class PickWidget;

class BlocksCCXElementType : public QWidget
{
  Q_OBJECT
  
public:
  explicit BlocksCCXElementType(QWidget *parent = 0);
  ~BlocksCCXElementType();
  
private slots:
  void on_pbApply_clicked();

private:
  Ui::BlocksCCXElementType *ui;
  PickWidget* pwBlockIds;  // Reference to our pickwidget.
  QButtonGroup group;
};

#endif // BLOCKSCCXELEMENTTYPEPANEL_HPP
