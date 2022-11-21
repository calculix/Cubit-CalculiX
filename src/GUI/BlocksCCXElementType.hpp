#ifndef BLOCKSCCXELEMENTTYPE_HPP
#define BLOCKSCCXELEMENTTYPE_HPP

#include <QWidget>

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
};

#endif // BLOCKSCCXELEMENTTYPE_HPP
