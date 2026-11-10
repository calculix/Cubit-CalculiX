#include "ReportExportDialog.hpp"
#include "ReportCommandText.hpp"
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QStringList>

bool ReportExportDialog::run(QWidget* parent, const std::string& format,
  const std::function<bool(const std::string&, std::string&)>& submit, std::string& error)
{
  error.clear();
  if (format != "pdf" && format != "odt" && format != "both")
  {
    error = "Unknown report export format.";
    return false;
  }
  QFileDialog dialog(parent, QObject::tr("Export report"));
  dialog.setAcceptMode(QFileDialog::AcceptSave);
  dialog.setFileMode(QFileDialog::AnyFile);
  dialog.setNameFilter(format == "pdf" ? "PDF (*.pdf)" : format == "odt" ? "OpenDocument text (*.odt)" : "Output base name (*)");
  if (format != "both")
  {
    dialog.setDefaultSuffix(QString::fromStdString(format));
  }
  // Confirm the actual output paths ourselves, including both files when the
  // selected filename is only a base name. Do not show two overwrite prompts.
  dialog.setOption(QFileDialog::DontConfirmOverwrite, true);
  if (dialog.exec() != QDialog::Accepted || dialog.selectedFiles().isEmpty())
  {
    return true;
  }
  const auto filename = dialog.selectedFiles().front();
  const QStringList targets = format == "both" ? QStringList{filename + ".pdf", filename + ".odt"} : QStringList{filename};
  QStringList existing;
  for (const auto& target : targets)
  {
    const QFileInfo file(target);
    if (file.isDir())
    {
      error = "Report destination is a directory: " + target.toStdString();
      return false;
    }
    if (file.exists() || file.isSymLink())
    {
      existing.append(target);
    }
  }
  const bool overwrite = !existing.isEmpty();
  if (overwrite && QMessageBox::question(parent, QObject::tr("Overwrite report files?"),
      QObject::tr("Overwrite the following existing files?\n\n%1").arg(existing.join('\n')),
      QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes)
  {
    return true;
  }
  std::string quoted;
  if (!ReportCommandText::quote(filename, quoted, error))
  {
    return false;
  }
  return submit("report export " + format + " " + quoted + (overwrite ? " overwrite" : ""), error);
}
