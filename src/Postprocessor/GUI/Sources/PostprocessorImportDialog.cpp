#include "PostprocessorImportDialog.hpp"
#include <QFileDialog>
#include <QFileInfo>
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QMessageBox>
namespace
{
  QString quote(const QString& value)
  {
    const QChar delimiter = value.contains('"') ? QChar(0x27) : QChar('"');
    if (value.contains(delimiter))
    {
      return {};
    }
    for (QChar character : value)
    {
      if (character.unicode() < 32 || character.unicode() == 127)
      {
        return {};
      }
    }
    return delimiter+value+delimiter;
  }
}
QString PostprocessorImportDialog::command(QWidget* parent)
{
  const auto path = QFileDialog::getOpenFileName(parent,QObject::tr("Import result files"),{},
    QObject::tr("Results (*.frd *.dat *.vtu *.vtpc *.vtt);;CalculiX (*.frd *.dat);;VTK (*.vtu *.vtpc *.vtt)"));
  if (path.isEmpty())
  {
    return {};
  }
  const auto extension = QFileInfo(path).suffix().toLower();
  QStringList paths{path};
  QString dat, input;
  if (extension == "frd" || extension == "dat")
  {
    QDialog dialog(parent);
    dialog.setWindowTitle(QObject::tr("Import CalculiX results"));
    auto* layout = new QFormLayout(&dialog);
    auto* primary = new QLineEdit(path,&dialog);
    primary->setReadOnly(true);
    layout->addRow(QObject::tr("Result"),primary);
    auto row = [&](const QString& label,const QString& filter,const QString& suggestion)
    {
      auto* widget = new QWidget(&dialog);
      auto* line = new QHBoxLayout(widget);
      line->setContentsMargins(0,0,0,0);
      auto* edit = new QLineEdit(suggestion,widget);
      auto* browse = new QPushButton(QObject::tr("Browse…"),widget);
      line->addWidget(edit); line->addWidget(browse);
      QObject::connect(browse,&QPushButton::clicked,&dialog,[&,edit,filter]()
      {
        const auto selected = QFileDialog::getOpenFileName(&dialog,QObject::tr("Choose companion file"),edit->text(),filter);
        if (!selected.isEmpty())
        {
          edit->setText(selected);
        }
      });
      layout->addRow(label,widget);
      return edit;
    };
    const auto base = path.left(path.size()-QFileInfo(path).suffix().size());
    QLineEdit* datEdit = nullptr;
    if (extension == "frd")
    {
      datEdit = row(QObject::tr("DAT (optional)"),QObject::tr("CalculiX DAT (*.dat)"),QFileInfo::exists(base+"dat") ? base+"dat" : QString{});
    }
    auto* inputEdit = row(QObject::tr("INP (optional)"),QObject::tr("CalculiX input (*.inp)"),QFileInfo::exists(base+"inp") ? base+"inp" : QString{});
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);
    layout->addRow(buttons);
    QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,&QDialog::accept);
    QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
    if (dialog.exec() != QDialog::Accepted)
    {
      return {};
    }
    dat = datEdit ? datEdit->text().trimmed() : QString{};
    input = inputEdit->text().trimmed();
  }
  const auto primary = quote(path);
  const auto datArgument = dat.isEmpty() ? QString{} : quote(dat);
  const auto inputArgument = input.isEmpty() ? QString{} : quote(input);
  if (primary.isEmpty() || (!dat.isEmpty() && datArgument.isEmpty()) || (!input.isEmpty() && inputArgument.isEmpty()))
  {
    QMessageBox::warning(parent,QObject::tr("Import results"),QObject::tr("A file path cannot be represented by a quoted command argument."));
    return {};
  }
  QString command = extension == "frd" || extension == "dat" ? QStringLiteral("post open ccx file ") : QStringLiteral("post open file ");
  command += primary;
  if (!dat.isEmpty())
  {
    command += " dat "+datArgument;
  }
  if (!input.isEmpty())
  {
    command += " input "+inputArgument;
  }
  return command;
}
