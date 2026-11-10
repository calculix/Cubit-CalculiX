#ifndef POSTPROCESSORIMPORTDIALOG_HPP
#define POSTPROCESSORIMPORTDIALOG_HPP
#include <QString>
class QWidget;
namespace PostprocessorImportDialog
{
  // Empty means cancelled; errors are shown in the dialog. No commands execute here.
  QString command(QWidget* parent);
}
#endif
