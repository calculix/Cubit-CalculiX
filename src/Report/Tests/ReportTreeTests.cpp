#include "ReportTestProviders.hpp"
#include "ReportTestProgress.hpp"
#include "Report/Core/ReportCore.hpp"
#include "Report/Core/ReportConfigurationResolver.hpp"
#include "Report/GUI/ReportContentTree.hpp"
#include "Report/GUI/ReportOutlineTree.hpp"
#include "Report/Document/ReportDocumentLayout.hpp"
#include "Report/GUI/ReportPreviewWindow.hpp"
#include <QListWidget>
#include <QTreeWidgetItemIterator>
#include <QApplication>
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
  progress.cancelStage = "Building preview tables";
  progress.cancelAfter = 256;
  check(!layout.prepare(large, error, &progress) && layout.report().id == originalId && layout.page_count() == originalPages,
    "Cancelling a preview cell batch preserves the existing prepared layout");
  progress.cancelStage.clear();
  progress.stages.clear();
  check(layout.prepare(large, error, &progress), "Batched preview construction can be retried");
  check(std::find(progress.stages.begin(), progress.stages.end(), "Paginating report preview") != progress.stages.end() &&
    std::find(progress.stages.begin(), progress.stages.end(), "Updating report contents") != progress.stages.end(),
    "Preview has separate construction, pagination and contents progress stages");
  return failures;
}
