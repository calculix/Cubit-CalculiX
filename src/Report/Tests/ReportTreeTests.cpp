#include "ReportTestProviders.hpp"
#include "ReportTestProgress.hpp"
#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportConfigurationResolver.hpp"
#include "Report/GUI/Content/ReportContentTree.hpp"
#include "Report/GUI/Content/ReportOutlineTree.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/GUI/Preview/ReportPreviewWindow.hpp"
#include <QListWidget>
#include <QTreeWidgetItemIterator>
#include <QApplication>
#include <QComboBox>
#include <algorithm>
#include <iostream>

namespace
{
QTreeWidgetItem* find(QTreeWidget& tree, const QString& id, int role)
{
  for (QTreeWidgetItemIterator it(&tree); *it; ++it)
  {
    if ((*it)->data(0, role).toString() == id)
    {
      return *it;
    }
  }
  return nullptr;
}
}

int reportTreeTests()
{
  int failures = 0;
  const auto check = [&](bool condition, const char* message)
  {
    if (!condition)
    {
      std::cerr << message << '\n';
      ++failures;
    }
  };
  std::string error;
  ReportTestProviders providers;
  ReportCore core(providers);
  ReportTemplate definition;
  check(core.describe(definition, error), "Describe tree fixture");
  definition.content_groups.push_back({"model", "Model"});
  definition.options.front().group_id = "model";
  definition.options.front().label = "Model overview";
  ReportOption material;
  material.key = "test.materials";
  material.label = "Materials";
  material.kind = "boolean";
  material.default_value = "true";
  material.group_id = "model";
  definition.options.push_back(material);
  definition.defaults.values[material.key] = "true";
  auto configuration = definition.defaults;
  std::vector<std::string> commands;
  ReportContentTree content(nullptr, [&](const std::string& command, std::string&)
  {
    commands.push_back(command);
    return true;
  });
  content.refresh(definition, configuration);
  auto* model = find(content, "model", Qt::UserRole + 1);
  auto* overview = find(content, "test.content", Qt::UserRole);
  check(model && overview && overview->parent() == model, "Template groups create actual child nodes");
  if (model && overview)
  {
    model->setExpanded(false);
    content.setCurrentItem(overview);
    model->setCheckState(0, Qt::Unchecked);
    QApplication::processEvents();
    check(commands.size() == 1 && commands.front().find("report configure settings") == 0 &&
      commands.front().find("test.content") != std::string::npos && commands.front().find("test.materials") != std::string::npos,
      "Group changes use one atomic readable settings command");
    model = find(content, "model", Qt::UserRole + 1);
    check(model && !model->isExpanded(), "Content expansion survives command refresh");
  }
  configuration.values["test.materials"] = "false";
  content.refresh(definition, configuration);
  model = find(content, "model", Qt::UserRole + 1);
  check(model && model->checkState(0) == Qt::PartiallyChecked, "Content groups display partial selection");
  const auto beforeRefresh = commands.size();
  content.refresh(definition, configuration);
  check(commands.size() == beforeRefresh, "Passive content refresh does not submit commands");
  auto malformed = definition;
  malformed.content_groups.push_back({"cycle", "Cycle", "cycle"});
  ReportConfiguration resolved;
  check(!ReportConfigurationResolver::resolve(malformed, configuration, resolved, error), "Core rejects cyclic content groups");

  check(core.generate("test", 1, "", error), "Generate outline fixture");
  auto document = std::make_shared<ReportDocument>(*core.document());
  document->sections.front().tables.front().rows = {{"A", 1.0}, {"B", 2.0}, {"C", 3.0}};
  int requestedPage = 0;
  ReportOutlineTree outline(nullptr, [&](int page)
  {
    requestedPage = page;
  });
  outline.refresh(document);
  auto* section = find(outline, "test.content", Qt::UserRole);
  check(section && section->text(1).contains("3 rows") && section->text(2).isEmpty(),
    "Outline row counts are available before pagination");
  const std::vector<std::pair<std::string, int>> pages = {{"test.content", 4}, {"report.diagnostics", 6}};
  outline.refresh(document, &pages, 7);
  section = find(outline, "test.content", Qt::UserRole);
  check(section && section->text(2) == "5" && outline.topLevelItem(0)->text(2).contains("7 pages"),
    "Outline converts zero-based section locations to displayed page numbers");
  if (section)
  {
    outline.itemActivated(section, 0);
  }
  check(requestedPage == 5, "Outline navigation uses the containing section's actual page");
  outline.refresh(document);
  section = find(outline, "test.content", Qt::UserRole);
  check(section && section->text(2) == "5", "Repeated pre-layout refresh retains cached page locations");

  ReportDocumentLayout layout;
  check(layout.prepare(*document, error), "Prepare initial layout");
  auto windowLayout = std::make_shared<ReportDocumentLayout>();
  check(windowLayout->prepare(*document, error), "Prepare preview-window anchor fixture");
  const auto command = [](const std::string&, std::string&)
  {
    return true;
  };
  ReportPreviewWindow window(nullptr, command, command);
  window.set_layout(windowLayout);
  const auto* navigation = window.findChild<QListWidget*>();
  check(navigation && navigation->findItems("Sources and diagnostics", Qt::MatchExactly).size() == 1,
    "Preview-window navigation accepts the layout-owned diagnostics anchor without map lookup exceptions");
  const auto originalId = layout.report().id;
  const int originalPages = layout.page_count();
  auto large = *document;
  large.id = "larger-layout";
  auto& rows = large.sections.front().tables.front().rows;
  rows.clear();
  for (int index = 0; index < 512; ++index)
  {
    rows.push_back({"Value", index});
  }
  ReportTestProgress progress;
  progress.cancelStage = "Preparing preview batch";
  progress.cancelAfter = 1;
  check(!layout.prepare(large, error, &progress) && layout.report().id == originalId && layout.page_count() == originalPages,
    "Cancelling a preview cell batch preserves the existing prepared layout");
  progress.cancelStage.clear();
  progress.stages.clear();
  check(layout.prepare(large, error, &progress), "Batched preview construction can be retried");
  check(std::find(progress.stages.begin(), progress.stages.end(), "Paginating report preview") != progress.stages.end() &&
    std::find(progress.stages.begin(), progress.stages.end(), "Updating report contents") != progress.stages.end(),
    "Preview has separate construction, pagination and contents progress stages");
  ReportTemplate choices = definition;
  ReportOption frames;
  frames.key = "post.results.frames";
  frames.kind = "choice";
  frames.label = "Frame selection";
  frames.group_id = "model";
  frames.default_value = "last_per_step";
  frames.choices = {"last_per_step", "envelope", "all_frames"};
  choices.options.push_back(frames);
  choices.defaults.values[frames.key] = frames.default_value;
  commands.clear();
  content.refresh(choices, choices.defaults);
  auto* mode = find(content, "post.results.frames", Qt::UserRole);
  QComboBox* editor = nullptr;
  if (mode)
  {
    editor = qobject_cast<QComboBox*>(content.itemWidget(mode, 1));
  }
  check(editor && editor->currentData() == "last_per_step" && editor->count() == 3,
    "Content tree displays all three mutually exclusive frame choices");
  if (editor)
  {
    editor->setCurrentIndex(1);
    QMetaObject::invokeMethod(editor, "activated", Q_ARG(int, 1));
    check(commands.size() == 1 && commands.front().find("envelope") != std::string::npos,
      "Frame choice submits a readable settings command");
    QApplication::processEvents();
  }
  commands.clear();
  auto* results = find(content, "model", Qt::UserRole + 1);
  if (results)
  {
    results->setCheckState(0, Qt::Unchecked);
    check(commands.size() == 1 && commands.front().find("post.results.frames") == std::string::npos,
      "Group toggles preserve the frame choice instead of writing a boolean to it");
    QApplication::processEvents();
  }
  check(layout.page_count() > 1 && layout.contents().size() >= 2,
    "Batched pagination retains global section and diagnostics anchors");
  for (const auto& anchor : layout.contents())
  {
    check(anchor.second >= 0 && anchor.second < layout.page_count(),
      "Global section anchors reference an existing batch page");
  }
  auto longText = *document;
  longText.sections.front().tables.front().rows.front()[0] = QString(100000, QChar('x'));
  check(layout.prepare(longText, error) &&
    layout.report().sections.front().tables.front().rows.front()[0].toString().size() == 100000,
    "Bounded text-cell continuations retain the original semantic table");
  return failures;
}
