#include "PostprocessorTopologyCutoffControls.hpp"
#include "Postprocessor/Commands/plots/PlotCommandText.hpp"
#include "Postprocessor/Core/sources/DataSource.hpp"
#include "CubitMessage.hpp"
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QSignalBlocker>
#include <QVBoxLayout>
#include <utility>

PostprocessorTopologyCutoffControls::PostprocessorTopologyCutoffControls(Submit submit, QWidget* parent)
  : QWidget(parent), submit(std::move(submit))
{
  setObjectName("Postprocessor.TopologyCutoffControls");
  auto* layout = new QVBoxLayout(this);
  layout->setContentsMargins(0,0,0,0);
  iterations = new QLabel(this);
  layout->addWidget(iterations);
  auto* form = new QFormLayout;
  cutoff = new QDoubleSpinBox(this);
  cutoff->setObjectName("Postprocessor.Topology.Cutoff");
  cutoff->setRange(0,1);
  cutoff->setDecimals(6);
  cutoff->setSingleStep(0.05);
  form->addRow(tr("Retention cutoff"),cutoff);
  layout->addLayout(form);
  apply = new QPushButton(tr("Apply"),this);
  apply->setObjectName("Postprocessor.Topology.ApplyCutoff");
  layout->addWidget(apply,0,Qt::AlignRight);
  auto* help = new QLabel(tr("Preserved elements are always kept. Applies to all iterations."),this);
  help->setWordWrap(true);
  layout->addWidget(help);
  connect(cutoff,qOverload<double>(&QDoubleSpinBox::valueChanged),this,[this](double)
  {
    update_apply();
  });
  connect(apply,&QPushButton::clicked,this,[this]()
  {
    cutoff->interpretText();
    if (!apply->isEnabled())
    {
      return;
    }
    const auto quoted = Postprocessor::plot_quote(sourceName);
    if (quoted.isEmpty())
    {
      PRINT_ERROR("Cannot quote the topology source name.\n");
      return;
    }
    const auto command = QStringLiteral("post topology threshold ")+QString::number(cutoff->value(),'g',17)+
      QStringLiteral(" source ")+quoted;
    std::string error;
    if (!this->submit(command.toUtf8().toStdString(),error))
    {
      PRINT_ERROR("Cannot submit topology cutoff: %s\n",error.c_str());
    }
  });
  hide();
}

void PostprocessorTopologyCutoffControls::show_source(const Postprocessor::DataSource* source, std::size_t frames)
{
  const Postprocessor::JobSourceReference* reference = nullptr;
  if (source)
  {
    reference = std::get_if<Postprocessor::JobSourceReference>(&source->reference);
  }
  if (!reference || !reference->topology_threshold)
  {
    sourceId.clear();
    sourceName.clear();
    committed.reset();
    hide();
    return;
  }
  if (sourceId != source->id || committed != reference->topology_threshold)
  {
    const QSignalBlocker blocker(cutoff);
    sourceId = source->id;
    committed = reference->topology_threshold;
    cutoff->setValue(*committed);
    displayedCutoff = cutoff->value();
  }
  sourceName = source->name;
  iterations->setText(tr("Completed iterations: %1").arg(static_cast<qulonglong>(frames)));
  cutoff->setEnabled(frames != 0);
  update_apply();
  show();
}

void PostprocessorTopologyCutoffControls::update_apply()
{
  apply->setEnabled(committed.has_value() && cutoff->isEnabled() && submit && cutoff->value() != displayedCutoff);
}
