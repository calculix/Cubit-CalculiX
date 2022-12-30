#ifndef CCXBLOCKSELEMENTTYPEPANEL_HPP
#define CCXBLOCKSELEMENTTYPEPANEL_HPP

#include <QWidget>
#include <QList>
#include <QRadioButton>
#include <QButtonGroup>


namespace Ui {
class CCXBlocksElementType;
}

class PickWidget;

class CCXBlocksElementType : public QWidget
{
  Q_OBJECT
  
public:
  explicit CCXBlocksElementType(QWidget *parent = 0);
  ~CCXBlocksElementType();
  
private slots:
  void on_pbApply_clicked();

private:
  Ui::CCXBlocksElementType *ui;
  PickWidget* pwBlockIds;  // Reference to our pickwidget.
  QButtonGroup group;
};

#endif // CCXBLOCKSELEMENTTYPEPANEL_HPP
