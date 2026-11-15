#include <type_traits>
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
  const auto probe = [&]() -> std::common_type_t<decltype(active->probe(error)), decltype(std::nullopt)> {
    if (active) {
      return active->probe(error);
    } else {
      return std::nullopt;
    }
  }();
  result->setText([&]() -> std::common_type_t<decltype(QString::fromUtf8(Postprocessor::probe_text(*probe).c_str())), decltype(tr("No result entity selected."))> {
    if (probe) {
      return QString::fromUtf8(Postprocessor::probe_text(*probe).c_str());
    } else {
      return tr("No result entity selected.");
    }
  }());
}
