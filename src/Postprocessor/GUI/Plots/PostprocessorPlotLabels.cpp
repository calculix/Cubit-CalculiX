#include "PostprocessorPlotLabels.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include <QDialog>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <vector>
QString plot_label_command(const std::string& plot,const std::string& target,const std::string& value,const std::string& curve)
{
  const auto scope = Postprocessor::plot_quote(plot);
  if (scope.isEmpty() || (!value.empty() && !Postprocessor::valid_plot_name(value)))
  {
    return {};
  }
  QString text = QStringLiteral("\"\"");
  if (!value.empty())
  {
    text = Postprocessor::plot_quote(value);
  }
  if (target == "curve")
  {
    const auto name = Postprocessor::plot_quote(curve);
    if (name.isEmpty())
    {
      return {};
    }
    return "post plot curve label "+text+" curve "+name+" plot "+scope;
  }
  if (target != "title" && target != "xlabel" && target != "ylabel")
  {
    return {};
  }
  return "post plot "+QString::fromStdString(target)+" "+text+" plot "+scope;
}
void edit_plot_labels(QWidget* parent,const Postprocessor::PlotDefinition& definition,const std::function<void(const QString&)>& submit)
{
  QDialog dialog(parent);
  dialog.setWindowTitle(QObject::tr("Plot labels"));
  auto* form = new QFormLayout(&dialog);
  auto* hint = new QLabel(QObject::tr("Leave a label empty to use its automatic title, units or curve name. Labels do not change data or units."),&dialog);
  hint->setWordWrap(true);
  form->addRow(hint);
  struct Input
  {
    std::string target, curve;
    std::optional<std::string> original;
    QLineEdit* editor;
  };
  std::vector<Input> inputs;
  const auto add = [&](const QString& title,const std::string& target,const std::optional<std::string>& value,const std::string& curve)
  {
    auto* editor = new QLineEdit(&dialog);
    editor->setPlaceholderText(QObject::tr("Automatic"));
    editor->setMaxLength(256);
    if (value)
    {
      editor->setText(QString::fromStdString(*value));
    }
    form->addRow(title,editor);
    inputs.push_back({target,curve,value,editor});
  };
  add(QObject::tr("Title"),"title",definition.title,{});
  add(QObject::tr("X axis"),"xlabel",definition.x_label,{});
  add(QObject::tr("Y axis"),"ylabel",definition.y_label,{});
  for (const auto& curve : definition.curves)
  {
    add(QObject::tr("Legend: %1").arg(QString::fromStdString(curve.name)),"curve",curve.label,curve.name);
  }
  auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok|QDialogButtonBox::Cancel,&dialog);
  QObject::connect(buttons,&QDialogButtonBox::rejected,&dialog,&QDialog::reject);
  QObject::connect(buttons,&QDialogButtonBox::accepted,&dialog,[&]()
  {
    for (const auto& input : inputs)
    {
      if (plot_label_command(definition.name,input.target,input.editor->text().toStdString(),input.curve).isEmpty())
      {
        QMessageBox::warning(&dialog,QObject::tr("Plot labels"),QObject::tr("Labels must be journalable text of at most 256 bytes."));
        return;
      }
    }
    dialog.accept();
  });
  form->addRow(buttons);
  if (dialog.exec() == QDialog::Accepted)
  {
    // Copy every command before dispatch, since refreshes can delete editors.
    std::vector<QString> commands;
    for (const auto& input : inputs)
    {
      const auto value = input.editor->text().toStdString();
      if ((!input.original && value.empty()) || (input.original && *input.original == value))
      {
        continue;
      }
      commands.push_back(plot_label_command(definition.name,input.target,value,input.curve));
    }
    for (const auto& command : commands)
    {
      submit(command);
    }
  }
}
