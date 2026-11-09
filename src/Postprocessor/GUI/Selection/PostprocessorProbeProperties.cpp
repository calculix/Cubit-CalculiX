#include "PostprocessorProbeProperties.hpp"
#include "Postprocessor/Interfaces/PostprocessorApplicationInterface.hpp"
#include <QLabel>
#include <QVBoxLayout>
#include <utility>
PostprocessorProbeProperties::PostprocessorProbeProperties(
  std::weak_ptr<PostprocessorApplicationInterface> active, QWidget* parent)
  : QGroupBox(tr("Selection / Probe"),parent), application(std::move(active))
{
  auto* layout = new QVBoxLayout(this);
  result = new QLabel(this);
  result->setObjectName("Postprocessor.ProbeResult");
  result->setTextFormat(Qt::PlainText);
  result->setTextInteractionFlags(Qt::TextSelectableByMouse);
  result->setWordWrap(true);
  layout->addWidget(result);
  refresh();
}
void PostprocessorProbeProperties::refresh()
{
  const auto active = application.lock();
  std::string error;
  const auto probe = active ? active->probe(error) : std::nullopt;
  result->setText(probe ? QString::fromUtf8(Postprocessor::probe_text(*probe).c_str()) : tr("No result entity selected."));
}
